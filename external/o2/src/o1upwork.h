#ifndef O1UPWORK_H
#define O1UPWORK_H

#include "o1.h"

class O1Upwork: public O1 {
    Q_OBJECT

public:
    explicit O1Upwork(QObject *parent = 0) : O1(parent) {
        setRequestTokenUrl(QUrl("https://www.upwork.com/api/auth/v1/oauth/token/request"));
        setAuthorizeUrl(QUrl("https://www.upwork.com/services/api/auth"));
        setAccessTokenUrl(QUrl("https://www.upwork.com/api/auth/v1/oauth/token/access"));
    }
};

#endif // O1UPWORK_H

