#ifndef QGSMAPCANVASSNAPPINGUTILS_H
#define QGSMAPCANVASSNAPPINGUTILS_H

#include "qgssnappingutils.h"

class QgsMapCanvas;

/** Snapping utils instance that is connected to a canvas and updates the configuration
 *  (map settings + current layer) whenever that is changed in the canvas.
 *  @note added in 2.8
 */
class GUI_EXPORT QgsMapCanvasSnappingUtils : public QgsSnappingUtils
{
  Q_OBJECT
public:
  QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent = 0 );

private slots:
  void canvasMapSettingsChanged();
  void canvasCurrentLayerChanged();

private:
  QgsMapCanvas* mCanvas;
};


#endif // QGSMAPCANVASSNAPPINGUTILS_H
