#include "qgsphongmaterialwidget.h"

#include "phongmaterialsettings.h"


QgsPhongMaterialWidget::QgsPhongMaterialWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setMaterial( PhongMaterialSettings() );

  connect( btnDiffuse, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnAmbient, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( btnSpecular, &QgsColorButton::colorChanged, this, &QgsPhongMaterialWidget::changed );
  connect( spinShininess, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPhongMaterialWidget::changed );
}

void QgsPhongMaterialWidget::setMaterial( const PhongMaterialSettings &material )
{
  btnDiffuse->setColor( material.diffuse() );
  btnAmbient->setColor( material.ambient() );
  btnSpecular->setColor( material.specular() );
  spinShininess->setValue( material.shininess() );
}

PhongMaterialSettings QgsPhongMaterialWidget::material() const
{
  PhongMaterialSettings m;
  m.setDiffuse( btnDiffuse->color() );
  m.setAmbient( btnAmbient->color() );
  m.setSpecular( btnSpecular->color() );
  m.setShininess( spinShininess->value() );
  return m;
}
