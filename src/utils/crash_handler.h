//
// Created by merlin
//

#pragma once

#include <QString>

class CrashHandler {
public:
    virtual void init(const QString& reportPath) = 0;
    virtual void requestDump() = 0;
    static CrashHandler* instance();
};
