/**
 * @brief The QgsMapLayerStyleManagerWidget class which is used to visually manage
 * the layer styles.
 */
class QgsMapLayerStyleManagerWidget : QgsMapStylePanel
{
%TypeHeaderCode
#include "qgsmaplayerstylemanagerwidget.h"
%End
  public:

  /**
     * @brief Style manager widget to mange the layers styles.
     * @param layer The layer for the widget
     * @param canvas The canvas object.
     * @param parent The parent.
     */
    QgsMapLayerStyleManagerWidget( QgsMapLayer* layer, QgsMapCanvas* canvas, QWidget *parent = 0 );

  public slots:
    void apply();
};
