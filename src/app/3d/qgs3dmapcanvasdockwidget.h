#ifndef QGS3DMAPCANVASDOCKWIDGET_H
#define QGS3DMAPCANVASDOCKWIDGET_H

#include "qgsdockwidget.h"

class Qgs3DMapCanvas;

class Map3D;


class Qgs3DMapCanvasDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvasDockWidget( QWidget *parent = nullptr );

    //! takes ownership
    void setMap( Map3D *map );

  private slots:
    void resetView();

  private:
    Qgs3DMapCanvas *mCanvas;
};

#endif // QGS3DMAPCANVASDOCKWIDGET_H
