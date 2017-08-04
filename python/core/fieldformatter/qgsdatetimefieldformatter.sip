/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsdatetimefieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsDateTimeFieldFormatter : QgsFieldFormatter
{
%Docstring
 Field formatter for a date time field.
 This represents a date, time or datetime value based on
 the field configuration.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsdatetimefieldformatter.h"
%End
  public:
    static const QString DEFAULT_DATE_FORMAT;
    static const QString DEFAULT_TIME_FORMAT;
    static const QString DEFAULT_DATETIME_FORMAT;

    QgsDateTimeFieldFormatter();
%Docstring
 Default constructor of field formatter for a date time field.
%End

    virtual QString id() const;

    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    static QString defaultFormat( QVariant::Type type );
%Docstring
 Get the default format in function of the type.
 The type is expected to be one of

 - QVariant.DateTime
 - QVariant.Date
 - QVariant.Time
 :rtype: str
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsdatetimefieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
