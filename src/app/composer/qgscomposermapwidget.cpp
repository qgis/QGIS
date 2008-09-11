/***************************************************************************
                         qgscomposermapwidget.cpp
                         ------------------------
    begin                : May 26, 2008
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

#include "qgscomposermapwidget.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermap.h"
#include "qgsmaprenderer.h"

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap ): QWidget(), mComposerMap( composerMap )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerMap );
  gridLayout->addWidget( itemPropertiesWidget, 9, 0, 1, 4 );

  mWidthLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mHeightLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mScaleLineEdit->setValidator( new QDoubleValidator( 0 ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( 0 ) );

  mPreviewModeComboBox->insertItem( 0, tr( "Cache" ) );
  //MH: disabled because this option leads to frequent crashes with Qt 4.4.0 and 4.4.1
  //mPreviewModeComboBox->insertItem(1, tr("Render"));
  mPreviewModeComboBox->insertItem( 2, tr( "Rectangle" ) );

  if ( composerMap )
  {
    connect( composerMap, SIGNAL( extentChanged() ), this, SLOT( updateSettingsNoSignals() ) );
  }

  updateGuiElements();
}

QgsComposerMapWidget::~QgsComposerMapWidget()
{

}

void QgsComposerMapWidget::on_mWidthLineEdit_editingFinished()
{
  if ( mComposerMap )
  {
    bool conversionSuccess = true;
    double newWidth = mWidthLineEdit->text().toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return;
    }
    QRectF composerMapRect = mComposerMap->rect();
    QTransform composerMapTransform = mComposerMap->transform();

    QRectF newRect( composerMapTransform.dx(), composerMapTransform.dy(), newWidth, composerMapRect.height() );
    mComposerMap->setSceneRect( newRect );
  }
}

void QgsComposerMapWidget::on_mHeightLineEdit_editingFinished()
{
  if ( mComposerMap )
  {
    bool conversionSuccess = true;
    double newHeight = mHeightLineEdit->text().toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return;
    }
    QRectF composerMapRect = mComposerMap->rect();
    QTransform composerMapTransform = mComposerMap->transform();

    QRectF newRect( composerMapTransform.dx(), composerMapTransform.dy(), composerMapRect.width(), newHeight );
    mComposerMap->setSceneRect( newRect );
  }
}

void QgsComposerMapWidget::on_mPreviewModeComboBox_activated( int i )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  QString comboText = mPreviewModeComboBox->currentText();
  if ( comboText == tr( "Cache" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Cache );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Render" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Render );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Rectangle" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Rectangle );
    mUpdatePreviewButton->setEnabled( false );
  }

  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mScaleLineEdit_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool conversionSuccess;
  double scaleDenominator = mScaleLineEdit->text().toDouble( &conversionSuccess );

  if ( !conversionSuccess )
  {
    return;
  }

  mComposerMap->setNewScale( scaleDenominator );
}

void QgsComposerMapWidget::on_mSetToMapCanvasExtentButton_clicked()
{
  if ( mComposerMap )
  {
    const QgsMapRenderer* renderer = mComposerMap->mapRenderer();
    if ( renderer )
    {
      QgsRect canvasExtent = renderer->extent();

      //fill text into line edits
      mXMinLineEdit->setText( QString::number( canvasExtent.xMin() ) );
      mXMaxLineEdit->setText( QString::number( canvasExtent.xMax() ) );
      mYMinLineEdit->setText( QString::number( canvasExtent.yMin() ) );
      mYMaxLineEdit->setText( QString::number( canvasExtent.yMax() ) );

      mComposerMap->setNewExtent( canvasExtent );
    }
  }
}

void QgsComposerMapWidget::on_mXMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mXMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::updateSettingsNoSignals()
{
  mHeightLineEdit->blockSignals( true );
  mWidthLineEdit->blockSignals( true );
  mScaleLineEdit->blockSignals( true );
  mPreviewModeComboBox->blockSignals( true );

  updateGuiElements();

  mHeightLineEdit->blockSignals( false );
  mWidthLineEdit->blockSignals( false );
  mScaleLineEdit->blockSignals( false );
  mPreviewModeComboBox->blockSignals( false );
}

void QgsComposerMapWidget::updateGuiElements()
{
  if ( mComposerMap )
  {
    //width, height, scale
    QRectF composerMapRect = mComposerMap->rect();
    mWidthLineEdit->setText( QString::number( composerMapRect.width() ) );
    mHeightLineEdit->setText( QString::number( composerMapRect.height() ) );
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 0 ) );

    //preview mode
    QgsComposerMap::PreviewMode previewMode = mComposerMap->previewMode();
    int index = -1;
    if ( previewMode == QgsComposerMap::Cache )
    {
      index = mPreviewModeComboBox->findText( tr( "Cache" ) );
    }
    else if ( previewMode == QgsComposerMap::Render )
    {
      index = mPreviewModeComboBox->findText( tr( "Render" ) );
    }
    else if ( previewMode == QgsComposerMap::Rectangle )
    {
      index = mPreviewModeComboBox->findText( tr( "Rectangle" ) );
      mUpdatePreviewButton->setEnabled( false );
    }
    if ( index != -1 )
    {
      mPreviewModeComboBox->setCurrentIndex( index );
    }

    //composer map extent
    QgsRect composerMapExtent = mComposerMap->extent();
    mXMinLineEdit->setText( QString::number( composerMapExtent.xMin(), 'f', 3));
    mXMaxLineEdit->setText( QString::number( composerMapExtent.xMax(), 'f', 3));
    mYMinLineEdit->setText( QString::number( composerMapExtent.yMin(), 'f', 3));
    mYMaxLineEdit->setText( QString::number( composerMapExtent.yMax(), 'f', 3));
  }
}

void QgsComposerMapWidget::updateComposerExtentFromGui()
{
  if ( !mComposerMap )
  {
    return;
  }

  double xmin, ymin, xmax, ymax;
  bool conversionSuccess;

  xmin = mXMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}
  xmax = mXMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}
  ymin = mYMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}
  ymax = mYMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}

  QgsRect newExtent( xmin, ymin, xmax, ymax );
  mComposerMap->setNewExtent( newExtent );
}

void QgsComposerMapWidget::on_mUpdatePreviewButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks

  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();

  mUpdatePreviewButton->setEnabled( true );
}
