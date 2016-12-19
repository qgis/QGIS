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
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgscolordialog.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>

QgsPalettedRendererWidget::QgsPalettedRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent ): QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  contextMenu = new QMenu( tr( "Options" ), this );
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeColor() ) );
  contextMenu->addAction( tr( "Change transparency" ), this, SLOT( changeTransparency() ) );

  mTreeWidget->setColumnWidth( ColorColumn, 50 );
  mTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  mTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( mTreeWidget, &QTreeView::customContextMenuRequested,  [=]( const QPoint& ) { contextMenu->exec( QCursor::pos() ); }
         );

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
  int bandNumber = mBandComboBox->currentData().toInt();
  return new QgsPalettedRasterRenderer( mRasterLayer->dataProvider(), bandNumber, colorArray, nColors, labels );
}

void QgsPalettedRendererWidget::on_mTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  if ( column == ColorColumn && item ) //change item color
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    QColor c = QgsColorDialog::getColor( item->background( column ).color(), this, QStringLiteral( "Change color" ), true );
    if ( c.isValid() )
    {
      item->setBackground( column, QBrush( c ) );
      emit widgetChanged();
    }
  }
  else if ( column == LabelColumn && item )
  {
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsPalettedRendererWidget::on_mTreeWidget_itemChanged( QTreeWidgetItem * item, int column )
{
  if ( column == LabelColumn && item ) //palette label modified
  {
    emit widgetChanged();
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
      QList<QgsColorRampShader::ColorRampItem> itemList = provider->colorTable( mBandComboBox->currentData().toInt() );
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

void QgsPalettedRendererWidget::changeColor()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem* firstItem = itemList.first();

  QColor newColor = QgsColorDialog::getColor( firstItem->background( ColorColumn ).color(), this, QStringLiteral( "Change color" ), true );
  if ( newColor.isValid() )
  {
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }
    emit widgetChanged();
  }
}

void QgsPalettedRendererWidget::changeTransparency()
{
  QList<QTreeWidgetItem *> itemList;
  itemList = mTreeWidget->selectedItems();
  if ( itemList.isEmpty() )
  {
    return;
  }
  QTreeWidgetItem* firstItem = itemList.first();

  bool ok;
  double oldTransparency = ( firstItem->background( ColorColumn ).color().alpha() / 255.0 ) * 100.0;
  double transparency = QInputDialog::getDouble( this, tr( "Transparency" ), tr( "Change color transparency [%]" ), oldTransparency, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newTransparency = transparency / 100 * 255;
    Q_FOREACH ( QTreeWidgetItem *item, itemList )
    {
      QColor newColor = item->background( ColorColumn ).color();
      newColor.setAlpha( newTransparency );
      item->setBackground( ColorColumn, QBrush( newColor ) );
    }
    emit widgetChanged();
  }
}
