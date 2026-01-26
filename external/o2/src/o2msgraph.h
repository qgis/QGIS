#ifndef O2MSGRAPH_H
#define O2MSGRAPH_H

#include "o0export.h"
#include "o2.h"

/// Microsoft Graph's dialect of OAuth 2.0
class O0_EXPORT O2Msgraph: public O2 {
    Q_OBJECT

public:
    explicit O2Msgraph(QObject *parent = nullptr);
};

#endif // O2MSGRAPH_H
