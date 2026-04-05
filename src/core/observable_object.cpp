#include <QSequentialIterable>
#include "observable_object.h"

Q_LOGGING_CATEGORY(qlcObservable, "core.observable");

QObservableObject::QObservableObject(QObject *parent) : QObject(parent)
                                                        , m_shots(ENABLE_FOREVER)
                                                        , m_propagation(true) {
}

QObservableObject::~QObservableObject() {
    if (!checkGroupChangeState()) {
        qCWarning(qlcObservable) << "[QObservableObject] invalid state, group change not end. ";
    }
}

const char *QObservableObject::dynamicName(const QObservableObject *source) {
    return source->metaObject()->className();
}

void QObservableObject::setShot(int shots) {
    m_shots = qMax(shots, static_cast<int>(DISABLE_FOREVER));
}

int QObservableObject::shots() const {
    return m_shots;
}

void QObservableObject::setPropagation(bool propagation) {
    m_propagation = propagation;
}

bool QObservableObject::checkGroupChangeState() const {
    if (!isGroupChangeBegin()) {
        return true;
    }
    qCritical() << "A group change notify is already begun, MUST call [QObservableObject::endGroupChange] on: "
            << dynamicName(this) << "=" << this;
    return false;
}

void QObservableObject::onPropertyChanged(ObservableChangeNotify &params) {
    // NOLINT(*-no-recursion)
    if (m_shots == DISABLE_FOREVER) {
        return;
    }
    auto *cur = &params;
    do {
        cur->appendSource(this);
    } while ((cur = cur->m_next.data()));

    if (isGroupChangeBegin()) {
        saveChangeNotify(params);
    } else {
        propagateChange(params);
    }
}

void QObservableObject::saveChangeNotify(const ObservableChangeNotify &params) const {
    const auto p = QSharedPointer<ObservableChangeNotify>(new ObservableChangeNotify(params));
    if (!m_groupChangeNotify->m_next) {
        m_groupChangeNotify->m_next = p;
        return;
    }
    auto last = m_groupChangeNotify->m_next;
    while (last->m_next) {
        last = last->m_next;
    }
    last->m_next = p;
    p->m_pre = last;
}

void QObservableObject::propagateChange(ObservableChangeNotify &params) {
    // NOLINT(*-no-recursion)
    if (m_shots < DISABLE_FOREVER) {
        m_shots ^= (DISABLE_FOREVER - 1);
        return;
    }
    if (m_shots > 0) {
        if (--m_shots == 0) {
            m_shots = DISABLE_FOREVER;
        }
    }
    emit propertyChanged(params);
    if (!m_propagation) {
        return;
    }
    for (auto it = m_observableParents.cbegin(); it != m_observableParents.cend();) {
        if (it->isNull()) {
            it = m_observableParents.erase(it);
        } else {
            (*it++)->onPropertyChanged(params);
        }
    }
}

ObservableShotGuard QObservableObject::setGuardedShot(int shots) {
    return ObservableShotGuard(this, shots);
}

static void
processUserProperty(QObservableObject *parent, const QVariant &value, bool recursive, QStringList &debug) {
    // NOLINT(*-no-recursion)
    if (value.typeId() < QMetaType::User) {
        return;
    }
    auto setChildParent = [parent](const QVariant &child) {
        if (!child.isNull() && child.canConvert<QObservableObject *>()) {
            const auto childObj = child.value<QObservableObject *>();
            childObj->setObservableParent(parent);
            return childObj;
        }
        return static_cast<QObservableObject *>(nullptr);
    };
    if (const auto childObj = setChildParent(value)) {
        if (recursive) {
            QObservableObject::observableChain(childObj, true);
        }
    } else if (value.canConvert<QVariantList>()) {
        auto iterable = value.value<QSequentialIterable>();
        for (const QVariant &v: iterable) {
            processUserProperty(parent, v, recursive, debug);
        }
    } else if (value.canConvert<QVariantHash>()) {
        auto iterable = value.value<QVariantHash>();
        for (const QVariant &v: iterable) {
            processUserProperty(parent, v, recursive, debug);
        }
    } else if (value.canConvert<QVariantPair>()) {
        auto pair = value.value<QVariantPair>();
        processUserProperty(parent, pair.first, recursive, debug);
        processUserProperty(parent, pair.second, recursive, debug);
    } else {
        debug << "nested type:" << value.typeName();
        qCInfo(qlcObservable) << "unhandled type: " << debug.join(" ");
    }
}

void QObservableObject::observableChain(QObservableObject *qObject, bool recursive) {
    // NOLINT(*-no-recursion)
    if (!qObject) {
        return;
    }
    qObject->m_propertyNotifiers.clear();
    const auto mo = qObject->metaObject();
    //0 is objectName
    for (int i = 1; i < mo->propertyCount(); ++i) {
        QMetaProperty mp = mo->property(i);
        auto className = mo->className();
        auto name = mp.name();
        auto typeName = mp.typeName();
        if (mp.isBindable()) {
            auto handlerWrap = [qObject, name, typeName] {
                ObservableChangeNotify params(name, typeName);
                qObject->onPropertyChanged(params);
            };
            qObject->m_propertyNotifiers.push_back(mp.bindable(qObject).addNotifier(handlerWrap));
        }
        if (mp.userType() >= QMetaType::User) {
            if (mp.isEnumType()) {
                continue;
            }
            auto value = qObject->property(name);
            QStringList debug;
            debug << "class: [" << className << "]" << mp.name();
            processUserProperty(qObject, value, recursive, debug);
        }
    }
}

void QObservableObject::setObservableParent(const QObservableObject *observableParent, bool rechain, bool recursive) {
    if (m_observableParents.contains(observableParent)) {
        if (rechain) {
            qCWarning(qlcObservable) << observableParent->metaObject()->className() <<
                    " is already an observable parent Object for:" << metaObject()->className();
        }
        return;
    }
    const auto p = const_cast<QObservableObject *>(observableParent);
    m_observableParents.append(p);
    if (rechain) {
        observableChain(recursive);
    }
}

void QObservableObject::observableChain(bool recursive) {
    observableChain(this, recursive);
}

bool QObservableObject::isShotEnable() const {
    return m_shots >= ENABLE_FOREVER;
}

void QObservableObject::disableShot() {
    setShot(DISABLE_FOREVER);
}

ObservableShotGuard QObservableObject::disableShotGuarded() {
    return setGuardedShot(DISABLE_FOREVER);
}

void QObservableObject::enableShot() {
    setShot(ENABLE_FOREVER);
}

ObservableShotGuard QObservableObject::enableShotGuarded() {
    return setGuardedShot(ENABLE_FOREVER);
}

void QObservableObject::disableShotOnce() {
    if (!isShotEnable()) {
        qCWarning(qlcObservable) << "already disabled on: " << this->metaObject()->className();
        return;
    }
    m_shots ^= (DISABLE_FOREVER - 1);
}

void QObservableObject::beginGroupChange() {
    if (const bool notified = checkGroupChangeState(); !notified) {
        throw std::runtime_error("unfinished group change after call [QObservableObject::beginGroupChange]");
    }
    m_groupChangeNotify.reset(new ObservableChangeNotify());
}

void QObservableObject::endGroupChange() {
    if (isGroupChangeBegin()) {
        if (m_groupChangeNotify->m_next) {
            ObservableChangeNotify &param = *m_groupChangeNotify->m_next;
            propagateChange(param);
        }
        m_groupChangeNotify.reset();
    }
}

bool QObservableObject::isGroupChangeBegin() const {
    return static_cast<bool>(m_groupChangeNotify);
}

void QObservableObject::removeObservableParent(const QObservableObject *observableParent) {
    m_observableParents.removeAll(observableParent);
}

ObservableChangeNotify::ObservableChangeNotify(const char *propertyName,
                                               const char *propertyType) : ObservableChangeNotify() {
    m_propertyName = propertyName;
    m_propertyTypeName = propertyType;
}

QString ObservableChangeNotify::sourceTypeName() const {
    return {QObservableObject::dynamicName(firstSource())};
}

QString ObservableChangeNotify::propertyName() const {
    return {m_propertyName};
}

QString ObservableChangeNotify::propertyTypeName() const {
    return {m_propertyTypeName};
}

bool ObservableChangeNotify::isValid() const {
    return !m_sourcePath.empty() && m_propertyName != nullptr && m_propertyTypeName != nullptr;
}

QString ObservableChangeNotify::toString() const {
    QStringList s;
    s << "Property:" << "[" << m_propertyTypeName << "]" << m_propertyName;
    s << "Changed On:" << sourceTypeName();
    s << "propagate path:" << propagatePath();
    s << "propagate deep:" << QString::number(propagateDeep());
    s << "has pre:" << (hasPre() ? "yes" : "no");
    s << "has next:" << (hasNext() ? "yes" : "no");
    return s.join(QChar('\n'));
}

QString ObservableChangeNotify::propagatePath() const {
    QStringList s;
    for (const auto &source: m_sourcePath) {
        s << QObservableObject::dynamicName(source);
    }
    return s.join(QStringView(u"->"));
}

qsizetype ObservableChangeNotify::propagateDeep() const {
    return m_sourcePath.count();
}

const QObservableObject *ObservableChangeNotify::firstSource() const {
    Q_ASSERT(!m_sourcePath.isEmpty());
    return m_sourcePath.at(0);
}

bool ObservableChangeNotify::hasPre() const {
    return static_cast<bool>(m_pre);
}

bool ObservableChangeNotify::hasNext() const {
    return static_cast<bool>(m_next);
}

const ObservableChangeNotify &ObservableChangeNotify::pre() const {
    Q_ASSERT(hasPre());
    return *m_pre.toStrongRef();
}

const ObservableChangeNotify &ObservableChangeNotify::next() const {
    Q_ASSERT(hasNext());
    return *m_next.data();
}

const ObservableChangeNotify &ObservableChangeNotify::last() const {
    auto last = this;
    while (last->m_next) {
        last = last->m_next.data();
    }
    return *last;
}

const ObservableChangeNotify &ObservableChangeNotify::first() const {
    auto first = this;
    while (first->m_pre) {
        first = first->m_pre.toStrongRef().data();
    }
    return *first;
}

ObservableChangeNotify::iterator ObservableChangeNotify::begin() const {
    return iterator(this);
}

ObservableChangeNotify::iterator ObservableChangeNotify::end() const {
    return iterator(nullptr);
}

void ObservableChangeNotify::appendSource(const QObservableObject *source) {
    m_sourcePath.append(source);
}

ObservableShotGuard::ObservableShotGuard(QObservableObject *observable, int shots) : m_object(observable) {
    m_lastShots = m_object->shots();
    m_object->setShot(shots);
}

ObservableShotGuard::ObservableShotGuard(ObservableShotGuard &&other) noexcept : m_object(std::move(other.m_object)) {
}

ObservableShotGuard &ObservableShotGuard::operator=(ObservableShotGuard &&other) noexcept {
    m_object = std::move(other.m_object);
    return *this;
}

ObservableShotGuard::~ObservableShotGuard() {
    dismiss();
}

void ObservableShotGuard::dismiss() {
    if (m_object) {
        m_object->setShot(m_lastShots);
        m_object = nullptr;
    }
}

ObservableChangeNotify::iterator::iterator(const ObservableChangeNotify *notify) : m_notify(
    const_cast<ObservableChangeNotify *>(notify)) {
}

const ObservableChangeNotify::iterator &ObservableChangeNotify::iterator::operator++() {
    m_notify = m_notify->m_next.get();
    return *this;
}

bool ObservableChangeNotify::iterator::operator!=(const iterator &other) const {
    return m_notify != other.m_notify;
}

const ObservableChangeNotify &ObservableChangeNotify::iterator::operator*() const {
    Q_ASSERT(m_notify);
    return *m_notify;
}
