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
    QgsExpressionSelectionDialog( QgsVectorLayer* layer, const QString& startText = QString(), QWidget* parent /TransferThis/ = 0 );

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

    /** Sets the message bar to display feedback from the dialog. This is used when zooming to
     * features to display the count of selected features.
     * @param messageBar target message bar
     * @note added in QGIS 3.0
     */
    void setMessageBar( QgsMessageBar* messageBar );

    /**
     * Sets a map canvas associated with the dialog.
     * @note added in QGIS 3.0
     */
    void setMapCanvas( QgsMapCanvas* canvas );

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
