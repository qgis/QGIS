#ifndef O2FACEBOOK_H
#define O2FACEBOOK_H

#include "o0export.h"
#include "o2.h"

/// Facebook's dialect of OAuth 2.0
class O0_EXPORT O2Facebook: public O2 {
    Q_OBJECT

public:
    explicit O2Facebook(QObject *parent = nullptr);

public Q_SLOTS:
    void onVerificationReceived(QMap<QString, QString>) override;

protected Q_SLOTS:
    void onTokenReplyFinished() override;
};

#endif // O2FACEBOOK_H
