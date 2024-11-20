//
// Created by michaelpollind on 3/13/17.
//
#ifndef O2_O0KEYCHAINSTORE_H
#define O2_O0KEYCHAINSTORE_H

#include <QtCore/QMap>
#include "o0abstractstore.h"
#include <QString>

namespace QKeychain {
class Job;
}

/// Calling persist(), fetchFromKeychain() and clearFromKeychain() member
/// functions is the responsibility of the user of this class.
/// This is important to minimize the number of keychain accesses (and
/// potentially the number of user password prompts).
/// For example: fetchFromKeychain() can be called immediately after
/// creating a keychain store; persist() - after a successful authorization;
/// clearFromKeychain() - when the user logs out from the service.
class O0_EXPORT o0keyChainStore  : public  O0AbstractStore{
    Q_OBJECT
public:
    explicit o0keyChainStore(const QString& app,const QString& name,QObject *parent = 0);

    /// Retrieve a string value by key.
    QString value(const QString &key, const QString &defaultValue = QString());

    /// Set a string value for a key.
    void setValue(const QString &key, const QString &value);

    // The functions below return QKeychain::Error casted to int. They don't
    // return the enumerator directly because it can not be forward-declared reliably,
    // and including <keychain.h> into this header may be undesirable.
    // Note that if 0 is returned, then there was no error.

    int persist();
    int fetchFromKeychain();
    int clearFromKeychain();

    /// @return true if @p errorCode is equal to QKeychain::EntryNotFound.
    /// @note This function can be used to single out one type of an error
    /// returned from the functions above without including <keychain.h>.
    /// The EntryNotFound error type is special because it can be considered
    /// not an error if returned from clearFromKeychain().
    static bool isEntryNotFoundError(int errorCode);

private:
    void initJob(QKeychain::Job &job) const;
    int executeJob(QKeychain::Job &job, const char *actionName) const;

    QString app_;
    QString name_;
    QMap<QString,QString> pairs_;

};


#endif //O2_O0KEYCHAINSTORE_H
