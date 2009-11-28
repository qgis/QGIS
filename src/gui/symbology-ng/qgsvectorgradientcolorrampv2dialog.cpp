
#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"

#include <QColorDialog>


QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp )
{

  setupUi( this );

  connect( btnColor1, SIGNAL( clicked() ), this, SLOT( setColor1() ) );
  connect( btnColor2, SIGNAL( clicked() ), this, SLOT( setColor2() ) );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::updatePreview()
{
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );

  btnColor1->setColor( mRamp->color1() );
  btnColor2->setColor( mRamp->color2() );
}

void QgsVectorGradientColorRampV2Dialog::setColor1()
{
  QColor color = QColorDialog::getColor( mRamp->color1(), this );
  if ( !color.isValid() )
    return;
  mRamp->setColor1( color );
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setColor2()
{
  QColor color = QColorDialog::getColor( mRamp->color2(), this );
  if ( !color.isValid() )
    return;
  mRamp->setColor2( color );
  updatePreview();
}
