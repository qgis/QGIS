#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2surveymonkey.h"
#include "o2globals.h"

static const char *SMEndpoint = "https://api.surveymonkey.net/oauth/authorize";
static const char *SMTokenUrl = "https://api.surveymonkey.net/oauth/token";
static const quint16 SMLocalPort = 8000;

O2SurveyMonkey::O2SurveyMonkey(QObject *parent): O2(parent) {
    setRequestUrl(SMEndpoint);
    setTokenUrl(SMTokenUrl);
    setLocalPort(SMLocalPort);
    setIgnoreSslErrors(true); //needed on Mac
}
