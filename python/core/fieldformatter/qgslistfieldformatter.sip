/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgslistfieldformatter.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsListFieldFormatter : QgsFieldFormatter
{
%Docstring
Field formatter for a list field.
This represents a list type value.
Values will be represented as a comma-separated list.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslistfieldformatter.h"
%End
  public:

    QgsListFieldFormatter();
%Docstring
Default constructor of field formatter for a list field.
%End
    virtual QString id() const;


    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgslistfieldformatter.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
