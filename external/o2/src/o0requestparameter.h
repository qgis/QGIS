#ifndef O0REQUESTPARAMETER_H
#define O0REQUESTPARAMETER_H

#include "o0export.h"

/// Request parameter (name-value pair) participating in authentication.
struct O0_EXPORT O0RequestParameter {
    O0RequestParameter(const QByteArray &n, const QByteArray &v): name(n), value(v) {}
    bool operator <(const O0RequestParameter &other) const {
        return (name == other.name)? (value < other.value): (name < other.name);
    }
    QByteArray name;
    QByteArray value;
};

#endif // O0REQUESTPARAMETER_H
