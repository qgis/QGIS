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


    static QStringList valueToStringList( const QVariant &value );
%Docstring
Utility to convert an array or a string representation of an array ``value`` to a string list

.. versionadded:: 3.2
%End

    static QgsValueRelationFieldFormatter::ValueRelationCache createCache( const QVariantMap &config, const QgsFeature &formFeature = QgsFeature() );
%Docstring
Create a cache for a value relation field.
This can be used to keep the value map in the local memory
if doing multiple lookups in a loop.

:param config: The widget configuration
:param formFeature: The feature currently being edited with current attribute values

:return: A kvp list of values for the widget

.. versionadded:: 3.0
%End

    static bool expressionRequiresFormScope( const QString &expression );
%Docstring
Check if the ``expression`` requires a form scope (i.e. if it uses fields
or geometry of the currently edited feature).

:param expression: The widget's filter expression

:return: ``True`` if the expression requires a form scope

.. versionadded:: 3.2
%End

    static QSet<QString> expressionFormAttributes( const QString &expression );
%Docstring
Returns a list of attributes required by the form context ``expression``

:param expression: Form filter expression

:return: list of attributes required by the expression

.. versionadded:: 3.2
%End

    static QSet<QString> expressionFormVariables( const QString &expression );
%Docstring
Returns a list of variables required by the form context ``expression``

:param expression: Form filter expression

:return: list of variables required by the expression

.. versionadded:: 3.2
%End

    static bool expressionIsUsable( const QString &expression, const QgsFeature &feature );
%Docstring
Check whether the ``feature`` has all values required by the ``expression``

:return: ``True`` if the expression can be used

.. versionadded:: 3.2
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/fieldformatter/qgsvaluerelationfieldformatter.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
