#ifndef QGS3DMAPCANVASDOCKWIDGET_H
#define QGS3DMAPCANVASDOCKWIDGET_H

#include "qgsdockwidget.h"

class Qgs3DMapCanvas;
class QgsMapCanvas;

class Qgs3DMapSettings;


class Qgs3DMapCanvasDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvasDockWidget( QWidget *parent = nullptr );

    //! takes ownership
    void setMap( Qgs3DMapSettings *map );

    void setMainCanvas( QgsMapCanvas *canvas );

  private slots:
    void resetView();
    void configure();

    void onMainCanvasLayersChanged();
    void onMainCanvasColorChanged();

  private:
    Qgs3DMapCanvas *mCanvas;
    QgsMapCanvas *mMainCanvas;
};

#endif // QGS3DMAPCANVASDOCKWIDGET_H
