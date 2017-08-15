#include "qgsphongmaterialwidget.h"

#include "qgsphongmaterialsettings.h"


QgsPhongMaterialWidget::QgsPhongMaterialWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setMaterial( QgsPhongMaterialSettings() );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
}

void QgsPhongMaterialWidget::setMaterial( const QgsPhongMaterialSettings &material )
{
  btnDiffuse->setColor( material.diffuse() );
  btnAmbient->setColor( material.ambient() );
  btnSpecular->setColor( material.specular() );
  spinShininess->setValue( material.shininess() );
}

QgsPhongMaterialSettings QgsPhongMaterialWidget::material() const
{
  QgsPhongMaterialSettings m;
  m.setDiffuse( btnDiffuse->color() );
  m.setAmbient( btnAmbient->color() );
  m.setSpecular( btnSpecular->color() );
  m.setShininess( spinShininess->value() );
  return m;
}
