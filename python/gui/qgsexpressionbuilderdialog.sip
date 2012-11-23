/** A generic dialog for building expression strings
  * @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
  */
class QgsExpressionBuilderDialog : QDialog
{
%TypeHeaderCode
#include <qgsexpressionbuilderdialog.h>
%End

  public:
    QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText = QString(), QWidget* parent = NULL );

    /** The builder widget that is used by the dialog */
    QgsExpressionBuilderWidget* expressionBuilder();

    void setExpressionText( const QString& text );

    QString expressionText();

  protected:
    /**
     * Handle closing of the window
     * @param event unused
     */
    void closeEvent( QCloseEvent * event );
};
