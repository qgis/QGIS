/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsvaluerelationfieldformatter.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsValueRelationFieldFormatter : QgsFieldFormatter
{
%Docstring
 Field formatter for a value relation field.
 A value relation field formatter looks up the values from
 features on another layer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvaluerelationfieldformatter.h"
%End
  public:
    struct ValueRelationItem
    {
      ValueRelationItem( const QVariant &key, const QString &value );
%Docstring
Constructor for ValueRelationItem
%End

      ValueRelationItem();
%Docstring
Constructor for ValueRelationItem
%End

      QVariant key;
      QString value;
    };

    typedef QVector < QgsValueRelationFieldFormatter::ValueRelationItem > ValueRelationCache;

    QgsValueRelationFieldFormatter();
%Docstring
 Constructor for QgsValueRelationFieldFormatter.
%End

    virtual QString id() const;

    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;


    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;


    virtual QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;


    static QgsValueRelationFieldFormatter::ValueRelationCache createCache( const QVariantMap &config );
%Docstring
 Create a cache for a value relation field.
 This can be used to keep the value map in the local memory
 if doing multiple lookups in a loop.

.. versionadded:: 3.0
 :rtype: QgsValueRelationFieldFormatter.ValueRelationCache
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsvaluerelationfieldformatter.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
