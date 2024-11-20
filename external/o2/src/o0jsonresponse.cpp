#include "o0jsonresponse.h"

#include <QByteArray>
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#include <QJsonObject>
#else
#include <QScriptEngine>
#include <QScriptValueIterator>
#endif

QVariantMap parseJsonResponse(const QByteArray &data) {
#if QT_VERSION >= 0x050000
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "parseTokenResponse: Failed to parse token response due to err:" << err.errorString();
        return QVariantMap();
    }

    if (!doc.isObject()) {
        qWarning() << "parseTokenResponse: Token response is not an object";
        return QVariantMap();
    }

    return doc.object().toVariantMap();
#else
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + QString(data) + ")");
    QScriptValueIterator it(value);
    QVariantMap map;

    while (it.hasNext()) {
        it.next();
        map.insert(it.name(), it.value().toVariant());
    }

    return map;
#endif
}
