class QgsRendererV2PropertiesDialog : QDialog
{
%TypeHeaderCode
#include <qgsrendererv2propertiesdialog.h>
%End

  public:

    /** Constructor for QgsRendererV2PropertiesDialog.
     * @param layer associated layer
     * @param style style collection
     * @param embedded set to true to indicate that the dialog will be embedded in another widget, rather
     * than shown as a dialog by itself
     * @param parent parent widget
     */
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false, QWidget* parent /TransferThis/ = nullptr );

    ~QgsRendererV2PropertiesDialog();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * @param dockMode True to enable dock mode.
     */
    void setDockMode( bool dockMode );

  signals:
    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

    /**
     * Emit when you require a panel to be show in the interface.
     * @param panel The panel widget to show.
     * @note If you are connected to this signal you should also connect
     * given panels showPanel signal as they can be nested.
     */
    void showPanel( QgsPanelWidget* panel );

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply();

    //! Apply and accept the changes for the dialog.
    void onOK();

    /**
     * Open a panel or dialog depending on dock mode setting
     * If dock mode is true this method will emit the showPanel signal
     * for connected slots to handle the open event.
     *
     * If dock mode is false this method will open a dialog
     * and block the user.
     *
     * @param panel The panel widget to open.
     */
    void openPanel( QgsPanelWidget* panel );

  protected:
    /**
     * Connect the given slot to the value changed event for the set of widgets
     * Each widget is checked for type and the common type of signal is connected
     * to the slot.
     *
     * @param widgets The list of widgets to check.
     * @param slot The slot to connect to the signals.
     */
    void connectValueChanged( QList<QWidget *> widgets, const char *slot );


    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

};
