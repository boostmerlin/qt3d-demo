//
// Created by merlin
//

#ifndef QOBSERVABLE_OBJECT_H
#define QOBSERVABLE_OBJECT_H

#include <QObject>
#include <QPointer>
#include <QMetaProperty>
#include "x_property.h"

Q_DECLARE_LOGGING_CATEGORY(qlcObservable)

class ObservableChangeNotify;

class ObservableShotGuard;

class QObservableObject : public QObject {
Q_OBJECT

public:
    enum {
        ENABLE_FOREVER = 0,
        DISABLE_FOREVER = -1,
    };

    static const char *dynamicName(const QObservableObject *source);

#define STATIC_NAME_OF(Type) Type::staticMetaObject.className()

    explicit QObservableObject(QObject *parent = nullptr);

    ~QObservableObject() override;

    void setShot(int shots = 1);

    [[nodiscard]] int shots() const;

    ObservableShotGuard setGuardedShot(int shots);

    ///set if the property change event should be propagated to the parent
    void setPropagation(bool propagation);

    void setObservableParent(const QObservableObject *observableParent, bool rechain = false, bool recursive = true);

    void removeObservableParent(const QObservableObject *observableParent);

    static void observableChain(QObservableObject *qObject, bool recursive);

    void observableChain(bool recursive = true);

    [[nodiscard]] bool isShotEnable() const;

    void disableShot();

    ObservableShotGuard disableShotGuarded();

    void enableShot();

    ObservableShotGuard enableShotGuarded();

    void disableShotOnce();

    //当前节点的属性变化分组，直到endGroupChange才通知，不影响父节点和子节点
    void beginGroupChange();

    //停止当前节点的属性变化分组，并发送信号
    void endGroupChange();

    [[nodiscard]] bool isGroupChangeBegin() const;

signals:

    void propertyChanged(const ObservableChangeNotify &params);

protected:
    virtual bool beforePropertySet(const char *name, const QVariant &value) { return true; }

    virtual void afterPropertySet(const char *name) {}

private:
    bool checkGroupChangeState() const;

    void onPropertyChanged(ObservableChangeNotify &params);

    void saveChangeNotify(const ObservableChangeNotify &params) const;

    void propagateChange(ObservableChangeNotify &params);

private:
    QList<QPointer<QObservableObject> > m_observableParents;
    std::vector<QPropertyNotifier> m_propertyNotifiers;
    QScopedPointer<ObservableChangeNotify> m_groupChangeNotify;
    int m_shots;
    bool m_propagation;
};

class ObservableShotGuard {
public:
    explicit ObservableShotGuard(QObservableObject *observable, int shots);

    //move:
    ObservableShotGuard(ObservableShotGuard &&) noexcept;

    ObservableShotGuard &operator=(ObservableShotGuard &&) noexcept;

    ~ObservableShotGuard();

    void dismiss();

private:
    Q_DISABLE_COPY(ObservableShotGuard);
    QPointer<QObservableObject> m_object;
    int m_lastShots{};
};

class ObservableChangeNotify {
public:
    class iterator {
    public:
        explicit iterator(const ObservableChangeNotify *notify);

        const iterator &operator++();

        bool operator!=(const iterator &other) const;

        const ObservableChangeNotify &operator*() const;

    private:
        ObservableChangeNotify *m_notify;
    };

    ObservableChangeNotify() = default;

    ObservableChangeNotify(const char *propertyName, const char *propertyType);

    template<typename T = QObservableObject>
    [[nodiscard]] const T *propagateSource(int i = 0) const {
        Q_ASSERT_X(i < m_sourcePath.size(), "PropertyChangeNotify", "index out of range");
        return qobject_cast<const T *>(m_sourcePath.at(i));
    }

    template<typename T>
    T propertyValue() const {
        const auto &value = firstSource()->property(m_propertyName);
        Q_ASSERT_X(value.canConvert<T>(), "PropertyChangeNotify", "value can't convert to T");
        return value.value<T>();
    }

    // 检查指定的类型是否在传播路径中
    template<typename First, typename... Rest>
    [[nodiscard]] bool checkOnPropagatePath() const {
        static_assert(
                (std::is_base_of_v<QObservableObject, First> && ... && std::is_base_of_v<QObservableObject, Rest>),
                "First and Rest must be QObservableObject");
        QStringList s;
        s << STATIC_NAME_OF(First);
        if constexpr (sizeof...(Rest) > 0) {
            (s << ... << STATIC_NAME_OF(Rest));
        }
        return propagatePath().contains(s.join("->"));
    }

    // 检查指定的对象是否在传播路径中
    template<typename First, typename... Rest>
    [[nodiscard]] bool checkOnPropagatePath(First const *first, Rest const *... rest) const {
        static_assert(
                (std::is_base_of_v<QObservableObject, First> && ... && std::is_base_of_v<QObservableObject, Rest>),
                "First and Rest must be QObservableObject");
        auto itSource = m_sourcePath.cbegin();
        bool matched = false;
        while (itSource != m_sourcePath.cend()) {
            if (*itSource++ == first) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            return false;
        }
        if constexpr (sizeof...(Rest)) {
            std::array<const QObservableObject *, sizeof...(Rest)> arr = {rest...};
            auto itDest = arr.cbegin();
            while (*itSource == *itDest) {
                ++itSource;
                ++itDest;
                if (itDest == arr.cend()) {
                    break;
                }
                if (itSource == m_sourcePath.cend()) {
                    matched = false;
                    break;
                }
            }
        }

        return matched;
    }

    [[nodiscard]] QString sourceTypeName() const;

    [[nodiscard]] QString propertyName() const;

    [[nodiscard]] QString propertyTypeName() const;

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] QString toString() const;

    //不能保证是完整的传播路径，只到当前监听点
    [[nodiscard]] QString propagatePath() const;

    [[nodiscard]] qsizetype propagateDeep() const;

    [[nodiscard]] const QObservableObject *firstSource() const;

    [[nodiscard]] bool hasPre() const;

    [[nodiscard]] bool hasNext() const;

    [[nodiscard]] const ObservableChangeNotify &pre() const;

    [[nodiscard]] const ObservableChangeNotify &next() const;

    [[nodiscard]] const ObservableChangeNotify &last() const;

    [[nodiscard]] const ObservableChangeNotify &first() const;

    [[nodiscard]] iterator begin() const;

    [[nodiscard]] iterator end() const;

private:
    friend class QObservableObject;

    void appendSource(const QObservableObject *source);

    QList<const QObservableObject *> m_sourcePath;
    //copy when use
    const char *m_propertyName{};
    const char *m_propertyTypeName{};

    QSharedPointer<ObservableChangeNotify> m_next;
    QWeakPointer<ObservableChangeNotify> m_pre;
};

#define CLASS_MEMBER_NAME(name) m_##name
#define GETTER_NAME(name) get##name
#define SETTER_NAME(name) set##name
#define X_PROPERTY_(Type) XProperty<Type>
#define NAME_OF(name) #name

#define WRITER_CODE_TEMPLATE_(Type, name, param) \
    X_PROPERTY_(Type) &p = CLASS_MEMBER_NAME(name); \
    const char* pname = NAME_OF(name); \
    auto state = p.validateProperty(pname, this, reinterpret_cast<X_PROPERTY_(Type)::parameter_type&>(value));                   \
    state = state && beforePropertySet(pname, QVariant::fromValue(value));           \
    if (state) { p.reset(param); p.setValue(param); afterPropertySet(pname); } \
    return state;

#define DECL_WRITER_(Writer, Type, name) \
ErrorHint Writer(X_PROPERTY_(Type)::parameter_type value) { \
    WRITER_CODE_TEMPLATE_(Type, name, value)                                  \
} \
ErrorHint Writer(X_PROPERTY_(Type)::rvalue_ref value) {     \
    if constexpr (std::is_rvalue_reference_v<XProperty<Type>::rvalue_ref>) { \
        WRITER_CODE_TEMPLATE_(Type, name, std::move(value))           \
    }                                     \
    else {                               \
        return ErrorHint();                        \
    }                                    \
}

#define DECL_READER_(Reader, Type, name) X_PROPERTY_(Type)::parameter_type Reader() const { return CLASS_MEMBER_NAME(name); }
#define DECL_VALIDATOR_(Writer, Type, name) \
    void Writer##Validator(const typename X_PROPERTY_(Type)::PropertyValidator &validator) { \
        CLASS_MEMBER_NAME(name).setValidator(validator); \
    }

#define X_BINDABLE_2(Type, name) \
QBindable<Type> name##Bindable() const { return &CLASS_MEMBER_NAME(name); }

#define X_BINDABLE_3(Type, name, Bindable) \
QBindable<Type> Bindable() const { return &CLASS_MEMBER_NAME(name); }

//生成Bindable返回给Q_PROPERTY用, (类型，名字,[函数名])
#define X_BINDABLE(...) QT_OVERLOADED_MACRO(X_BINDABLE, __VA_ARGS__)

#define X_BINDABLE_PROPERTY_2(Type, name) X_PROPERTY_(Type) CLASS_MEMBER_NAME(name) {}
#define X_BINDABLE_PROPERTY_3(Type, name, initValue) X_PROPERTY_(Type) CLASS_MEMBER_NAME(name) {initValue}

//声明一个QProperty, (类型，名字,[初始值])
#define X_BINDABLE_PROPERTY(...) QT_OVERLOADED_MACRO(X_BINDABLE_PROPERTY, __VA_ARGS__)

#define X_PROPERTY_4(Type, name, Reader, Writer) \
Q_PROPERTY(Type name BINDABLE name##Bindable READ Reader WRITE Writer)          \
public:                                \
    X_BINDABLE_2(Type, name)                                     \
    DECL_READER_(Reader, Type, name)                                       \
    DECL_WRITER_(Writer, Type, name)                               \
    DECL_VALIDATOR_(Writer, Type, name)                            \
private:                                          \
    X_BINDABLE_PROPERTY_2(Type, name);

#define X_PROPERTY_2(Type, name) X_PROPERTY_4(Type, name, GETTER_NAME(name), SETTER_NAME(name))

#define X_PROPERTY_6(Type, name, initValue, Reader, Writer, Rest) \
Q_PROPERTY(Type name BINDABLE name##Bindable READ Reader WRITE Writer Rest)          \
public:                                                                               \
    X_BINDABLE_2(Type, name)                     \
    DECL_READER_(Reader, Type, name)             \
    DECL_WRITER_(Writer, Type, name)             \
    DECL_VALIDATOR_(Writer, Type, name)          \
private:                                         \
    X_BINDABLE_PROPERTY_3(Type, name, initValue);
#define X_PROPERTY_5(Type, name, initValue, Reader, Writer) X_PROPERTY_6(Type, name, initValue, Reader, Writer,)
#define X_PROPERTY_3(Type, name, initValue) X_PROPERTY_5(Type, name, initValue, GETTER_NAME(name), SETTER_NAME(name))

#define X_BINDABLE_PROPERTY_PAREN(Type, name, initValue) X_PROPERTY_(Type) CLASS_MEMBER_NAME(name) {QT_VA_ARGS_EXPAND initValue}
#define X_PROPERTY_PAREN_6(Type, name, initValue, Reader, Writer, Rest)               \
Q_PROPERTY(Type name BINDABLE name##Bindable READ Reader WRITE Writer Rest)           \
public:                                                                               \
    X_BINDABLE_2(Type, name)                                                          \
    DECL_READER_(Reader, Type, name)                           \
    DECL_WRITER_(Writer, Type, name)                           \
    DECL_VALIDATOR_(Writer, Type, name)                        \
private:                                                       \
    X_BINDABLE_PROPERTY_PAREN(Type, name, initValue);

#define X_PROPERTY_PAREN_5(Type, name, initValue, Reader, Writer) X_PROPERTY_PAREN_6(Type, name, initValue, Reader, Writer,)
#define X_PROPERTY_PAREN_3(Type, name, initValue) X_PROPERTY_PAREN_5(Type, name, initValue, GETTER_NAME(name), SETTER_NAME(name))

#define X_PROPERTY(...) QT_OVERLOADED_MACRO(X_PROPERTY, __VA_ARGS__)

#define PAREN(...) (__VA_ARGS__)
//! 没有2参数和4参数的版本
#define X_PROPERTY_PAREN(...) QT_OVERLOADED_MACRO(X_PROPERTY_PAREN, __VA_ARGS__)

#define IS_PROPERTY(name_str, prop) QString(name_str) == NAME_OF(prop)
#define ANY_PROPERTY(name_str, ...) \
[](const QString& name, const QString& params) {         \
     auto ss = params.split(", ");                       \
     return std::any_of(ss.cbegin(), ss.cend(), [&name](auto s){return s == name;}); \
}(name_str, #__VA_ARGS__)

#define X_META_BEGIN private: bool _=(
#define X_VALIDATOR(property, validator) CLASS_MEMBER_NAME(property).setValidator(validator),
#define X_DELETER_1(property) CLASS_MEMBER_NAME(property).setDeleter \
                              (HeapDeleter<decltype(CLASS_MEMBER_NAME(property))::value_type>),
#define X_DELETER_2(property, deleter) CLASS_MEMBER_NAME(property).setDeleter(deleter),
#define X_DELETER(...) QT_OVERLOADED_MACRO(X_DELETER, __VA_ARGS__)
#define X_META_END true);

#endif //QOBSERVABLE_OBJECT_H
