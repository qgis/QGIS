/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayercombobox.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMapLayerComboBox : QComboBox
{
%Docstring
 The QgsMapLayerComboBox class is a combo box which displays the list of layers

.. versionadded:: 2.3
%End

%TypeHeaderCode
#include "qgsmaplayercombobox.h"
%End
  public:

    explicit QgsMapLayerComboBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
 QgsMapLayerComboBox creates a combo box to dislpay the list of layers (currently in the registry).
The layers can be filtered and/or ordered.
%End

    void setFilters( QgsMapLayerProxyModel::Filters filters );
%Docstring
setFilters allows fitering according to layer type and/or geometry type.
%End

    QgsMapLayerProxyModel::Filters filters() const;
%Docstring
currently used filter on list layers
%End

    void setExceptedLayerList( const QList<QgsMapLayer *> &layerList );
%Docstring
except a list of layers not to be listed
%End

    QList<QgsMapLayer *> exceptedLayerList() const;
%Docstring
returns the list of excepted layers
%End

    void setExcludedProviders( const QStringList &providers );
%Docstring
Sets a list of data providers which should be excluded from the combobox.

.. versionadded:: 3.0

.. seealso:: :py:func:`excludedProviders`
%End

    QStringList excludedProviders() const;
%Docstring
Returns the list of data providers which are excluded from the combobox.

.. seealso:: :py:func:`setExcludedProviders`

.. versionadded:: 3.0
%End

    void setAllowEmptyLayer( bool allowEmpty );
%Docstring
Sets whether an optional empty layer ("not set") option is shown in the combo box.

.. seealso:: :py:func:`allowEmptyLayer`

.. versionadded:: 3.0
%End

    bool allowEmptyLayer() const;
%Docstring
Returns true if the combo box allows the empty layer ("not set") choice.

.. seealso:: :py:func:`setAllowEmptyLayer`

.. versionadded:: 3.0
%End

    void setShowCrs( bool showCrs );
%Docstring
Sets whether the CRS of layers is also included in the combo box text.

.. seealso:: :py:func:`showCrs`

.. versionadded:: 3.0
%End

    bool showCrs() const;
%Docstring
Returns true if the combo box shows the layer's CRS.

.. seealso:: :py:func:`setShowCrs`

.. versionadded:: 3.0
%End

    void setAdditionalItems( const QStringList &items );
%Docstring
Sets a list of additional (non map layer) items to include at the end of the combobox.
These may represent additional layers such as layers which are not included in the map
layer registry, or paths to layers which have not yet been loaded into QGIS.

.. seealso:: :py:func:`additionalItems`

.. versionadded:: 3.0
%End

    QStringList additionalItems() const;
%Docstring
Return the list of additional (non map layer) items included at the end of the combo box.

.. seealso:: :py:func:`setAdditionalItems`

.. versionadded:: 3.0
%End

    QgsMapLayer *currentLayer() const;
%Docstring
Returns the current layer selected in the combo box.

.. seealso:: :py:func:`layer`
%End

    QgsMapLayer *layer( int layerIndex ) const;
%Docstring
Return the layer currently shown at the specified index within the combo box.

:param layerIndex: position of layer to return

.. versionadded:: 2.10

.. seealso:: :py:func:`currentLayer`
%End

  public slots:
    void setLayer( QgsMapLayer *layer );
%Docstring
setLayer set the current layer selected in the combo
%End

  signals:
    void layerChanged( QgsMapLayer *layer );
%Docstring
layerChanged this signal is emitted whenever the currently selected layer changes
%End

  protected slots:
    void indexChanged( int i );
    void rowsChanged();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayercombobox.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
