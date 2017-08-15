#ifndef QGS3DMAPWIDGET_H
#define QGS3DMAPWIDGET_H

#include <QWidget>

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class Qgs3DMapSettings;
class Qgs3DMapScene;


class Qgs3DMapCanvas : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvas( QWidget *parent = nullptr );
    ~Qgs3DMapCanvas();

    //! Configure map scene being displayed. Takes ownership.
    void setMap( Qgs3DMapSettings *map );

    Qgs3DMapSettings *map() { return mMap; }

    //! Resets camera position to the default: looking down at the origin of world coordinates
    void resetView();

  protected:
    void resizeEvent( QResizeEvent *ev ) override;

  private:
    //! 3D window with all the 3D magic inside
    Qt3DExtras::Qt3DWindow *mWindow3D;
    //! Container QWidget that encapsulates mWindow3D so we can use it embedded in ordinary widgets app
    QWidget *mContainer;
    //! Description of the 3D scene
    Qgs3DMapSettings *mMap;
    //! Root entity of the 3D scene
    Qgs3DMapScene *mScene;
};

#endif // QGS3DMAPWIDGET_H
