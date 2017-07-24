#include "qgs3dmapcanvas.h"

#include <QBoxLayout>
#include <Qt3DExtras/Qt3DWindow>

#include "cameracontroller.h"
#include "map3d.h"
#include "scene.h"


Qgs3DMapCanvas::Qgs3DMapCanvas( QWidget *parent )
  : QWidget( parent )
  , mWindow3D( nullptr )
  , mContainer( nullptr )
  , mMap( nullptr )
  , mScene( nullptr )
{
  mWindow3D = new Qt3DExtras::Qt3DWindow;
  mContainer = QWidget::createWindowContainer( mWindow3D );

  QHBoxLayout *hLayout = new QHBoxLayout( this );
  hLayout->setMargin( 0 );
  hLayout->addWidget( mContainer, 1 );
}

Qgs3DMapCanvas::~Qgs3DMapCanvas()
{
  delete mMap;
}

void Qgs3DMapCanvas::resizeEvent( QResizeEvent *ev )
{
  QWidget::resizeEvent( ev );

  QRect viewportRect( QPoint( 0, 0 ), size() );
  mScene->cameraController()->setViewport( viewportRect );
}

void Qgs3DMapCanvas::setMap( Map3D *map )
{
  // TODO: eventually we want to get rid of this
  Q_ASSERT( !mMap );
  Q_ASSERT( !mScene );

  QRect viewportRect( QPoint( 0, 0 ), size() );
  Scene *newScene = new Scene( *map, mWindow3D->defaultFrameGraph(), mWindow3D->renderSettings(), mWindow3D->camera(), viewportRect );

  mWindow3D->setRootEntity( newScene );

  if ( mScene )
    mScene->deleteLater();
  mScene = newScene;

  delete mMap;
  mMap = map;
}

void Qgs3DMapCanvas::resetView()
{
  mScene->cameraController()->resetView();
}
