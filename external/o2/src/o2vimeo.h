#ifndef O2VIMEO_H
#define O2VIMEO_H

#include "o0export.h"
#include "o2.h"

/// Vimeo dialect of OAuth 2.0
class O0_EXPORT O2Vimeo : public O2 {
    Q_OBJECT

public:
    explicit O2Vimeo(QObject *parent = nullptr);
};

#endif // O2VIMEO_H
