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
