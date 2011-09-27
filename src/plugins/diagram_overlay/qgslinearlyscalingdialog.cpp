/***************************************************************************
                         qgslinearlyscalingdialog.cpp  -  description
                         ----------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinearlyscalingdialog.h"
#include "qgsdiagramrenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsbardiagramfactory.h"
#include "qgspiediagramfactory.h"

QgsLinearlyScalingDialog::QgsLinearlyScalingDialog( QgsVectorLayer* vl ): QgsDiagramRendererWidget( vl )
{
  setupUi( this );
  QObject::connect( mFindMaximumValueButton, SIGNAL( clicked() ), this, SLOT( insertMaximumAttributeValue() ) );
  mSizeUnitComboBox->addItem( tr( "Millimeter" ) );
  mSizeUnitComboBox->addItem( tr( "Map units" ) );
}

QgsLinearlyScalingDialog::~QgsLinearlyScalingDialog()
{

}

QgsDiagramRenderer* QgsLinearlyScalingDialog::createRenderer( int classAttr, const QgsAttributeList& attributes ) const
{
  Q_UNUSED( attributes );
  //create a linearly scaling renderer
  QList<int> attributesList;
  attributesList.push_back( classAttr );
  QgsDiagramRenderer* renderer = new QgsDiagramRenderer( attributesList );

  //and items of renderer
  QList<QgsDiagramItem> itemList;
  QgsDiagramItem firstItem; firstItem.value = QVariant( 0.0 ); firstItem.size = 0;
  QgsDiagramItem secondItem;
  secondItem.value = QVariant( mValueLineEdit->text().toDouble() );
  secondItem.size = mSizeSpinBox->value();
  itemList.push_back( firstItem );
  itemList.push_back( secondItem );
  renderer->setDiagramItems( itemList );
  renderer->setItemInterpretation( QgsDiagramRenderer::LINEAR );

  return renderer;
}

void QgsLinearlyScalingDialog::applySettings( const QgsDiagramRenderer* renderer )
{
  const QgsDiagramRenderer* linearRenderer = dynamic_cast<const QgsDiagramRenderer *>( renderer );
  if ( linearRenderer )
  {
    QList<QgsDiagramItem> itemList = linearRenderer->diagramItems();
    QgsDiagramItem theItem = itemList.at( 1 ); //take the upper item
    mValueLineEdit->setText( theItem.value.toString() );
    mSizeSpinBox->setValue( theItem.size );

    if ( linearRenderer->factory() )
    {
      QgsDiagramFactory::SizeUnit sizeUnit = linearRenderer->factory()->sizeUnit();
      if ( sizeUnit == QgsDiagramFactory::MM )
      {
        mSizeUnitComboBox->setCurrentIndex( mSizeUnitComboBox->findText( tr( "Millimeter" ) ) );
      }
      else if ( sizeUnit == QgsDiagramFactory::MapUnits )
      {
        mSizeUnitComboBox->setCurrentIndex( mSizeUnitComboBox->findText( tr( "Map units" ) ) );
      }
    }
  }
}

void QgsLinearlyScalingDialog::insertMaximumAttributeValue()
{
  //find the maximum value for this attribute
  if ( mVectorLayer )
  {
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>( mVectorLayer->dataProvider() );
    if ( provider )
    {
      mValueLineEdit->setText( provider->maximumValue( mClassificationField ).toString() );
    }
  }
}

QgsDiagramFactory::SizeUnit QgsLinearlyScalingDialog::sizeUnit() const
{
  if ( mSizeUnitComboBox->currentText() == tr( "Map units" ) )
  {
    return QgsDiagramFactory::MapUnits;
  }
  else
  {
    return QgsDiagramFactory::MM;
  }
}
