//
// Created by michaelpollind on 3/13/17.
//
#ifndef O2_O0KEYCHAINSTORE_H
#define O2_O0KEYCHAINSTORE_H

#include <QtCore/QMap>
#include "o0abstractstore.h"
#include <QString>

class O0_EXPORT o0keyChainStore  : public  O0AbstractStore{
    Q_OBJECT
public:
    explicit o0keyChainStore(const QString& app,const QString& name,QObject *parent = 0);

    /// Retrieve a string value by key.
    virtual QString value(const QString &key, const QString &defaultValue = QString()) = 0;

    /// Set a string value for a key.
    virtual void setValue(const QString &key, const QString &value) = 0;

    void persist();
    void fetchFromKeychain();
    void clearFromKeychain();
private:
    QString app_;
    QString name_;
    QMap<QString,QString> pairs_;

};


#endif //O2_O0KEYCHAINSTORE_H
