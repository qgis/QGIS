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
#include "qgsmaprenderer.h"
#include <QColorDialog>
#include <QFontDialog>

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap ): QWidget(), mComposerMap( composerMap )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerMap );
  toolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  mWidthLineEdit->setValidator( new QDoubleValidator( mWidthLineEdit ) );
  mHeightLineEdit->setValidator( new QDoubleValidator( mHeightLineEdit ) );
  mScaleLineEdit->setValidator( new QDoubleValidator( mScaleLineEdit ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( mXMaxLineEdit ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( mYMinLineEdit ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( mYMaxLineEdit ) );

  blockAllSignals( true );
  mPreviewModeComboBox->insertItem( 0, tr( "Cache" ) );
  mPreviewModeComboBox->insertItem( 1, tr( "Render" ) );
  mPreviewModeComboBox->insertItem( 2, tr( "Rectangle" ) );

  mGridTypeComboBox->insertItem( 0, tr( "Solid" ) );
  mGridTypeComboBox->insertItem( 1, tr( "Cross" ) );

  insertAnnotationPositionEntries( mAnnotationPositionLeftComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionRightComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionTopComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionBottomComboBox );

  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxLeft );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxRight );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxTop );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxBottom );

  mFrameStyleComboBox->insertItem( 0, tr( "No frame" ) );
  mFrameStyleComboBox->insertItem( 1, tr( "Zebra" ) );

  if ( composerMap )
  {
    connect( composerMap, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  }

  if ( mComposerMap )
  {
    //insert available maps into mMapComboBox
    const QgsComposition* composition = mComposerMap->composition();
    if ( composition )
    {
      QList<const QgsComposerMap*> availableMaps = composition->composerMapItems();
      QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
      for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
      {
        if (( *mapItemIt )->id() != mComposerMap->id() )
        {
          mOverviewFrameMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), ( *mapItemIt )->id() );
        }
      }
    }
  }


  updateGuiElements();
  blockAllSignals( false );
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

    mComposerMap->beginCommand( tr( "Change item width" ) );
    mComposerMap->setSceneRect( newRect );
    mComposerMap->endCommand();
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
    mComposerMap->beginCommand( tr( "Change item height" ) );
    mComposerMap->setSceneRect( newRect );
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mPreviewModeComboBox_activated( int i )
{
  Q_UNUSED( i );

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

  mComposerMap->beginCommand( tr( "Map scale changed" ) );
  mComposerMap->setNewScale( scaleDenominator );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mRotationSpinBox_valueChanged( double value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Map rotation changed" ), QgsComposerMergeCommand::ComposerMapRotation );
  mComposerMap->setMapRotation( value );
  mComposerMap->endCommand();
  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mSetToMapCanvasExtentButton_clicked()
{
  if ( mComposerMap )
  {
    const QgsMapRenderer* renderer = mComposerMap->mapRenderer();
    if ( renderer )
    {
      QgsRectangle newExtent = renderer->extent();

      //Make sure the width/height ratio is the same as in current composer map extent.
      //This is to keep the map item frame and the page layout fixed
      QgsRectangle currentMapExtent = mComposerMap->extent();
      double currentWidthHeightRatio = currentMapExtent.width() / currentMapExtent.height();
      double newWidthHeightRatio = newExtent.width() / newExtent.height();

      if ( currentWidthHeightRatio < newWidthHeightRatio )
      {
        //enlarge height of new extent
        double newHeight = newExtent.width() / currentWidthHeightRatio;
        newExtent.setYMinimum( newExtent.yMaximum() - newHeight );
      }
      else if ( currentWidthHeightRatio > newWidthHeightRatio )
      {
        //enlarge width of new extent
        double newWidth = currentWidthHeightRatio * newExtent.height();
        newExtent.setXMaximum( newExtent.xMinimum() + newWidth );
      }

      //fill text into line edits
      mXMinLineEdit->setText( QString::number( newExtent.xMinimum() ) );
      mXMaxLineEdit->setText( QString::number( newExtent.xMaximum() ) );
      mYMinLineEdit->setText( QString::number( newExtent.yMinimum() ) );
      mYMaxLineEdit->setText( QString::number( newExtent.yMaximum() ) );

      mComposerMap->beginCommand( tr( "Map extent changed" ) );
      mComposerMap->setNewExtent( newExtent );
      mComposerMap->endCommand();
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

void QgsComposerMapWidget::setGuiElementValues()
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
    blockAllSignals( true );

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
      mUpdatePreviewButton->setEnabled( true );
    }
    else if ( previewMode == QgsComposerMap::Render )
    {
      index = mPreviewModeComboBox->findText( tr( "Render" ) );
      mUpdatePreviewButton->setEnabled( true );
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
    QgsRectangle composerMapExtent = mComposerMap->extent();
    mXMinLineEdit->setText( QString::number( composerMapExtent.xMinimum(), 'f', 3 ) );
    mXMaxLineEdit->setText( QString::number( composerMapExtent.xMaximum(), 'f', 3 ) );
    mYMinLineEdit->setText( QString::number( composerMapExtent.yMinimum(), 'f', 3 ) );
    mYMaxLineEdit->setText( QString::number( composerMapExtent.yMaximum(), 'f', 3 ) );

    mRotationSpinBox->setValue( mComposerMap->rotation() );

    //keep layer list check box
    if ( mComposerMap->keepLayerSet() )
    {
      mKeepLayerListCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
    }

    //draw canvas items
    if ( mComposerMap->drawCanvasItems() )
    {
      mDrawCanvasItemsCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mDrawCanvasItemsCheckBox->setCheckState( Qt::Unchecked );
    }

    //grid
    if ( mComposerMap->gridEnabled() )
    {
      mGridCheckBox->setChecked( true );
    }
    else
    {
      mGridCheckBox->setChecked( false );
    }

    mIntervalXSpinBox->setValue( mComposerMap->gridIntervalX() );
    mIntervalYSpinBox->setValue( mComposerMap->gridIntervalY() );
    mOffsetXSpinBox->setValue( mComposerMap->gridOffsetX() );
    mOffsetYSpinBox->setValue( mComposerMap->gridOffsetY() );

    QgsComposerMap::GridStyle gridStyle = mComposerMap->gridStyle();
    if ( gridStyle == QgsComposerMap::Cross )
    {
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Cross" ) ) );
    }
    else
    {
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Solid" ) ) );
    }

    mCrossWidthSpinBox->setValue( mComposerMap->crossLength() );

    //grid frame
    mFrameWidthSpinBox->setValue( mComposerMap->gridFrameWidth() );
    QgsComposerMap::GridFrameStyle gridFrameStyle = mComposerMap->gridFrameStyle();
    if ( gridFrameStyle == QgsComposerMap::Zebra )
    {
      mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findText( tr( "Zebra" ) ) );
    }
    else //NoGridFrame
    {
      mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findText( tr( "No frame" ) ) );
    }

    //grid annotation position
    initAnnotationPositionBox( mAnnotationPositionLeftComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Left ) );
    initAnnotationPositionBox( mAnnotationPositionRightComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Right ) );
    initAnnotationPositionBox( mAnnotationPositionTopComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Top ) );
    initAnnotationPositionBox( mAnnotationPositionBottomComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Bottom ) );

    //grid annotation direction
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mComposerMap->gridAnnotationDirection( QgsComposerMap::Left ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mComposerMap->gridAnnotationDirection( QgsComposerMap::Right ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mComposerMap->gridAnnotationDirection( QgsComposerMap::Top ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mComposerMap->gridAnnotationDirection( QgsComposerMap::Bottom ) );

    mDistanceToMapFrameSpinBox->setValue( mComposerMap->annotationFrameDistance() );

    if ( mComposerMap->showGridAnnotation() )
    {
      mDrawAnnotationCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mDrawAnnotationCheckBox->setCheckState( Qt::Unchecked );
    }

    mCoordinatePrecisionSpinBox->setValue( mComposerMap->gridAnnotationPrecision() );

    QPen gridPen = mComposerMap->gridPen();
    mLineWidthSpinBox->setValue( gridPen.widthF() );
    mLineColorButton->setColor( gridPen.color() );

    blockAllSignals( false );
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
  if ( !conversionSuccess )
    return;
  xmax = mXMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymin = mYMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymax = mYMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;

  QgsRectangle newExtent( xmin, ymin, xmax, ymax );
  mComposerMap->beginCommand( tr( "Map extent changed" ) );
  mComposerMap->setNewExtent( newExtent );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::blockAllSignals( bool b )
{
  mWidthLineEdit->blockSignals( b );
  mHeightLineEdit->blockSignals( b );
  mScaleLineEdit->blockSignals( b );
  mXMinLineEdit->blockSignals( b );
  mXMaxLineEdit->blockSignals( b );
  mYMinLineEdit->blockSignals( b );
  mYMaxLineEdit->blockSignals( b );
  mIntervalXSpinBox->blockSignals( b );
  mIntervalYSpinBox->blockSignals( b );
  mOffsetXSpinBox->blockSignals( b );
  mOffsetYSpinBox->blockSignals( b );
  mGridTypeComboBox->blockSignals( b );
  mCrossWidthSpinBox->blockSignals( b );
  mPreviewModeComboBox->blockSignals( b );
  mKeepLayerListCheckBox->blockSignals( b );
  mSetToMapCanvasExtentButton->blockSignals( b );
  mUpdatePreviewButton->blockSignals( b );
  mLineWidthSpinBox->blockSignals( b );
  mLineColorButton->blockSignals( b );
  mDrawAnnotationCheckBox->blockSignals( b );
  mAnnotationFontButton->blockSignals( b );
  mAnnotationPositionLeftComboBox->blockSignals( b );
  mAnnotationPositionRightComboBox->blockSignals( b );
  mAnnotationPositionTopComboBox->blockSignals( b );
  mAnnotationPositionBottomComboBox->blockSignals( b );
  mDistanceToMapFrameSpinBox->blockSignals( b );
  mAnnotationDirectionComboBoxLeft->blockSignals( b );
  mAnnotationDirectionComboBoxRight->blockSignals( b );
  mAnnotationDirectionComboBoxTop->blockSignals( b );
  mAnnotationDirectionComboBoxBottom->blockSignals( b );
  mCoordinatePrecisionSpinBox->blockSignals( b );
  mDrawCanvasItemsCheckBox->blockSignals( b );
  mFrameStyleComboBox->blockSignals( b );
  mFrameWidthSpinBox->blockSignals( b );
  mOverviewFrameMapComboBox->blockSignals( b );
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

void QgsComposerMapWidget::on_mKeepLayerListCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->storeCurrentLayerSet();
    mComposerMap->setKeepLayerSet( true );
  }
  else
  {
    QStringList emptyLayerSet;
    mComposerMap->setLayerSet( emptyLayerSet );
    mComposerMap->setKeepLayerSet( false );
  }
}

void QgsComposerMapWidget::on_mDrawCanvasItemsCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Canvas items toggled" ) );
  mComposerMap->setDrawCanvasItems( state == Qt::Checked );
  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks
  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();
  mUpdatePreviewButton->setEnabled( true );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewFrameMapComboBox_activated( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  //get composition
  const QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  const QgsComposerMap* composerMap = composition->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }

  mComposerMap->setOverviewFrameMap( id );
  mComposerMap->update();

#if 0
  f( !mPicture || text.isEmpty() || !mPicture->useRotationMap() )
  {
    return;
  }

  //get composition
  const QgsComposition* composition = mPicture->composition();
  if ( !composition )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  const QgsComposerMap* composerMap = composition->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }
  mPicture->beginCommand( tr( "Rotation map changed" ) );
  mPicture->setRotationMap( id );
  mPicture->update();
  mPicture->endCommand();
#endif //0
}

void QgsComposerMapWidget::on_mGridCheckBox_toggled( bool state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid checkbox toggled" ) );
  if ( state )
  {
    mComposerMap->setGridEnabled( true );
  }
  else
  {
    mComposerMap->setGridEnabled( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mIntervalXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMap->setGridIntervalX( mIntervalXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mIntervalYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMap->setGridIntervalY( mIntervalYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMap->setGridOffsetX( mOffsetXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMap->setGridOffsetY( mOffsetYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid pen changed" ) );
  mComposerMap->setGridPenWidth( d );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mLineColorButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }
  QColor newColor = QColorDialog::getColor( mLineColorButton->color() );
  if ( newColor.isValid() )
  {
    mComposerMap->beginCommand( tr( "Grid pen changed" ) );
    mLineColorButton->setColor( newColor );
    mComposerMap->setGridPenColor( newColor );
    mComposerMap->endCommand();
  }
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mGridTypeComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( text == tr( "Cross" ) )
  {
    mComposerMap->setGridStyle( QgsComposerMap::Cross );
  }
  else
  {
    mComposerMap->setGridStyle( QgsComposerMap::Solid );
  }
  mComposerMap->beginCommand( tr( "Grid type changed" ) );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCrossWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid cross width changed" ) );
  mComposerMap->setCrossLength( d );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationFontButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont() );
#endif
  if ( ok )
  {
    mComposerMap->beginCommand( tr( "Annotation font changed" ) );
    mComposerMap->setGridAnnotationFont( newFont );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Annotation distance changed" ), QgsComposerMergeCommand::ComposerMapAnnotationDistance );
  mComposerMap->setAnnotationFrameDistance( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Bottom, text );
}

void QgsComposerMapWidget::on_mDrawAnnotationCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation toggled" ) );
  if ( state == Qt::Checked )
  {
    mComposerMap->setShowGridAnnotation( true );
  }
  else
  {
    mComposerMap->setShowGridAnnotation( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxRight_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxTop_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Bottom, text );
}

void QgsComposerMapWidget::on_mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Changed annotation precision" ) );
  mComposerMap->setGridAnnotationPrecision( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameStyleComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame style" ) );
  if ( text == tr( "Zebra" ) )
  {
    mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  }
  else //no frame
  {
    mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameWidthSpinBox_valueChanged( double d )
{
  if ( mComposerMap )
  {
    mComposerMap->beginCommand( tr( "Changed grid frame width" ) );
    mComposerMap->setGridFrameWidth( d );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::insertAnnotationPositionEntries( QComboBox* c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
  c->insertItem( 2, tr( "Disabled" ) );
}

void QgsComposerMapWidget::insertAnnotationDirectionEntries( QComboBox* c )
{
  c->insertItem( 0, tr( "Horizontal" ) );
  c->insertItem( 1, tr( "Vertical" ) );
}

void QgsComposerMapWidget::handleChangedAnnotationPosition( QgsComposerMap::Border border, const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, border );
  }
  else if ( text == tr( "Disabled" ) )
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, border );
  }
  else //Outside frame
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDirection( QgsComposerMap::Border border, const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  if ( text == tr( "Horizontal" ) )
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, border );
  }
  else //Vertical
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Vertical, border );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::initAnnotationPositionBox( QComboBox* c, QgsComposerMap::GridAnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsComposerMap::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside frame" ) ) );
  }
  else if ( pos == QgsComposerMap::OutsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Outside frame" ) ) );
  }
  else //disabled
  {
    c->setCurrentIndex( c->findText( tr( "Disabled" ) ) );
  }
}

void QgsComposerMapWidget::initAnnotationDirectionBox( QComboBox* c, QgsComposerMap::GridAnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }

  if ( dir == QgsComposerMap::Vertical )
  {
    c->setCurrentIndex( c->findText( tr( "Vertical" ) ) );
  }
  else //horizontal
  {
    c->setCurrentIndex( c->findText( tr( "Horizontal" ) ) );
  }
}
