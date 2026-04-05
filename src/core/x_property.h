//
// Created by merlin
//

#ifndef X_PROPERTY_H
#define X_PROPERTY_H

#include <QProperty>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(qlcValidator)

class QObservableObject;

using ErrorHint = bool;
template<typename T>
class XProperty : public QProperty<T> {
public:
    using PropertyValidator = std::function<ErrorHint(const T &, const QObservableObject *owner)>;
    using Deleter = std::function<void(T &value)>;
    using QProperty<T>::QProperty;
    void setValidator(const PropertyValidator &validator);
    void setDeleter(const Deleter &deleter);
    ErrorHint validateProperty(const char *name, const QObservableObject *owner, const T &value);
    void reset();

    template<class P>
    void reset(P&& newVal);
    bool isEqual(const T& newVal);
    XProperty &operator=(typename QProperty<T>::rvalue_ref newValue);
    XProperty &operator=(typename QProperty<T>::parameter_type newValue);
    ~XProperty();

private:
    Deleter m_deleter;
    PropertyValidator m_validator;
};

template<typename T>
template<typename P>
void XProperty<T>::reset(P &&newVal) {
    if (m_deleter && !isEqual(newVal)) {
        m_deleter(this->val);
    }
}

template<typename T>
bool XProperty<T>::isEqual(const T &newVal) {
    if constexpr (QTypeTraits::has_operator_equal_v<T>) {
        return newVal == this->val;
    }
    return false;
}

template<typename T>
void XProperty<T>::setValidator(const PropertyValidator &validator) {
    m_validator = validator;
}

template<typename T>
void XProperty<T>::setDeleter(const Deleter &deleter) {
    m_deleter = deleter;
}

template<typename T>
ErrorHint XProperty<T>::validateProperty(const char *name, const QObservableObject *owner, const T &value) {
    Q_ASSERT(owner);
    ErrorHint state = true;
    if (m_validator) {
        state = m_validator(value, owner);
        if (!state) {
            qCWarning(qlcValidator) << "Property [" << name << "] validation failed on: " << owner;
        }
    }
    return state;
}

template<typename T>
XProperty<T> &XProperty<T>::operator=(typename QProperty<T>::rvalue_ref newValue) {
    this->setValue(std::move(newValue));
    return *this;
}

template<typename T>
XProperty<T> &XProperty<T>::operator=(typename QProperty<T>::parameter_type newValue) {
    this->setValue(newValue);
    return *this;
}

template<typename T>
XProperty<T>::~XProperty() {
    //reset is not a dependent names
    reset();
}

template<typename T>
void XProperty<T>::reset() {
    if (m_deleter) {
        m_deleter(this->val);
    }
}

template<typename T>
inline void HeapDeleter(T &value) {
    static_assert(std::is_pointer_v<T>, "T must be a pointer type");
    delete value;
    value = nullptr;
}

template<typename T>
inline void ArrayDeleter(T &value) {
    static_assert(std::is_pointer_v<T>, "T must be a pointer type of array");
    delete[] value;
    value = nullptr;
}

#endif //X_PROPERTY_H
