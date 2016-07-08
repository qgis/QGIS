/** \ingroup gui
 * \class QgsMapLayerConfigWidgetFactory
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

    /** Constructor */
    QgsMapLayerConfigWidgetFactory(QString title, QIcon icon);

    /** Destructor */
    virtual ~QgsMapLayerConfigWidgetFactory();

    /**
     * @brief The icon that will be shown in the UI for the panel.
     * @return A QIcon for the panel icon.
     */
    virtual QIcon icon() const;

    void setIcon(const QIcon& icon);

    /**
     * @brief The title of the panel.
     * @note This may or may not be shown to the user.
     * @return Title of the panel
     */
    virtual QString title() const;

    void setTitle(const QString title);

    virtual bool supportsStyleDock() const;

    void setSupportsStyleDock(bool supports);

    virtual bool supportLayerPropertiesDialog() const;

    void setSupportLayerPropertiesDialog(bool supports);

    /**
     * @brief Check if the layer is supported for this widget.
     * @return True if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer ) const;

    /**
     * @brief Factory fucntion to create the widget on demand as needed by the dock.
     * @note This function is called each time the panel is selected. Keep it light for better UX.
     * @param layer The active layer in the dock.
     * @param canvas The map canvas.
     * @param dockWidget True of the widget will be shown a dock style widget.
     * @param parent The parent of the widget.
     * @return A new QgsMapStylePanel which is shown in the map style dock.
     */
    virtual QgsMapLayerConfigWidget* createWidget( QgsMapLayer* layer, QgsMapCanvas *canvas, bool dockWidget = true, QWidget* parent /TransferThis/ = 0) const = 0 /Factory/;
};
