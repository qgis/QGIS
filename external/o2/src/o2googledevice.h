#ifndef O2GOOGLEDEVICE_H
#define O2GOOGLEDEVICE_H

#include "o0export.h"
#include "o2.h"

/// "Google Sign-In for TVs and Devices",
/// A dialect of RFC 8628: OAuth 2.0 Device Authorization Grant
class O0_EXPORT O2GoogleDevice : public O2 {
    Q_OBJECT

public:
    explicit O2GoogleDevice(QObject *parent = nullptr);
};

#endif // O2GOOGLEDEVICE_H
