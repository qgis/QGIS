/** \ingroup gui
 * \class QgsMapLayerConfigWidget
 * \class A panel widget that can be shown in the map style dock
 * \note added in QGIS 2.16
 */
class QgsMapLayerConfigWidget : public QgsPanelWidget
{
%TypeHeaderCode
#include <qgsmaplayerconfigwidget.h>
%End
public:
  /**
     * @brief A panel widget that can be shown in the map style dock
     * @param layer The layer active in the dock.
     * @param canvas The canvas object.
     * @param parent The parent of the widget.
     * @note The widget is created each time the panel is selected in the dock.
     * Keep the loading light as possible for speed in the UI.
     */
  QgsMapLayerConfigWidget(QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget *parent = 0);

public slots:

    /**
     * @brief Called when changes to the layer need to be made.
     * Will be called when live update is enabled.
     */
  virtual void apply() = 0;
};
