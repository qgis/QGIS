
#include "qgsvectorrandomcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"

#include <QColorDialog>


QgsVectorRandomColorRampV2Dialog::QgsVectorRandomColorRampV2Dialog( QgsVectorRandomColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp )
{
  setupUi( this );

  spinCount->setValue( ramp->count() );
  spinHue1->setValue( ramp->hueMin() );
  spinHue2->setValue( ramp->hueMax() );
  spinSat1->setValue( ramp->satMin() );
  spinSat2->setValue( ramp->satMax() );
  spinVal1->setValue( ramp->valMin() );
  spinVal2->setValue( ramp->valMax() );

  connect( spinCount, SIGNAL( valueChanged( int ) ), this, SLOT( setCount( int ) ) );
  connect( spinHue1, SIGNAL( valueChanged( int ) ), this, SLOT( setHue1( int ) ) );
  connect( spinHue2, SIGNAL( valueChanged( int ) ), this, SLOT( setHue2( int ) ) );
  connect( spinSat1, SIGNAL( valueChanged( int ) ), this, SLOT( setSat1( int ) ) );
  connect( spinSat2, SIGNAL( valueChanged( int ) ), this, SLOT( setSat2( int ) ) );
  connect( spinVal1, SIGNAL( valueChanged( int ) ), this, SLOT( setVal1( int ) ) );
  connect( spinVal2, SIGNAL( valueChanged( int ) ), this, SLOT( setVal2( int ) ) );

  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::updatePreview()
{
  mRamp->updateColors();

  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );
}

void QgsVectorRandomColorRampV2Dialog::setCount( int val )
{
  mRamp->setCount( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setHue1( int val )
{
  mRamp->setHueMin( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setHue2( int val )
{
  mRamp->setHueMax( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setSat1( int val )
{
  mRamp->setSatMin( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setSat2( int val )
{
  mRamp->setSatMax( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setVal1( int val )
{
  mRamp->setValMin( val );
  updatePreview();
}

void QgsVectorRandomColorRampV2Dialog::setVal2( int val )
{
  mRamp->setValMax( val );
  updatePreview();
}
