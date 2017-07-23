#include "qgs3dmapcanvasdockwidget.h"

#include "qgs3dmapcanvas.h"

#include <QBoxLayout>
#include <QToolBar>

Qgs3DMapCanvasDockWidget::Qgs3DMapCanvasDockWidget( QWidget *parent )
  : QgsDockWidget( parent )
{
  QWidget *contentsWidget = new QWidget( this );

  QToolBar *toolBar = new QToolBar( contentsWidget );
  toolBar->addAction( "Reset view", this, &Qgs3DMapCanvasDockWidget::resetView );

  mCanvas = new Qgs3DMapCanvas( contentsWidget );
  mCanvas->setMinimumSize( QSize( 200, 200 ) );

  QVBoxLayout *layout = new QVBoxLayout( contentsWidget );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addWidget( toolBar );
  layout->addWidget( mCanvas, 1 );

  setWidget( contentsWidget );
}

void Qgs3DMapCanvasDockWidget::setMap( Map3D *map )
{
  mCanvas->setMap( map );
}

void Qgs3DMapCanvasDockWidget::resetView()
{
  mCanvas->resetView();
}
