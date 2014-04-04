/**
 * @brief The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
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

    /**
     * @brief currentField returns the currently selected field
     */
    QString currentField();

  signals:
    /**
     * @brief fieldChanged the signal is emitted when the currently selected field changes
     */
    void fieldChanged( QString fieldName );

  public slots:
    /**
     * @brief setLayer sets the layer of which the fields are listed
     */
    void setLayer( QgsMapLayer* layer );
    /**
     * @brief setField sets the currently selected field
     */
    void setField( QString fieldName );

};
