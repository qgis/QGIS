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

    /**
     * Sets whether an optional empty field ("not set") option is shown in the combo box.
     * @see allowEmptyFieldName()
     * @note added in QGIS 3.0
     */
    void setAllowEmptyFieldName( bool allowEmpty );

    /**
     * Returns true if the combo box allows the empty field ("not set") choice.
     * @see setAllowEmptyFieldName()
     * @note added in QGIS 3.0
     */
    bool allowEmptyFieldName() const;

    //! return the currently selected field
    QString currentField() const;

    /**
     * Returns the layer currently associated with the combobox.
     * @see setLayer()
     */
    QgsVectorLayer* layer() const;

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( const QString& fieldName );

  public slots:

    /**
     * Sets the layer for which fields are listed in the combobox. If no layer is set
     * or a non-vector layer is set then the combobox will be empty.
     * @see layer()
     */
    void setLayer( QgsMapLayer* layer );

    //! setField sets the currently selected field
    void setField( const QString& fieldName );

  protected slots:
    void indexChanged( int i );
};
