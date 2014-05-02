
class QgsFieldExpressionWidget : QWidget
{
%TypeHeaderCode
#include "qgsfieldexpressionwidget.h"
%End

  public:
    /**
     * @brief QgsFieldExpressionWidget creates a widget with a combo box to display the fields and expression and a button to open the expression dialog
     */
    explicit QgsFieldExpressionWidget( QWidget *parent /TransferThis/ = 0 );

    //! define the title used in the expression dialog
    void setExpressionDialogTitle( QString title );

    //! set the geometry calculator used in the expression dialog
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * @brief currentField returns the currently selected field or expression if allowed
     * @param isExpression determines if the string returned is the name of a field or an expression
     */
    QString currentField( bool *isExpression = 0 );

    //! Returns the currently used layer
    QgsVectorLayer* layer();

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( QString fieldName );

    //! fieldChanged signal with indication of the validity of the expression
    void fieldChanged( QString fieldName, bool isValid );

  public slots:
    //! set the layer used to display the fields and expression
    void setLayer( QgsVectorLayer* layer );

    //! convenience slot to connect QgsMapLayerComboBox layer signal
    void setLayer( QgsMapLayer* layer );

    //! sets the current field or expression in the widget
    void setField( QString fieldName );
};
