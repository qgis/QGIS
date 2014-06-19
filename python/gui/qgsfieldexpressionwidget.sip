
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

    //! return the title used for the expression dialog
    const QString expressionDialogTitle();

    //! setFilters allows fitering according to the type of field
    void setFilters( QgsFieldProxyModel::Filters filters );

    void setLeftHandButtonStyle( bool isLeft );

    //! currently used filter on list of fields
    QgsFieldProxyModel::Filters filters();

    //! set the geometry calculator used in the expression dialog
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * @brief currentField returns the currently selected field or expression if allowed
     * @param isExpression determines if the string returned is the name of a field or an expression
     * @param isValid determines if the expression (or field) returned is valid
     */
    QString currentField( bool *isExpression = 0, bool *isValid = 0 );

    /**
      * Return true if the current expression is valid
      */
    bool isValidExpression( QString *expressionError = 0 );

    bool isExpression();
    /**
      * Return the current text that is set in the expression area
      */
    QString currentText();

    //! Returns the currently used layer
    QgsVectorLayer* layer();

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( QString fieldName );

    //! fieldChanged signal with indication of the validity of the expression
    void fieldChanged( QString fieldName, bool isValid );

//    void returnPressed();

  public slots:
    //! set the layer used to display the fields and expression
    void setLayer( QgsVectorLayer* layer );

    //! convenience slot to connect QgsMapLayerComboBox layer signal
    void setLayer( QgsMapLayer* layer );

    //! sets the current field or expression in the widget
    void setField( const QString &fieldName );

  protected slots:
    //! open the expression dialog to edit the current or add a new expression
    void editExpression();

    //! when expression is edited by the user in the line edit, it will be checked for validity
    void expressionEdited( const QString expression );

    //! when expression has been edited (finished) it will be added to the model
    void expressionEditingFinished();

    void currentFieldChanged();

    /**
     * @brief updateLineEditStyle will re-style (color/font) the line edit depending on content and status
     * @param expression if expression is given it will be evaluated for the given string, otherwise it takes
     * current expression from the model
     */
    void updateLineEditStyle( const QString expression = QString() );

    bool isExpressionValid( const QString expressionStr );

  protected:
    void changeEvent( QEvent* event );
};
