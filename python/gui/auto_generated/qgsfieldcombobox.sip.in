/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldcombobox.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsFieldComboBox : QComboBox
{
%Docstring
 The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
It might be combined with a QgsMapLayerComboBox to automatically update fields according to a chosen layer.
If expression must be used, QgsFieldExpressionWidget shall be used instead.

.. seealso:: :py:class:`QgsMapLayerComboBox`

.. versionadded:: 2.3
%End

%TypeHeaderCode
#include "qgsfieldcombobox.h"
%End
  public:

    explicit QgsFieldComboBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
 QgsFieldComboBox creates a combo box to display the fields of a layer.
The layer can be either manually given or dynamically set by connecting the signal QgsMapLayerComboBox.layerChanged to the slot setLayer.
%End

    void setFilters( QgsFieldProxyModel::Filters filters );
%Docstring
setFilters allows fitering according to the type of field
%End

    QgsFieldProxyModel::Filters filters() const;
%Docstring
currently used filter on list of fields
%End

    void setAllowEmptyFieldName( bool allowEmpty );
%Docstring
Sets whether an optional empty field ("not set") option is shown in the combo box.

.. seealso:: :py:func:`allowEmptyFieldName`

.. versionadded:: 3.0
%End

    bool allowEmptyFieldName() const;
%Docstring
Returns true if the combo box allows the empty field ("not set") choice.

.. seealso:: :py:func:`setAllowEmptyFieldName`

.. versionadded:: 3.0
%End

    QString currentField() const;
%Docstring
return the currently selected field
%End

    QgsVectorLayer *layer() const;
%Docstring
Returns the layer currently associated with the combobox.

.. seealso:: :py:func:`setLayer`
%End

  signals:
    void fieldChanged( const QString &fieldName );
%Docstring
the signal is emitted when the currently selected field changes
%End

  public slots:

    void setLayer( QgsMapLayer *layer );
%Docstring
Sets the layer for which fields are listed in the combobox. If no layer is set
or a non-vector layer is set then the combobox will be empty.

.. seealso:: :py:func:`layer`
%End

    void setField( const QString &fieldName );
%Docstring
setField sets the currently selected field
%End

  protected slots:
    void indexChanged( int i );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldcombobox.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
