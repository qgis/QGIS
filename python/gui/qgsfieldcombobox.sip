/**
 * @brief The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
 * If allowed, expression may also be added at the bottom of the list of fields.
 * It can be combined with a QgsExpressioButton to set the expression using the dedicated expression builder dialog.
 * It might also be combined with a QgsMapLayerComboBox and the fields will be automatically updated according to the chosen layer.
 * @see QgsExpressioButton, QgsMapLayerComboBox
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
     * @brief currentField returns the currently selected field or expression if allowed
     * @param isExpression determines if the string returned is the name of a field or an expression
     */
    QString currentField( bool *isExpression = 0 );
    
    //!! setAllowExpression sets if expression can be added the combo box
    void setAllowExpression( bool allowExpression );
    //! returns if the widget allows expressions to be added or not
    bool allowExpression();
    
    //! Returns the currently used layer
    QgsVectorLayer* layer();

  signals:
    /**
     * @brief setField sets the currently selected field
     * if expressions are allowed in the widget,
     * then it will either set it as selected
     * if it already exists, or it will add it otherwise
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
