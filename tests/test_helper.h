//
// Created by ml
//

#pragma once

#include <doctest/doctest.h>
#include <QVector2D>

constexpr bool approximateEqual(double a, double b, double epsilon = 0.0001) {
    return qFuzzyCompare(a, b) || doctest::Approx(a).epsilon(epsilon) == b;
}

template<typename... Args>
struct FunctionVerification {
    using CallInfo = std::tuple<int, std::decay_t<Args>...>;

    void operator()(Args... args) {
        *m_callInfo = std::make_tuple(std::get<0>(*m_callInfo) + 1, std::decay_t<Args>(args)...);
    }

    /*
     * get args N of last call, start from 1
     */
    template<int N>
    std::tuple_element_t<N + 1, CallInfo>& getArg() {
        static_assert(N == -1 || N < sizeof...(Args), "N out of range");
        return std::get<N + 1>(*m_callInfo);
    }

    int called() {
        const int called = std::get<0>(*m_callInfo);
        std::get<0>(*m_callInfo) = 0;
        return called;
    }

private:
    std::shared_ptr<CallInfo> m_callInfo = std::make_shared<CallInfo>();
};

#define CHECK_CALLED(FuncVerify) CHECK_MESSAGE((FuncVerify).called() > 0, "Expected function to be called")
#define CHECK_NOT_CALLED(FuncVerify) CHECK_MESSAGE((FuncVerify).called() == 0, "Expected function not to be called")
#define CHECK_CALLED_N(FuncVerify, n) CHECK_MESSAGE((FuncVerify).called() == (n), "Expected function to be called " #n " times")
