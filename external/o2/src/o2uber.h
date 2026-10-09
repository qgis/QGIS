#ifndef O2UBER_H
#define O2UBER_H

#include "o0export.h"
#include "o2.h"

class O0_EXPORT O2Uber: public O2{
    Q_OBJECT

public:
    explicit O2Uber(QObject *parent = nullptr);

public slots:
    void onVerificationReceived(QMap<QString, QString>) override;

protected slots:
    void onTokenReplyFinished() override;
};

#endif // O2UBER_H
