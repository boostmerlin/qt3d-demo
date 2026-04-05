//
// Created by merlin
//

#pragma once

#include <QList>

template<typename Out, typename In>
QList<Out> removeAdjacentElements(const QList<In> &input, std::function<Out(const In &)> transformer = [](
        const In &in) { return static_cast<Out>(in); }) {
    QList<Out> output;
    auto size = input.size();
    for (int i = 0; i < size; ++i) {
        if (approxEqual(input.at(i), input.at((i + 1) % size))) {
            continue;
        }
        output.append(transformer(input.at(i)));
    }
    return output;
}
