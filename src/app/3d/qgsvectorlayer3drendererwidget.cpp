#include "qgsvectorlayer3drendererwidget.h"

#include "abstract3dsymbol.h"
#include "qgspolygon3dsymbolwidget.h"
#include "vectorlayer3drenderer.h"

#include "qgsvectorlayer.h"

#include <QBoxLayout>
#include <QCheckBox>

QgsVectorLayer3DRendererWidget::QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  chkEnabled = new QCheckBox( "Enable 3D renderer", this );
  widgetPolygon = new QgsPolygon3DSymbolWidget( this );
  layout->addWidget( chkEnabled );
  layout->addWidget( widgetPolygon );

  widgetPolygon->setEnabled( false );

  connect( chkEnabled, &QCheckBox::clicked, this, &QgsVectorLayer3DRendererWidget::onEnabledClicked );
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
  widgetPolygon->setEnabled( chkEnabled->isChecked() );

  if ( mRenderer && mRenderer->symbol() && mRenderer->symbol()->type() == "polygon" )
  {
    whileBlocking( widgetPolygon )->setSymbol( *static_cast<const Polygon3DSymbol *>( mRenderer->symbol() ) );
  }
}

VectorLayer3DRenderer *QgsVectorLayer3DRendererWidget::renderer()
{
  if ( chkEnabled->isChecked() )
  {
    VectorLayer3DRenderer *r = new VectorLayer3DRenderer( new Polygon3DSymbol( widgetPolygon->symbol() ) );
    r->setLayer( qobject_cast<QgsVectorLayer *>( mLayer ) );
    mRenderer.reset( r );
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
  widgetPolygon->setEnabled( chkEnabled->isChecked() );
  emit widgetChanged();
}
