/**
 * @brief Widget to control a layers transparency and related options
 */
class QgsRasterTransparencyWidget: QWidget
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

  signals:
    /**
     * Emmited when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

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
