/** A generic dialog for building expression strings
  * @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
  */
class QgsExpressionBuilderDialog : QDialog
{
%TypeHeaderCode
#include <qgsexpressionbuilderdialog.h>
%End

  public:
    QgsExpressionBuilderDialog( QgsVectorLayer* layer, const QString& startText = QString(), QWidget* parent /TransferThis/ = NULL, const QString& key = "generic",
                                const QgsExpressionContext& context = QgsExpressionContext() );

    /** The builder widget that is used by the dialog */
    QgsExpressionBuilderWidget* expressionBuilder();

    void setExpressionText( const QString& text );

    QString expressionText();

    /** Returns the expression context for the dialog. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * @see setExpressionContext
     * @note added in QGIS 2.12
     */
    QgsExpressionContext expressionContext() const;

    /** Sets the expression context for the dialog. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * @param context expression context
     * @see expressionContext
     * @note added in QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext& context );

    /** Sets geometry calculator used in distance/area calculations. */
    void setGeomCalculator( const QgsDistanceArea & da );

  protected:
    /**
     * Is called when the dialog get accepted or rejected
     * Used to save geometry
     *
     * @param r result value (unused)
     */
    virtual void done( int r );

    virtual void accept();
};
