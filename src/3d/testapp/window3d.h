#ifndef WINDOW3D_H
#define WINDOW3D_H

#include <Qt3DExtras/Qt3DWindow>

namespace Qt3DLogic
{
  class QFrameAction;
}

#include <QTimer>

class Qgs3DMapSettings;
class Qgs3DMapScene;
class SidePanel;

//! window with 3D content
class Window3D : public Qt3DExtras::Qt3DWindow
{
  public:
    Window3D( SidePanel *p, Qgs3DMapSettings &map );

  protected:
    void resizeEvent( QResizeEvent *ev ) override;

  private slots:
    void onTimeout();
    void onFrameTriggered( float dt );

  private:

    SidePanel *panel;
    Qgs3DMapSettings &map;
    Qgs3DMapScene *scene;

    QTimer timer;
    int frames = 0;
    Qt3DLogic::QFrameAction *mFrameAction;
};

#endif // WINDOW3D_H
