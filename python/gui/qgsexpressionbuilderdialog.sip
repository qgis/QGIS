/** A generic dialog for building expression strings
  * @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
  */
class QgsExpressionBuilderDialog : QDialog
{
%TypeHeaderCode
#include <qgsexpressionbuilderdialog.h>
%End

  public:
    QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText = QString(), QWidget* parent /TransferThis/ = NULL, QString key = "generic" );

    /** The builder widget that is used by the dialog */
    QgsExpressionBuilderWidget* expressionBuilder();

    void setExpressionText( const QString& text );

    QString expressionText();

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
