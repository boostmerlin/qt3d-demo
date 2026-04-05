//
// Created by merlin
//

#ifndef X_PROPERTY_VALIDATOR_H
#define X_PROPERTY_VALIDATOR_H

#include "x_property.h"

struct XValidationError final : std::logic_error {
    explicit XValidationError(const QString &message, int code = -1) : std::logic_error(message.toStdString().c_str()),
                                                                       errCode(code) {
    }

    int errCode;
};

template<typename T>
struct XValidator : XProperty<T>::PropertyValidator {
#define BEGIN_VALIDATE(Type, value, owner) \
    auto _validator_=[=,*this](const Type &(value), const QObservableObject *(owner)) {
#define END_VALIDATE }; this->reset(_validator_);
    using Validator = typename XProperty<T>::PropertyValidator;
    XValidator() noexcept = default;
    XValidator(decltype(nullptr)) noexcept : Validator(nullptr) {} // NOLINT(*-explicit-constructor)
    XValidator(const Validator &validator) : XProperty<T>::PropertyValidator(validator) {} // NOLINT(*-explicit-constructor)

    void reset(const Validator &validator) {
        Validator(validator).swap(*this);
    }
};

template<typename T>
struct XVAnd : XValidator<T> {
    XVAnd(const XValidator<T> &v1, const XValidator<T> &v2) {
        BEGIN_VALIDATE(T, value, owner)
            if (const ErrorHint ret = v1(value, owner); !ret) {
                return ret;
            }
            return v2(value, owner);
        END_VALIDATE
    }
};

template<typename T>
struct XVOr : XValidator<T> {
    XVOr(const XValidator<T> &v1, const XValidator<T> &v2) {
        BEGIN_VALIDATE(T, value, owner)
            try {
                if (const ErrorHint ret = v1(value, owner); ret) {
                    return ret;
                }
            } catch (...) {
                return v2(value, owner);
            }
            return v2(value, owner);
        END_VALIDATE
    }
};

template<typename T>
struct XVNot : XValidator<T> {
    explicit XVNot(const XValidator<T> &v) {
        BEGIN_VALIDATE(T, value, owner)
            try {
                return !v(value, owner);
            } catch (...) {
                return true;
            }
        END_VALIDATE
    }

    XVNot(const XValidator<T> &v, const XValidationError &exception) {
        BEGIN_VALIDATE(T, value, owner)
            bool ok;
            try {
                ok = v(value, owner);
            } catch (...) {
                ok = false;
            }
            if (ok) {
                throw exception;
            }
            return true;
        END_VALIDATE
    }
};

template<typename T>
XVAnd<T> operator&(const XValidator<T> &v1, const XValidator<T> &v2) {
    return XVAnd<T>(v1, v2);
}

template<typename T>
XVOr<T> operator|(const XValidator<T> &v1, const XValidator<T> &v2) {
    return XVOr<T>(v1, v2);
}

template<typename T>
XVNot<T> operator!(const XValidator<T> &v) {
    return XVNot<T>(v);
}

template<typename T, bool except = false>
struct XVEqual : XValidator<T> {
    explicit XVEqual(T targetValue) {
        BEGIN_VALIDATE(T, value, _)
            if (value == targetValue) {
                return true;
            }
            auto message = QStringLiteral("[XVEqual] value: %1 not equal to expected: %2").arg(value).arg(targetValue);
            if constexpr (except) {
                throw XValidationError(message);
            }
            qCWarning(qlcValidator) << message;
            return false;
        END_VALIDATE
    }
};

template<typename T, bool except = false>
struct XVRange : XValidator<T> {
    XVRange(const T &min, const T &max, bool inclusiveMin = true, bool inclusiveMax = true) {
        BEGIN_VALIDATE(T, value, _)
            if ((value > min || inclusiveMin && value == min) && (
                    value < max || inclusiveMax && value <= max)) {
                return true;
            }
            auto message = QStringLiteral("[XVRange] value out of range:%1 not in %2%3,%4%5").arg(value).arg(
                    inclusiveMin ? '[' : '(').arg(min).arg(max).arg(inclusiveMax ? ']' : ')');
            if constexpr (except) {
                throw XValidationError(message);
            }
            qCWarning(qlcValidator) << message;
            return false;
        END_VALIDATE
    }
};

template<typename T, bool except = false>
struct XVLess : XValidator<T> {
    explicit XVLess(const T &min, bool inclusive = false) {
        BEGIN_VALIDATE(T, value, _)
            if (value < min || inclusive && value == min) {
                return true;
            }
            auto message = QStringLiteral("[XVLess] value out of range: %1 should <%2 %3").arg(value).arg(
                    inclusive ? "=" : "").arg(min);
            if constexpr (except) {
                throw XValidationError(message);
            }
            qCWarning(qlcValidator) << message;
            return false;
        END_VALIDATE
    }
};

template<typename T, bool except = false>
struct XVGreater : XValidator<T> {
    explicit XVGreater(const T &max, bool inclusive = false) {
        BEGIN_VALIDATE(T, value, _)
            if (value > max || inclusive && value == max) {
                return true;
            }
            auto message = QStringLiteral("[XVGreater] value out of range: %1 should >%2 %3").arg(value).arg(
                    inclusive ? "=" : "").arg(max);
            if constexpr (except) {
                throw XValidationError(message);
            }
            qCWarning(qlcValidator) << message;
            return false;
        END_VALIDATE
    }
};

#endif //X_PROPERTY_VALIDATOR_H
