/**
 * @brief The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
 * It might be combined with a QgsMapLayerComboBox to automatically update fields according to a chosen layer.
 * If expression must be used, QgsFieldExpressionWidget shall be used instead.
 * @see QgsMapLayerComboBox
 * @note added in 2.3
 */
class QgsFieldComboBox : QComboBox
{

%TypeHeaderCode
#include "qgsfieldcombobox.h"
%End

  public:
    /**
     * @brief QgsFieldComboBox creates a combo box to display the fields of a layer.
     * The layer can be either manually given or dynamically set by connecting the signal QgsMapLayerComboBox::layerChanged to the slot setLayer.
     */
    explicit QgsFieldComboBox( QWidget *parent /TransferThis/ = 0 );

    //! setFilters allows fitering according to the type of field
    void setFilters( QgsFieldProxyModel::Filters filters );

    //! currently used filter on list of fields
    QgsFieldProxyModel::Filters filters() const;

    //! return the currently selected field
    QString currentField() const;

    //! Returns the currently used layer
    QgsVectorLayer* layer() const;

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( QString fieldName );

  public slots:
    //! set the layer of which the fields are listed
    void setLayer( QgsVectorLayer* layer );

    //! convenience slot to connect QgsMapLayerComboBox layer signal
    void setLayer( QgsMapLayer* layer );

    //! setField sets the currently selected field
    void setField( QString fieldName );
};
