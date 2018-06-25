#ifndef O0SETTINGSSTORE_H
#define O0SETTINGSSTORE_H

#include <QSettings>
#include <QString>

#include "o0baseauth.h"
#include "o0abstractstore.h"
#include "o0simplecrypt.h"

/// Persistent storage for authentication tokens, using QSettings.
class O0_EXPORT O0SettingsStore: public O0AbstractStore {
    Q_OBJECT

public:
    /// Constructor
    explicit O0SettingsStore(const QString &encryptionKey, QObject *parent = 0);

    /// Construct with an explicit QSettings instance
    explicit O0SettingsStore(QSettings *settings, const QString &encryptionKey, QObject *parent = 0);

    /// Group key prefix
    Q_PROPERTY(QString groupKey READ groupKey WRITE setGroupKey NOTIFY groupKeyChanged)
    QString groupKey() const;
    void setGroupKey(const QString &groupKey);

    /// Get a string value for a key
    QString value(const QString &key, const QString &defaultValue = QString());

    /// Set a string value for a key
    void setValue(const QString &key, const QString &value);

Q_SIGNALS:
    // Property change signals
    void groupKeyChanged();

protected:
    QSettings* settings_;
    QString groupKey_;
    O0SimpleCrypt crypt_;
};

#endif // O0SETTINGSSTORE_H
