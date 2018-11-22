#ifndef O0ABSTRACTSTORE_H
#define O0ABSTRACTSTORE_H

#include <QObject>
#include <QString>

#include "o0export.h"

/// Storage for strings.
class O0_EXPORT O0AbstractStore: public QObject {
    Q_OBJECT

public:
    explicit O0AbstractStore(QObject *parent = 0): QObject(parent) {
    }

    /// Retrieve a string value by key.
    virtual QString value(const QString &key, const QString &defaultValue = QString()) = 0;

    /// Set a string value for a key.
    virtual void setValue(const QString &key, const QString &value) = 0;
};

#endif // O0ABSTRACTSTORE_H
