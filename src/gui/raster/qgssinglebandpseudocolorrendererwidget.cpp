/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.cpp
                         ------------------------------------------
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

#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrasterlayer.h"

QgsSingleBandPseudoColorRendererWidget::QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer ):
    QgsRasterRendererWidget( layer )
{
  setupUi( this );

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  //fill available bands into combo box
  int nBands = provider->bandCount();
  for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
  {
    mBandComboBox->addItem( provider->colorInterpretationName( i ), i );
  }

  mColorInterpolationComboBox->addItem( tr( "Discrete" ), 0 );
  mColorInterpolationComboBox->addItem( tr( "Linear" ), 1 );
  mColorInterpolationComboBox->addItem( tr( "Exact" ), 2 );

  mClassificationModeComboBox->addItem( tr( "Equal interval" ) );
  //quantile would be nice as well

  QgsSingleBandPseudoColorRenderer* r = dynamic_cast<QgsSingleBandPseudoColorRenderer*>( layer->renderer() );
  if ( r )
  {
    QgsRasterShader* rasterShader = r->shader();
    if ( rasterShader )
    {
      QgsColorRampShader* colorRampShader = dynamic_cast<QgsColorRampShader*>( rasterShader->rasterShaderFunction() );
      if ( colorRampShader )
      {
        if ( colorRampShader->colorRampType() == QgsColorRampShader::INTERPOLATED )
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Linear" ) ) );
        }
        else if ( colorRampShader->colorRampType() == QgsColorRampShader::DISCRETE )
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Discrete" ) ) );
        }
        else
        {
          mColorInterpolationComboBox->setCurrentIndex( mColorInterpolationComboBox->findText( tr( "Exact" ) ) );
        }

        const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader->colorRampItemList();
        QList<QgsColorRampShader::ColorRampItem>::const_iterator it = colorRampItemList.constBegin();
        for ( ; it != colorRampItemList.end(); ++it )
        {
          QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
          newItem->setText( 0, QString::number( it->value, 'f' ) );
          newItem->setBackground( 1, QBrush( it->color ) );
          newItem->setText( 2, it->label );
        }
      }
    }
  }
}

QgsSingleBandPseudoColorRendererWidget::~QgsSingleBandPseudoColorRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandPseudoColorRendererWidget::renderer()
{
  QgsRasterShader* rasterShader = new QgsRasterShader();
  QgsColorRampShader* colorRampShader = new QgsColorRampShader();

  //iterate through mColormapTreeWidget and set colormap info of layer
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;
  int topLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* currentItem;
  for ( int i = 0; i < topLevelItemCount; ++i )
  {
    currentItem = mColormapTreeWidget->topLevelItem( i );
    if ( !currentItem )
    {
      continue;
    }
    QgsColorRampShader::ColorRampItem newColorRampItem;
    newColorRampItem.value = currentItem->text( 0 ).toDouble();
    newColorRampItem.color = currentItem->background( 1 ).color();
    newColorRampItem.label = currentItem->text( 2 );
    colorRampItems.append( newColorRampItem );
  }
  // sort the shader items
  qSort( colorRampItems );
  colorRampShader->setColorRampItemList( colorRampItems );

  if ( mColorInterpolationComboBox->currentText() == tr( "Linear" ) )
  {
    colorRampShader->setColorRampType( QgsColorRampShader::INTERPOLATED );
  }
  else if ( mColorInterpolationComboBox->currentText() == tr( "Discrete" ) )
  {
    colorRampShader->setColorRampType( QgsColorRampShader::DISCRETE );
  }
  else
  {
    colorRampShader->setColorRampType( QgsColorRampShader::EXACT );
  }
  rasterShader->setRasterShaderFunction( colorRampShader );

  int bandNumber = mBandComboBox->itemData( mBandComboBox->currentIndex() ).toInt();
  return new QgsSingleBandPseudoColorRenderer( mRasterLayer->dataProvider(), bandNumber, rasterShader );
}

void QgsSingleBandPseudoColorRendererWidget::on_mClassifyButton_clicked()
{
  int bandComboIndex = mBandComboBox->currentIndex();
  if ( bandComboIndex == -1 || !mRasterLayer )
  {
    return;
  }

  int bandNr = mBandComboBox->itemData( bandComboIndex ).toInt();
  QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( bandNr );
  int numberOfEntries = mNumberOfEntriesSpinBox->value();

  QList<double> entryValues;
  QList<QColor> entryColors;

  if ( mClassificationModeComboBox->currentText() == tr( "Equal interval" ) )
  {
    double currentValue = myRasterBandStats.minimumValue;
    double intervalDiff;
    if ( numberOfEntries > 1 )
    {
      //because the highest value is also an entry, there are (numberOfEntries - 1)
      //intervals
      intervalDiff = ( myRasterBandStats.maximumValue - myRasterBandStats.minimumValue ) /
                     ( numberOfEntries - 1 );
    }
    else
    {
      intervalDiff = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
    }

    for ( int i = 0; i < numberOfEntries; ++i )
    {
      entryValues.push_back( currentValue );
      currentValue += intervalDiff;
    }
  }

  //hard code color range from blue -> red for now. Allow choice of ramps in future
  int colorDiff = 0;
  if ( numberOfEntries != 0 )
  {
    colorDiff = ( int )( 255 / numberOfEntries );
  }

  for ( int i = 0; i < numberOfEntries; ++i )
  {
    QColor currentColor;
    currentColor.setRgb( colorDiff*i, 0, 255 - colorDiff * i );
    entryColors.push_back( currentColor );
  }

  mColormapTreeWidget->clear();

  QList<double>::const_iterator value_it = entryValues.begin();
  QList<QColor>::const_iterator color_it = entryColors.begin();

  for ( ; value_it != entryValues.end(); ++value_it, ++color_it )
  {
    QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( 0, QString::number( *value_it, 'f' ) );
    newItem->setBackground( 1, QBrush( *color_it ) );
    newItem->setText( 2, QString::number( *value_it, 'f' ) );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}
