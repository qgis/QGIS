/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldformatter.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFieldFormatter
{
%Docstring
A field formatter helps to handle and display values for a field.

It allows for using a shared configuration with the editor widgets
for representation of attribute values.
Field kits normally have one single instance which is managed by the
QgsFieldFormatterRegistry. Custom field formatters should be registered there and
field formatters for use within code should normally be obtained from there.

This is an abstract base class and will always need to be subclassed.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfieldformatter.h"
%End
  public:
    QgsFieldFormatter();

    virtual ~QgsFieldFormatter();

    virtual QString id() const = 0;
%Docstring
Returns a unique id for this field formatter.
This id will later be used to identify this field formatter in the registry with :py:func:`QgsFieldFormatterRegistry.fieldFormatter()`

This id matches the id of a :py:class:`QgsEditorWidgetFactory`.
%End

    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;
%Docstring
Create a pretty String representation of the value.

:return: By default the string representation of the provided value as implied by the field definition is returned.

.. versionadded:: 3.0
%End

    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;
%Docstring
If the default sort order should be overwritten for this widget, you can transform the value in here.

:return: an unmodified value by default.

.. versionadded:: 3.0
%End

    virtual Qt::AlignmentFlag alignmentFlag( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;
%Docstring
Returns the alignment for a particular field. By default this will consider the field type but can be overwritten if mapped
values are represented.

.. versionadded:: 3.0
%End

    virtual QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;
%Docstring
Create a cache for a given field.

This will be used in situations where a field is being represented various times in a loop. And will be passed
to other methods on QgsFieldKit and :py:class:`QgsEditorWidgetWrapper`.

For example, the attribute table will create a cache once for each field and then use this
cache for representation. The QgsValueRelationFieldFormatter and :py:class:`QgsValueRelationEditorWidget`
implement this functionality to create a lookuptable once (a QVariantMap / dict) and are
make use of a cache if present.

.. versionadded:: 3.0
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfieldformatter.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
