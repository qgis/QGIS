/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgskeyvaluefieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsKeyValueFieldFormatter : QgsFieldFormatter
{
%Docstring
Field formatter for a key value field.
This represents a list type value.
Values will be represented as a colon-delimited and
comma-separated list.

E.g. "color: yellow, amount: 5"

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgskeyvaluefieldformatter.h"
%End
  public:

    QgsKeyValueFieldFormatter();
%Docstring
Default constructor of field formatter for a key value field.
%End
    virtual QString id() const;

    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgskeyvaluefieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
