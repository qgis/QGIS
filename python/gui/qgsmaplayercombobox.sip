/**
 * @brief The QgsMapLayerComboBox class is a combo box which displays the list of layers
 * @note added in 2.3
 */
class QgsMapLayerComboBox : QComboBox
{

%TypeHeaderCode
#include "qgsmaplayercombobox.h"
%End

  public:
    /**
     * @brief QgsMapLayerComboBox creates a combo box to dislpay the list of layers (currently in the registry).
     * The layers can be filtered and/or ordered.
     */
    explicit QgsMapLayerComboBox( QWidget *parent /TransferThis/ = 0 );

    //! setFilters allows fitering according to layer type and/or geometry type.
    void setFilters( QgsMapLayerProxyModel::Filters filters );

    //! currently used filter on list layers
    QgsMapLayerProxyModel::Filters filters() const;

    //! except a list of layers not to be listed
    void setExceptedLayerList( QList<QgsMapLayer*> layerList );

    //! returns the list of excepted layers
    QList<QgsMapLayer*> exceptedLayerList() const;

    /** Returns the current layer selected in the combo box.
     * @see layer
     */
    QgsMapLayer* currentLayer() const;

    /** Return the layer currently shown at the specified index within the combo box.
     * @param layerIndex position of layer to return
     * @note added in QGIS 2.10
     * @see currentLayer
     */
    QgsMapLayer* layer( int layerIndex ) const;

  public slots:
    //! setLayer set the current layer selected in the combo
    void setLayer( QgsMapLayer* layer );

  signals:
    //! layerChanged this signal is emitted whenever the currently selected layer changes
    void layerChanged( QgsMapLayer* layer );
};
