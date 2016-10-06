/** \ingroup gui
 * @class QgsExpressionLineEdit
 * @brief The QgsExpressionLineEdit widget includes a line edit for entering expressions
 * together with a button to open the expression creation dialog.
 *
 * This widget is designed for use in contexts where no layer fields are available for
 * use in an expression. In contexts where the expression is directly associated with
 * a layer and fields can be used, then QgsFieldExpressionWidget is a more appropriate
 * choice as it gives users direct access to select fields from a drop down list.
 * @note added in QGIS 3.0
 */

class QgsExpressionLineEdit : QWidget
{
%TypeHeaderCode
#include "qgsexpressionlineedit.h"
%End

  public:

    /**
     * Constructor for QgsExpressionLineEdit.
     * @param parent parent widget
     */
    explicit QgsExpressionLineEdit( QWidget *parent /TransferThis/ = nullptr );
        /** Sets the title used in the expression builder dialog
     * @param title dialog title
     * @see expressionDialogTitle()
     */
    void setExpressionDialogTitle( const QString& title );

    /** Returns the title used for the expression dialog.
     * @see setExpressionDialogTitle()
     */
    QString expressionDialogTitle() const;

    /** Sets whether the widget should show a multiline text editor.
     * @param multiLine set to true to show multiline editor, or false
     * to show single line editor (the default).
     */
    void setMultiLine( bool multiLine );

    /** Set the geometry calculator used in the expression dialog.
     * @param distanceArea calculator
     */
    void setGeomCalculator( const QgsDistanceArea &distanceArea );

    /** Sets a layer associated with the widget. Required in order to get the fields and values
      * from the layer.
      * @param layer vector layer
      */
    void setLayer( QgsVectorLayer* layer );

    /** Returns the current expression shown in the widget.
     * @see setExpression()
     */
    QString expression() const;

    /**
      * Returns true if the current expression is valid.
      * @param expressionError will be set to any generated error message if specified
      */
    bool isValidExpression( QString *expressionError /Out/ = nullptr ) const;

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget.
     * @param generator A QgsExpressionContextGenerator class that will be used to
     *                  create an expression context when required.
     */
    void registerExpressionContextGenerator( const QgsExpressionContextGenerator* generator );

  signals:

    /** Emitted when the expression is changed.
     * @param expression new expression
     */
    void expressionChanged( const QString& expression );

  public slots:

    /** Sets the current expression to show in the widget.
     * @param expression expression string
     * @see expression()
     */
    void setExpression( const QString& expression );

  protected:
    void changeEvent( QEvent* event );
};
