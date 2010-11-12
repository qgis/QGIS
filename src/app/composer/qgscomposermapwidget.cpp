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

  mAnnotationPositionComboBox->insertItem( 0, tr( "Inside frame" ) );
  mAnnotationPositionComboBox->insertItem( 1, tr( "Outside frame" ) );

  mAnnotationDirectionComboBox->insertItem( 0, tr( "Horizontal" ) );
  mAnnotationDirectionComboBox->insertItem( 1, tr( "Vertical" ) );
  mAnnotationDirectionComboBox->insertItem( 2, tr( "Horizontal and Vertical" ) );
  mAnnotationDirectionComboBox->insertItem( 2, tr( "Boundary direction" ) );
  if ( composerMap )
  {
    connect( composerMap, SIGNAL( extentChanged() ), this, SLOT( updateSettingsNoSignals() ) );
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

void QgsComposerMapWidget::on_mRotationSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->setMapRotation( value );
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

      mComposerMap->setNewExtent( newExtent );
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

    QgsComposerMap::GridAnnotationPosition annotationPos = mComposerMap->gridAnnotationPosition();
    if ( annotationPos == QgsComposerMap::InsideMapFrame )
    {
      mAnnotationPositionComboBox->setCurrentIndex( mAnnotationPositionComboBox->findText( tr( "Inside frame" ) ) );
    }
    else
    {
      mAnnotationPositionComboBox->setCurrentIndex( mAnnotationPositionComboBox->findText( tr( "Outside frame" ) ) );
    }

    mDistanceToMapFrameSpinBox->setValue( mComposerMap->annotationFrameDistance() );

    if ( mComposerMap->showGridAnnotation() )
    {
      mDrawAnnotationCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mDrawAnnotationCheckBox->setCheckState( Qt::Unchecked );
    }

    QgsComposerMap::GridAnnotationDirection dir = mComposerMap->gridAnnotationDirection();
    if ( dir == QgsComposerMap::Horizontal )
    {
      mAnnotationDirectionComboBox->setCurrentIndex( mAnnotationDirectionComboBox->findText( tr( "Horizontal" ) ) );
    }
    else if ( dir == QgsComposerMap::Vertical )
    {
      mAnnotationDirectionComboBox->setCurrentIndex( mAnnotationDirectionComboBox->findText( tr( "Vertical" ) ) );
    }
    else if ( dir == QgsComposerMap::HorizontalAndVertical )
    {
      mAnnotationDirectionComboBox->setCurrentIndex( mAnnotationDirectionComboBox->findText( tr( "Horizontal and Vertical" ) ) );
    }
    else //BoundaryDirection
    {
      mAnnotationDirectionComboBox->setCurrentIndex( mAnnotationDirectionComboBox->findText( tr( "Boundary direction" ) ) );
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
  if ( !conversionSuccess ) {return;}
  xmax = mXMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}
  ymin = mYMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}
  ymax = mYMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess ) {return;}

  QgsRectangle newExtent( xmin, ymin, xmax, ymax );
  mComposerMap->setNewExtent( newExtent );
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
  mAnnotationPositionComboBox->blockSignals( b );
  mDistanceToMapFrameSpinBox->blockSignals( b );
  mAnnotationDirectionComboBox->blockSignals( b );
  mCoordinatePrecisionSpinBox->blockSignals( b );
  mDrawCanvasItemsCheckBox->blockSignals( b );
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

  mComposerMap->setDrawCanvasItems( state == Qt::Checked );
  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks
  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();
  mUpdatePreviewButton->setEnabled( true );
}

void QgsComposerMapWidget::on_mGridCheckBox_toggled( bool state )
{
  if ( !mComposerMap )
  {
    return;
  }

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
}

void QgsComposerMapWidget::on_mIntervalXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridIntervalX( mIntervalXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mIntervalYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridIntervalY( mIntervalYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOffsetXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridOffsetX( mOffsetXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOffsetYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridOffsetY( mOffsetYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridPenWidth( d );
  mComposerMap->update();
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
    mLineColorButton->setColor( newColor );
    mComposerMap->setGridPenColor( newColor );
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
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mCrossWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->setCrossLength( d );
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mAnnotationFontButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__)
  // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug)
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont(), this );
#endif
  if ( ok )
  {
    mComposerMap->setGridAnnotationFont( newFont );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
  }
}

void QgsComposerMapWidget::on_mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setAnnotationFrameDistance( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mAnnotationPositionComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( text == tr( "Inside frame" ) )
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame );
  }
  else
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mDrawAnnotationCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

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
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( text == tr( "Horizontal" ) )
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal );
  }
  else if ( text == tr( "Vertical" ) )
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Vertical );
  }
  else if ( text == tr( "Horizontal and Vertical" ) )
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::HorizontalAndVertical );
  }
  else //BoundaryDirection
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::BoundaryDirection );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->setGridAnnotationPrecision( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
}
