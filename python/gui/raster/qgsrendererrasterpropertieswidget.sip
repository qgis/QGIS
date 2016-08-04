class QgsRendererRasterPropertiesWidget : QgsMapLayerConfigWidget
{
%TypeHeaderCode
#include <qgsrendererrasterpropertieswidget.h>
%End

  public:
    QgsRendererRasterPropertiesWidget( QgsMapLayer* layer, QgsMapCanvas* canvas, QWidget* parent );
    ~QgsRendererRasterPropertiesWidget();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * Return the active render widget. Can be null.
     */
    QgsRasterRendererWidget* currentRenderWidget();

  signals:

    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply();

    /**
     * @brief Sync the widget to the given layer.
     * @param layer The layer to use for the widget
     */
    void syncToLayer( QgsRasterLayer* layer );
};
