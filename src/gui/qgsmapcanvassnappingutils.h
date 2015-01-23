#ifndef QGSMAPCANVASSNAPPINGUTILS_H
#define QGSMAPCANVASSNAPPINGUTILS_H

#include "qgssnappingutils.h"

class QgsMapCanvas;

class QProgressDialog;

/** Snapping utils instance that is connected to a canvas and updates the configuration
 *  (map settings + current layer) whenever that is changed in the canvas.
 *  @note added in 2.8
 */
class GUI_EXPORT QgsMapCanvasSnappingUtils : public QgsSnappingUtils
{
    Q_OBJECT
  public:
    QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent = 0 );

  protected:
    virtual void prepareIndexStarting( int count ) override;
    virtual void prepareIndexProgress( int index ) override;

  private slots:
    void canvasMapSettingsChanged();
    void canvasCurrentLayerChanged();

  private:
    QgsMapCanvas* mCanvas;
    QProgressDialog* mProgress;
};


#endif // QGSMAPCANVASSNAPPINGUTILS_H
