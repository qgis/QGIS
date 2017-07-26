#include "qgsvectorlayer3drendererwidget.h"

#include "abstract3dsymbol.h"
#include "qgsline3dsymbolwidget.h"
#include "qgspolygon3dsymbolwidget.h"
#include "vectorlayer3drenderer.h"

#include "qgsvectorlayer.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QStackedWidget>

QgsVectorLayer3DRendererWidget::QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  chkEnabled = new QCheckBox( "Enable 3D renderer", this );
  widgetStack = new QStackedWidget( this );
  layout->addWidget( chkEnabled );
  layout->addWidget( widgetStack );

  widgetUnsupported = new QLabel( tr( "Sorry, this layer is not supported." ), this );
  widgetLine = new QgsLine3DSymbolWidget( this );
  widgetPolygon = new QgsPolygon3DSymbolWidget( this );

  widgetStack->addWidget( widgetUnsupported );
  widgetStack->addWidget( widgetLine );
  widgetStack->addWidget( widgetPolygon );

  connect( chkEnabled, &QCheckBox::clicked, this, &QgsVectorLayer3DRendererWidget::onEnabledClicked );
  connect( widgetLine, &QgsLine3DSymbolWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
  connect( widgetPolygon, &QgsPolygon3DSymbolWidget::changed, this, &QgsVectorLayer3DRendererWidget::widgetChanged );
}

QgsVectorLayer3DRendererWidget::~QgsVectorLayer3DRendererWidget()
{
}

void QgsVectorLayer3DRendererWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  QgsAbstract3DRenderer *r = layer->renderer3D();
  if ( r && r->type() == "vector" )
  {
    VectorLayer3DRenderer *vectorRenderer = static_cast<VectorLayer3DRenderer *>( r );
    setRenderer( vectorRenderer );
  }
  else
  {
    setRenderer( nullptr );
  }
}

void QgsVectorLayer3DRendererWidget::setRenderer( const VectorLayer3DRenderer *renderer )
{
  mRenderer.reset( renderer ? renderer->clone() : nullptr );

  whileBlocking( chkEnabled )->setChecked( ( bool )mRenderer );
  widgetLine->setEnabled( chkEnabled->isChecked() );
  widgetPolygon->setEnabled( chkEnabled->isChecked() );

  int pageIndex;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::LineGeometry:
      pageIndex = 1;
      if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == "line" )
      {
        whileBlocking( widgetLine )->setSymbol( *static_cast<const Line3DSymbol *>( mRenderer->symbol() ) );
      }
      else
      {
        whileBlocking( widgetLine )->setSymbol( Line3DSymbol() );
      }
      break;

    case QgsWkbTypes::PolygonGeometry:
      pageIndex = 2;
      if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == "polygon" )
      {
        whileBlocking( widgetPolygon )->setSymbol( *static_cast<const Polygon3DSymbol *>( mRenderer->symbol() ) );
      }
      else
      {
        whileBlocking( widgetPolygon )->setSymbol( Polygon3DSymbol() );
      }
      break;

    default:
      pageIndex = 0;   // unsupported
      break;
  }
  widgetStack->setCurrentIndex( pageIndex );
}

VectorLayer3DRenderer *QgsVectorLayer3DRendererWidget::renderer()
{
  if ( chkEnabled->isChecked() )
  {
    int pageIndex = widgetStack->currentIndex();
    if ( pageIndex == 1 || pageIndex == 2 )
    {
      Abstract3DSymbol *sym;
      if ( pageIndex == 1 )
        sym = new Line3DSymbol( widgetLine->symbol() );
      else
        sym = new Polygon3DSymbol( widgetPolygon->symbol() );
      VectorLayer3DRenderer *r = new VectorLayer3DRenderer( sym );
      r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
      mRenderer.reset( r );
    }
    else
    {
      mRenderer.reset();
    }
  }
  else
  {
    mRenderer.reset();
  }

  return mRenderer.get();
}

void QgsVectorLayer3DRendererWidget::apply()
{
  VectorLayer3DRenderer *r = renderer();
  mLayer->setRenderer3D( r ? r->clone() : nullptr );
}

void QgsVectorLayer3DRendererWidget::onEnabledClicked()
{
  widgetLine->setEnabled( chkEnabled->isChecked() );
  widgetPolygon->setEnabled( chkEnabled->isChecked() );
  emit widgetChanged();
}
