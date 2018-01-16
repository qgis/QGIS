/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsvaluemapfieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsValueMapFieldFormatter : QgsFieldFormatter
{
%Docstring
Field formatter for a ValueMap field.
A value relation field formatter looks up the values a map.

The map is defined in the configuration as dictionary under the key "map".

{ "map": { 1: "one", 2: "two", 3: "three" } }

Values that are not on the map will be wrapped in parentheses. So with the above
configuration:

- 3 => "three"
- 5 => "(5)"

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvaluemapfieldformatter.h"
%End
  public:

    static const QString NULL_VALUE;

    QgsValueMapFieldFormatter();
%Docstring
Default constructor of field formatter for a value map field.
%End

    virtual QString id() const;


    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;


    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsvaluemapfieldformatter.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
