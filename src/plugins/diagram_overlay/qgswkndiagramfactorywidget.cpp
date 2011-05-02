/***************************************************************************
                         qgswkndiagramfactorywidget.cpp  -  description
                         ------------------------------
    begin                : December 2008
    copyright            : (C) 2008 by Marco Hugentobler
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

#include "qgswkndiagramfactorywidget.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsbardiagramfactory.h"
#include "qgsdiagramoverlay.h"
#include "qgspiediagramfactory.h"
#include <QColorDialog>

QgsWKNDiagramFactoryWidget::QgsWKNDiagramFactoryWidget( QgsVectorLayer* vl, const QString& wellKnownName ): QgsDiagramFactoryWidget(), mVectorLayer( vl ), mDiagramTypeName( wellKnownName )
{
  setupUi( this );

  QStringList headerLabels;
  headerLabels << "Attribute";
  headerLabels << "Color";
  mAttributesTreeWidget->setHeaderLabels( headerLabels );
  QObject::connect( mAddPushButton, SIGNAL( clicked() ), this, SLOT( addAttribute() ) );
  QObject::connect( mRemovePushButton, SIGNAL( clicked() ), this, SLOT( removeAttribute() ) );
  QObject::connect( mAttributesTreeWidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( handleItemDoubleClick( QTreeWidgetItem*, int ) ) );

  //insert attributes into combo box
  QgsVectorDataProvider *provider;
  if (( provider = dynamic_cast<QgsVectorDataProvider *>( mVectorLayer->dataProvider() ) ) )
  {
    const QgsFieldMap & fields = provider->fields();
    QString str;

    int comboIndex = 0;
    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
    {
      str = ( *it ).name();
      mAttributesComboBox->insertItem( comboIndex, str );
      ++comboIndex;
    }
  }
}

QgsWKNDiagramFactoryWidget::QgsWKNDiagramFactoryWidget(): mVectorLayer( 0 )
{
}

QgsWKNDiagramFactoryWidget::~QgsWKNDiagramFactoryWidget()
{

}

QgsDiagramFactory* QgsWKNDiagramFactoryWidget::createFactory()
{
  QgsWKNDiagramFactory* f = 0;//new QgsWKNDiagramFactory();
  if ( mDiagramTypeName == "Bar" )
  {
    f = new QgsBarDiagramFactory();
  }
  else if ( mDiagramTypeName == "Pie" )
  {
    f = new QgsPieDiagramFactory();
  }
  else
  {
    return 0; //unknown diagram type
  }
  f->setDiagramType( mDiagramTypeName );

  int topLevelItemCount = mAttributesTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem = 0;
  int currentAttribute = -1;

  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    currentItem = mAttributesTreeWidget->topLevelItem( i );
    currentAttribute = QgsDiagramOverlay::indexFromAttributeName( currentItem->text( 0 ), mVectorLayer );
    if ( currentAttribute != -1 )
    {
      QgsDiagramCategory newCategory;
      newCategory.setPropertyIndex( currentAttribute );
      newCategory.setBrush( QBrush( currentItem->background( 1 ).color() ) );
      f->addCategory( newCategory );
    }
  }

  return f;
}

void QgsWKNDiagramFactoryWidget::setExistingFactory( const QgsDiagramFactory* f )
{
  const QgsWKNDiagramFactory* existingWKNFactory = dynamic_cast<const QgsWKNDiagramFactory *>( f );
  if ( existingWKNFactory )
  {
    //insert attribute names and colors into mAttributesTreeWidget
    mAttributesTreeWidget->clear();
    //insert attribute names and colors into mAttributesTreeWidget
    QList<QgsDiagramCategory> categoryList = existingWKNFactory->categories();
    QList<QgsDiagramCategory>::const_iterator c_it = categoryList.constBegin();

    for ( ; c_it != categoryList.constEnd(); ++c_it )
    {
      QTreeWidgetItem* newItem = new QTreeWidgetItem( mAttributesTreeWidget );
      newItem->setText( 0, QgsDiagramOverlay::attributeNameFromIndex( c_it->propertyIndex(), mVectorLayer ) );
      newItem->setBackground( 1, c_it->brush() );
      mAttributesTreeWidget->addTopLevelItem( newItem );
    }
  }
}

void QgsWKNDiagramFactoryWidget::addAttribute()
{
  QString currentText = mAttributesComboBox->currentText();
  if ( currentText.isEmpty() )
    return;

  QTreeWidgetItem *newItem = new QTreeWidgetItem( mAttributesTreeWidget );

  //text
  newItem->setText( 0, currentText );

  //and icon
  int red = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int green = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int blue = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  QColor randomColor( red, green, blue );
  newItem->setBackground( 1, QBrush( randomColor ) );

  mAttributesTreeWidget->addTopLevelItem( newItem );
}

void QgsWKNDiagramFactoryWidget::removeAttribute()
{
  QTreeWidgetItem* currentItem = mAttributesTreeWidget->currentItem();
  if ( currentItem )
  {
    delete currentItem;
  }
}

void QgsWKNDiagramFactoryWidget::handleItemDoubleClick( QTreeWidgetItem * item, int column )
{
  if ( column == 1 ) //change color
  {
    QColor newColor = QColorDialog::getColor();
    if ( newColor.isValid() )
    {
      item->setBackground( 1, QBrush( newColor ) );
    }
  }
}
