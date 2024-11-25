//
// Created by michaelpollind on 3/13/17.
//
#include "o0keychainstore.h"
#include "o0baseauth.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif
#include <QtCore/QDataStream>
#include <QtCore/QBuffer>
#include <QtCore/QEventLoop>

using namespace QKeychain;

o0keyChainStore::o0keyChainStore(const QString& app,const QString& name,QObject *parent):
    O0AbstractStore(parent), app_(app),name_(name),pairs_()
{
}

QString o0keyChainStore::value(const QString &key, const QString &defaultValue) {
    return pairs_.value(key, defaultValue);
}

void o0keyChainStore::setValue(const QString &key, const QString &value) {
    pairs_.insert(key,value);
}

int o0keyChainStore::persist() {
    WritePasswordJob job(app_);
    initJob(job);

    QByteArray data;
    QDataStream ds(&data,QIODevice::ReadWrite);
    ds << pairs_;
    job.setBinaryData(data);

    return executeJob(job, "persist");
}

int o0keyChainStore::fetchFromKeychain() {
    ReadPasswordJob job(app_);
    initJob(job);
    const int result = executeJob(job, "fetch");
    if (result == 0) { // success
        QByteArray data;
        data.append(job.binaryData());
        QDataStream ds(&data, QIODevice::ReadOnly);
        ds >> pairs_;
    }
    return result;
}

int o0keyChainStore::clearFromKeychain() {
    DeletePasswordJob job(app_);
    initJob(job);
    return executeJob(job, "clear");
}

bool o0keyChainStore::isEntryNotFoundError(int errorCode) {
    return errorCode == QKeychain::EntryNotFound;
}

void o0keyChainStore::initJob(QKeychain::Job &job) const {
    job.setAutoDelete(false);
    job.setKey(name_);
}

int o0keyChainStore::executeJob(QKeychain::Job &job, const char *actionName) const {
    QEventLoop loop;
    job.connect(&job, &Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    const QKeychain::Error errorCode = job.error();
    if (errorCode != QKeychain::NoError) {
        O0BaseAuth::log( QStringLiteral("keychain store could not %1 %2: %3 (%4)").arg(
                            actionName, name_, job.errorString() )
                            .arg( errorCode ), O0BaseAuth::LogLevel::Warning );
    }
    return errorCode;
}
