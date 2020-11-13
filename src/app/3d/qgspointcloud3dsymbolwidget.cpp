#include "qgspointcloud3dsymbolwidget.h"

#include "qgspointcloudlayer.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudlayer3drenderer.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  this->setupUi( this );

  if ( layer )
  {
    QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() );
    QgsPointCloud3DSymbol *symbol;
    if ( renderer != nullptr )
    {
      symbol = const_cast<QgsPointCloud3DSymbol *>( renderer->symbol() );
      setSymbol( symbol );
    }
    else
    {
      symbol = new QgsPointCloud3DSymbol;
      setSymbol( symbol );
      delete symbol;
    }
  }

//  connect( mPointSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QgsPointCloud3DSymbolWidget::changed);
  connect( mPointSizeSpinBox, QOverload<double>::of( &QDoubleSpinBox::valueChanged ), [&]( double value )
  {
    qDebug() << "mPointSizeSpinBox->valueChanged()";
    emit changed();
  } );
}

void QgsPointCloud3DSymbolWidget::setLayer( QgsPointCloudLayer *layer, bool updateSymbol )
{
  mLayer = layer;
  if ( layer )
  {
    QgsPointCloudLayer3DRenderer *renderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() );
    QgsPointCloud3DSymbol *symbol;
    if ( renderer != nullptr )
    {
      symbol = const_cast<QgsPointCloud3DSymbol *>( renderer->symbol() );
      setSymbol( symbol );
    }
    else
    {
      symbol = new QgsPointCloud3DSymbol;
      setSymbol( symbol );
      delete symbol;
    }
  }
}

void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mPointSizeSpinBox->setValue( symbol->pointSize() );
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  // TODO: fix memory leak
  QgsPointCloud3DSymbol *symb = new QgsPointCloud3DSymbol;
  symb->setPointSize( mPointSizeSpinBox->value() );
  return symb;
}

