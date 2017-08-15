#include "qgspolygon3dsymbolwidget.h"

#include "qgspolygon3dsymbol.h"


QgsPolygon3DSymbolWidget::QgsPolygon3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setSymbol( QgsPolygon3DSymbol() );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsPolygon3DSymbolWidget::changed );
}

void QgsPolygon3DSymbolWidget::setSymbol( const QgsPolygon3DSymbol &symbol )
{
  spinHeight->setValue( symbol.height() );
  spinExtrusion->setValue( symbol.extrusionHeight() );
  cboAltClamping->setCurrentIndex( ( int ) symbol.altitudeClamping() );
  cboAltBinding->setCurrentIndex( ( int ) symbol.altitudeBinding() );
  widgetMaterial->setMaterial( symbol.material() );
}

QgsPolygon3DSymbol QgsPolygon3DSymbolWidget::symbol() const
{
  QgsPolygon3DSymbol sym;
  sym.setHeight( spinHeight->value() );
  sym.setExtrusionHeight( spinExtrusion->value() );
  sym.setAltitudeClamping( ( AltitudeClamping ) cboAltClamping->currentIndex() );
  sym.setAltitudeBinding( ( AltitudeBinding ) cboAltBinding->currentIndex() );
  sym.setMaterial( widgetMaterial->material() );
  return sym;
}
