/***************************************************************************
                         qgspalettedrendererwidget.cpp
                         -----------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspalettedrendererwidget.h"
#include "qgspalettedrasterrenderer.h"
#include "qgsrasterlayer.h"
#include "qgscolordialog.h"
#include <QColorDialog>

QgsPalettedRendererWidget::QgsPalettedRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent ): QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    //fill available bands into combo box
    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mBandComboBox->addItem( displayBandName( i ), i );
    }

    setFromRenderer( mRasterLayer->renderer() );
    connect( mBandComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  }
}

QgsPalettedRendererWidget::~QgsPalettedRendererWidget()
{

}

QgsRasterRenderer* QgsPalettedRendererWidget::renderer()
{
  int nColors = mTreeWidget->topLevelItemCount();
  QColor* colorArray = new QColor[nColors];
  QVector<QString> labels;
  for ( int i = 0; i < nColors; ++i )
  {
    colorArray[i] = mTreeWidget->topLevelItem( i )->background( 1 ).color();
    QString label = mTreeWidget->topLevelItem( i )->text( 2 );
    if ( !label.isEmpty() )
    {
      if ( i >= labels.size() ) labels.resize( i + 1 );
      labels[i] = label;
    }
  }
  int bandNumber = mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt();
  return new QgsPalettedRasterRenderer( mRasterLayer->dataProvider(), bandNumber, colorArray, nColors, labels );
}

void QgsPalettedRendererWidget::on_mTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  if ( column == 1 && item ) //change item color
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor c = QgsColorDialogV2::getColor( item->background( column ).color(), nullptr );
    if ( c.isValid() )
    {
      item->setBackground( column, QBrush( c ) );
      emit widgetChanged();
    }
  }
  else if ( column == 2 && item )
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsPalettedRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsPalettedRasterRenderer* pr = dynamic_cast<const QgsPalettedRasterRenderer*>( r );
  if ( pr )
  {
    //read values and colors and fill into tree widget
    int nColors = pr->nColors();
    QColor* colors = pr->colors();
    for ( int i = 0; i < nColors; ++i )
    {
      QTreeWidgetItem* item = new QTreeWidgetItem( mTreeWidget );
      item->setText( 0, QString::number( i ) );
      item->setBackground( 1, QBrush( colors[i] ) );
      item->setText( 2, pr->label( i ) );
    }
    delete[] colors;
  }
  else
  {
    //read default palette settings from layer
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( provider )
    {
      QList<QgsColorRampShader::ColorRampItem> itemList = provider->colorTable( mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt() );
      QList<QgsColorRampShader::ColorRampItem>::const_iterator itemIt = itemList.constBegin();
      int index = 0;
      for ( ; itemIt != itemList.constEnd(); ++itemIt )
      {
        QTreeWidgetItem* item = new QTreeWidgetItem( mTreeWidget );
        item->setText( 0, QString::number( index ) );
        item->setBackground( 1, QBrush( itemIt->color ) );
        item->setText( 2, itemIt->label );
        ++index;
      }
    }
  }
}
