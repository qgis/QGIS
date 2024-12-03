#include "o0jsonresponse.h"
#include "o0baseauth.h"

#include <QByteArray>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QJsonDocument>
#include <QJsonObject>
#else
#include <QScriptEngine>
#include <QScriptValueIterator>
#endif

QVariantMap parseJsonResponse(const QByteArray &data) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        O0BaseAuth::log( QStringLiteral("parseTokenResponse: Failed to parse token response due to err: %1").arg( err.errorString() ), O0BaseAuth::LogLevel::Warning );
        return QVariantMap();
    }

    if (!doc.isObject()) {
        O0BaseAuth::log( QStringLiteral("parseTokenResponse: Token response is not an object"), O0BaseAuth::LogLevel::Warning );
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
