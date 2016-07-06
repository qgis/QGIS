/** \ingroup gui
 * \class QgsMapLayerPropertiesFactory
 * \note added in QGIS 2.16
 * Factory class for creating custom map layer property pages
 */
class QgsMapLayerConfigWidgetFactory
{
%TypeHeaderCode
#include <qgsmaplayerconfigwidgetfactory.h>
%End

  public:
    /** Constructor */
    QgsMapLayerConfigWidgetFactory();

    /** Destructor */
    virtual ~QgsMapLayerConfigWidgetFactory();

    /**
     * @brief The icon that will be shown in the UI for the panel.
     * @return A QIcon for the panel icon.
     */
    virtual QIcon icon() const;

    /**
     * @brief The title of the panel.
     * @note This may or may not be shown to the user.
     * @return Title of the panel
     */
    virtual QString title() const;

    /**
     * @brief Check if the layer is supported for this widget.
     * @return True if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer ) const;

    /**
     * @brief Create a new properties page
     * @param layer The layer for which to create the page
     * @param parent The parent widget
     * @return The new properties page instance
     */
    virtual QgsMapLayerConfigWidget* createPropertiesPage( QgsVectorLayer* layer, QWidget* parent = nullptr ) const;

    /**
     * @brief Factory fucntion to create the widget on demand as needed by the dock.
     * @note This function is called each time the panel is selected. Keep it light for better UX.
     * @param layer The active layer in the dock.
     * @param canvas The map canvas.
     * @param parent The parent of the widget.
     * @return A new QgsMapStylePanel which is shown in the map style dock.
     */
    virtual QgsMapLayerConfigWidget* createPanel( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget* parent /TransferThis/) const /Factory/;
};
