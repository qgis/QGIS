/**
 * @brief Widget to control a layers transparency and related options
 */
class QgsRasterTransparencyWidget: QgsMapLayerConfigWidget
{
%TypeHeaderCode
#include <qgsrastertransparencywidget.h>
%End
  public:
    /**
     * @brief Widget to control a layers transparency and related options
     */
    QgsRasterTransparencyWidget( QgsRasterLayer* layer, QgsMapCanvas *canvas, QWidget *parent = 0 );
    ~QgsRasterTransparencyWidget();

  public slots:
    /**
     * Sync the widget state to the layer set for the widget.
     */
    void syncToLayer();

    /**
     * Apply any changes on the widget to the set layer.
     */
    void apply();
};
