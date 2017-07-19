#include "window3d.h"

#include "cameracontroller.h"
#include "map3d.h"
#include "scene.h"
#include "sidepanel.h"
#include "terrain.h"

#include <Qt3DLogic/QFrameAction>


Window3D::Window3D( SidePanel *p, Map3D &map )
  : panel( p )
  , map( map )
  , scene( nullptr )
{
  QRect viewportRect( QPoint( 0, 0 ), size() );
  scene = new Scene( map, defaultFrameGraph(), renderSettings(), camera(), viewportRect );

  mFrameAction = new Qt3DLogic::QFrameAction();
  connect( mFrameAction, &Qt3DLogic::QFrameAction::triggered,
           this, &Window3D::onFrameTriggered );
  scene->addComponent( mFrameAction ); // takes ownership

  setRootEntity( scene );

  timer.start( 1000 );
  connect( &timer, &QTimer::timeout, this, &Window3D::onTimeout );
}


void Window3D::resizeEvent( QResizeEvent *ev )
{
  Qt3DExtras::Qt3DWindow::resizeEvent( ev );
  QRect viewportRect( QPoint( 0, 0 ), size() );
  scene->cameraController()->setViewport( viewportRect );
}

void Window3D::onTimeout()
{
  panel->setFps( frames );
  frames = 0;
}

void Window3D::onFrameTriggered( float dt )
{
  Q_UNUSED( dt );
  //qDebug() << dt*1000;
  frames++;
}
