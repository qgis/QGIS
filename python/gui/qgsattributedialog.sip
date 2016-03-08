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
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, const QgsDistanceArea& myDa, QWidget* parent /TransferThis/ = 0, bool showDialogButtons = true ) /Deprecated/;

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
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget* parent /TransferThis/ = 0, bool showDialogButtons = true, const QgsAttributeEditorContext& context = QgsAttributeEditorContext() );

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

    /**
     * @brief Returns reference to self. Only here for legacy compliance
     *
     * @return this
     *
     * @deprecated Do not use. Just use this object itself. Or QgsAttributeForm if you want to embed.
     */
    QDialog *dialog() /Deprecated/;

    QgsAttributeForm *attributeForm();

    const QgsFeature *feature();

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
     * @deprecated use setMode() instead
     */
    void setIsAddDialog( bool isAddDialog ) /Deprecated/;

    /**
     * Toggles the form mode.
     * @param mode form mode. Eg if set to QgsAttributeForm::AddFeatureMode, the dialog will be editable even with an invalid feature and
     * will add a new feature when the form is accepted.
     */
    void setMode( QgsAttributeForm::Mode mode );

    /**
     * Sets the edit command message (Undo) that will be used when the dialog is accepted
     *
     * @param message The message
     */
    void setEditCommandMessage( const QString& message );

    /**
     * Intercept window activate/deactive events to show/hide the highlighted feature.
     *
     * @param e The event
     *
     * @return The same as the parent QDialog
     */
    virtual bool event( QEvent *e );

  public slots:
    void accept();

    //! Show the dialog non-blocking. Reparents this dialog to be a child of the dialog form and is deleted when
    //! closed.
    void show();

    int exec();

  protected:
    bool eventFilter( QObject *obj, QEvent *e );
};
