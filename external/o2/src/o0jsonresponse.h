#ifndef O0JSONRESPONSE_H
#define O0JSONRESPONSE_H

#include <QVariantMap>

class QByteArray;

/// Parse JSON data into a QVariantMap
QVariantMap parseJsonResponse(const QByteArray &data);

#endif // O0JSONRESPONSE_H
