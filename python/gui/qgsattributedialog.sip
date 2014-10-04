class QgsAttributeDialog : QDialog
{
%TypeHeaderCode
#include <qgsattributedialog.h>
%End

  public:
    /**
     * Create an attribute dialog for a given layer and feature
     *
     * @param vl                The layer for which the dialog will be generated
     * @param thepFeature       A feature for which the dialog will be generated
     * @param featureOwner      Set to true, if the dialog should take ownership of the feature
     * @param myDa              A QgsDistanceArea which will be used for expressions
     * @param parent            A parent widget for the dialog
     * @param showDialogButtons True: Show the dialog buttons accept/cancel
     *
     * @deprecated
     */
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent = 0, bool showDialogButtons = true ) /Deprecated/;

    /**
     * Create an attribute dialog for a given layer and feature
     *
     * @param vl                The layer for which the dialog will be generated
     * @param thepFeature       A feature for which the dialog will be generated
     * @param featureOwner      Set to true, if the dialog should take ownership of the feature
     * @param parent            A parent widget for the dialog
     * @param showDialogButtons True: Show the dialog buttons accept/cancel
     * @param context           The context in which this dialog is created
     *
     */
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget* parent = 0, bool showDialogButtons = true, QgsAttributeEditorContext context = QgsAttributeEditorContext() );

    /** Saves the size and position for the next time
     *  this dialog box will be used.
     */
    void saveGeometry();

    /** Restores the size and position from the last time
     *  this dialog box was used.
     */
    void restoreGeometry();

    /**
     * @brief setHighlight
     * @param h The highlight. Ownership is taken.
     */
    void setHighlight( QgsHighlight *h );

    QDialog *dialog();

    const QgsFeature* feature();

    /**
     * Is this dialog editable?
     *
     * @return returns true, if this dialog was created in an editable manner.
     */
    bool editable();

    /**
     * Toggles the form mode between edit feature and add feature.
     * If set to true, the dialog will be editable even with an invalid feature.
     * If set to true, the dialog will add a new feature when the form is accepted.
     *
     * @param isAddDialog If set to true, turn this dialog into an add feature dialog.
     */
    void setIsAddDialog( bool isAddDialog );

    /**
     * Sets the edit command message (Undo) that will be used when the dialog is accepted
     *
     * @param message The message
     */
    void setEditCommandMessage( const QString& message );

  public slots:
    void accept();

    int exec();
    void show();

  protected:
    bool eventFilter( QObject *obj, QEvent *e );
};
