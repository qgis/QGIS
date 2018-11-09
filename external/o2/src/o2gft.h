#ifndef O2GFT_H
#define O2GFT_H

#include "o0export.h"
#include "o2google.h"

/// Google Fusion Tables' dialect of OAuth 2.0
class O0_EXPORT O2Gft: public O2Google{
    Q_OBJECT

public:
    explicit O2Gft(QObject *parent = 0);
};

#endif // O2GFT_H
