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
    QgsMapLayerProxyModel::Filters filters();

    //! currentLayer returns the current layer selected in the combo box
    QgsMapLayer* currentLayer();

  public slots:
    //! setLayer set the current layer selected in the combo
    void setLayer( QgsMapLayer* layer );

  signals:
    //! layerChanged this signal is emitted whenever the currently selected layer changes
    void layerChanged( QgsMapLayer* layer );
};
