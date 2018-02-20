/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldvalueslineedit.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFieldValuesLineEdit: QgsFilterLineEdit
{
%Docstring
A line edit with an autocompleter which takes unique values from a vector layer's fields.
The autocompleter is populated from the vector layer in the background to ensure responsive
interaction with the widget.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfieldvalueslineedit.h"
%End
  public:

    QgsFieldValuesLineEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsFieldValuesLineEdit

:param parent: parent widget
%End

    ~QgsFieldValuesLineEdit();

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets the layer containing the field that values will be shown from.

:param layer: vector layer

.. seealso:: :py:func:`layer`

.. seealso:: :py:func:`setAttributeIndex`
%End

    QgsVectorLayer *layer() const;
%Docstring
Returns the layer containing the field that values will be shown from.

.. seealso:: :py:func:`setLayer`

.. seealso:: :py:func:`attributeIndex`
%End

    void setAttributeIndex( int index );
%Docstring
Sets the attribute index for the field containing values to show in the widget.

:param index: index of attribute

.. seealso:: :py:func:`attributeIndex`

.. seealso:: :py:func:`setLayer`
%End

    int attributeIndex() const;
%Docstring
Returns the attribute index for the field containing values shown in the widget.

.. seealso:: :py:func:`setAttributeIndex`

.. seealso:: :py:func:`layer`
%End

  signals:

    void layerChanged( QgsVectorLayer *layer );
%Docstring
Emitted when the layer associated with the widget changes.

:param layer: vector layer
%End

    void attributeIndexChanged( int index );
%Docstring
Emitted when the field associated with the widget changes.

:param index: new attribute index for field
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldvalueslineedit.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
