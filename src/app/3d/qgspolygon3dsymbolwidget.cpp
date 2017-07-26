#include "qgspolygon3dsymbolwidget.h"

#include "abstract3dsymbol.h"


QgsPolygon3DSymbolWidget::QgsPolygon3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setSymbol( Polygon3DSymbol() );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsPolygon3DSymbolWidget::changed );
}

void QgsPolygon3DSymbolWidget::setSymbol( const Polygon3DSymbol &symbol )
{
  spinHeight->setValue( symbol.height );
  spinExtrusion->setValue( symbol.extrusionHeight );
  cboAltClamping->setCurrentIndex( ( int ) symbol.altClamping );
  cboAltBinding->setCurrentIndex( ( int ) symbol.altBinding );
  widgetMaterial->setMaterial( symbol.material );
}

Polygon3DSymbol QgsPolygon3DSymbolWidget::symbol() const
{
  Polygon3DSymbol sym;
  sym.height = spinHeight->value();
  sym.extrusionHeight = spinExtrusion->value();
  sym.altClamping = ( AltitudeClamping ) cboAltClamping->currentIndex();
  sym.altBinding = ( AltitudeBinding ) cboAltBinding->currentIndex();
  sym.material = widgetMaterial->material();
  return sym;
}
