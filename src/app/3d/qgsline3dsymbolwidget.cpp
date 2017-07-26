#include "qgsline3dsymbolwidget.h"

#include "abstract3dsymbol.h"


QgsLine3DSymbolWidget::QgsLine3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setSymbol( Line3DSymbol() );

  connect( spinWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsLine3DSymbolWidget::changed );
}

void QgsLine3DSymbolWidget::setSymbol( const Line3DSymbol &symbol )
{
  spinWidth->setValue( symbol.width );
  spinHeight->setValue( symbol.height );
  spinExtrusion->setValue( symbol.extrusionHeight );
  cboAltClamping->setCurrentIndex( ( int ) symbol.altClamping );
  cboAltBinding->setCurrentIndex( ( int ) symbol.altBinding );
  widgetMaterial->setMaterial( symbol.material );
}

Line3DSymbol QgsLine3DSymbolWidget::symbol() const
{
  Line3DSymbol sym;
  sym.width = spinWidth->value();
  sym.height = spinHeight->value();
  sym.extrusionHeight = spinExtrusion->value();
  sym.altClamping = ( AltitudeClamping ) cboAltClamping->currentIndex();
  sym.altBinding = ( AltitudeBinding ) cboAltBinding->currentIndex();
  sym.material = widgetMaterial->material();
  return sym;
}
