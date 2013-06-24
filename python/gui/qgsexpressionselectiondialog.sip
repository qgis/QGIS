class QgsExpressionSelectionDialog : QDialog
{
%TypeHeaderCode
#include <qgsexpressionselectiondialog.h>
%End
  public:
    /**
     * Creates a new selection dialog.
     * @param layer     The layer on which the selection is to be performed.
     * @param startText A default expression text to be applied (Defaults to empty)
     * @param parent parent object (owner)
     */
    QgsExpressionSelectionDialog( QgsVectorLayer* layer, QString startText = QString(), QWidget* parent = 0 );

    /**
     * The builder widget that is used by the dialog
     * @return The builder widget that is used by the dialog
     */
    QgsExpressionBuilderWidget* expressionBuilder();

    /**
     * Sets the current expression text
     * @param text the expression text to set
     */
    void setExpressionText( const QString& text );

    /**
     * Returns the current expression text
     * @return The expression text
     */
    QString expressionText();

    /**
     *Sets geometry calculator used in distance/area calculations.
     */
    void setGeomCalculator( const QgsDistanceArea & da );

  public slots:
    void on_mActionSelect_triggered();
    void on_mActionAddToSelection_triggered();
    void on_mActionRemoveFromSelection_triggered();
    void on_mActionSelectInstersect_triggered();
    void on_mPbnClose_clicked();

  protected:
    /**
     * Implementation for closeEvent
     * Saves the window geometry
     * @param closeEvent Event object. Unused.
     */
    virtual void closeEvent( QCloseEvent *closeEvent );

    /**
     * Implementation for done (default behavior when pressing esc)
     * Calls close, so the window geometry gets saved and the object deleted.
     * @param r   Result value. Unused.
     */
    virtual void done( int r );
};
