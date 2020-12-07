#ifndef O2SURVEYMONKEY_H
#define O2SURVEYMONKEY_H

#include "o0export.h"
#include "o2.h"

/// SurveyMonkey's dialect of OAuth 2.0
class O0_EXPORT O2SurveyMonkey: public O2 {
    Q_OBJECT

public:
    explicit O2SurveyMonkey(QObject *parent = 0);
};

#endif // O2SURVEYMONKEY_H
