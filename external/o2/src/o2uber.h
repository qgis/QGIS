#ifndef O2UBER_H
#define O2UBER_H

#include "o0export.h"
#include "o2.h"

class O0_EXPORT O2Uber: public O2{
    Q_OBJECT

public:
    O2Uber(QObject *parent = 0);

public Q_SLOTS:
    void onVerificationReceived(QMap<QString, QString>);

protected Q_SLOTS:
    virtual void onTokenReplyFinished();
};

#endif // O2UBER_H
