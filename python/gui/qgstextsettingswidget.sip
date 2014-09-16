class QgsTextSettingsWidget: QWidget
{
%TypeHeaderCode
  #include <qgstextsettingswidget.h>
%End
  public:
    QgsTextSettingsWidget( QWidget* parent /TransferThis/ = 0 );

    ~QgsTextSettingsWidget();

    void loadSettings( QgsTextRendererSettings *settings );

    void saveToSettings( QgsTextRendererSettings* settings );

    void setLayer( QgsVectorLayer* layer );

};

/** \ingroup gui
 * \class QgsTextSettingsDialog
 * A simple dialog for displaying and setting the properties of a QgsTextRendererSettings
 * \note Added in version 2.5
 */

class QgsTextSettingsDialog: QDialog
{
%TypeHeaderCode
  #include <qgstextsettingswidget.h>
%End
  public:

    /**Construct a new text settings dialog.
     * @param parent The parent QWidget for the dialog
     * @param fl window flags for dialog
     */
    QgsTextSettingsDialog( QWidget* parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    ~QgsTextSettingsDialog();

  public slots:

    /**Shows the dialog using the settings from a specified QgsTextRendererSettings. If the
     * user accepts the dialog the text renderer settings will be updated based on the values
     * from the GUI.
     * @param settings QgsTextRendererSettings for the dialog settings
     * @returns true if dialog is accepted
     */
    bool execForSettings( QgsTextRendererSettings* settings );

};
