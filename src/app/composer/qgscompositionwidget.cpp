/***************************************************************************
                              qgscompositionwidget.cpp
                             --------------------------
    begin                : June 11 2008
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

#include <qgis.h>
#include "qgscompositionwidget.h"
#include "qgscomposition.h"
#include <QColorDialog>
#include <QWidget>
#include <QPrinter> //for screen resolution

QgsCompositionWidget::QgsCompositionWidget( QWidget* parent, QgsComposition* c ): QWidget( parent ), mComposition( c )
{
  setupUi( this );
  blockSignals( true );
  createPaperEntries();

  //unit
  mPaperUnitsComboBox->addItem( tr( "mm" ) );
  mPaperUnitsComboBox->addItem( tr( "inch" ) );

  //orientation
  mPaperOrientationComboBox->insertItem( 0, tr( "Landscape" ) );
  mPaperOrientationComboBox->insertItem( 1, tr( "Portrait" ) );
  mPaperOrientationComboBox->setCurrentIndex( 0 );

  //read with/height from composition and find suitable entries to display
  displayCompositionWidthHeight();

  if ( mComposition )
  {
    //read printout resolution from composition
    mResolutionSpinBox->setValue( mComposition->printResolution() );

    //print as raster
    if ( mComposition->printAsRaster() )
    {
      mPrintAsRasterCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mPrintAsRasterCheckBox->setCheckState( Qt::Unchecked );
    }

    //snap grid
    if ( mComposition->snapToGridEnabled() )
    {
      mSnapToGridCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mSnapToGridCheckBox->setCheckState( Qt::Unchecked );
    }
    mGridResolutionSpinBox->setValue( mComposition->snapGridResolution() );
    mOffsetXSpinBox->setValue( mComposition->snapGridOffsetX() );
    mOffsetYSpinBox->setValue( mComposition->snapGridOffsetY() );


    //grid pen width
    mPenWidthSpinBox->setValue( mComposition->gridPen().widthF() );

    //grid pen color
    mGridColorButton->setColor( mComposition->gridPen().color() );

    mGridStyleComboBox->insertItem( 0, tr( "Solid" ) );
    mGridStyleComboBox->insertItem( 1, tr( "Dots" ) );
    mGridStyleComboBox->insertItem( 2, tr( "Crosses" ) );

    QgsComposition::GridStyle snapGridStyle = mComposition->gridStyle();
    if ( snapGridStyle == QgsComposition::Solid )
    {
      mGridStyleComboBox->setCurrentIndex( 0 );
    }
    else if ( snapGridStyle == QgsComposition::Dots )
    {
      mGridStyleComboBox->setCurrentIndex( 1 );
    }
    else
    {
      mGridStyleComboBox->setCurrentIndex( 2 );
    }
  }
  blockSignals( false );
}

QgsCompositionWidget::QgsCompositionWidget(): QWidget( 0 ), mComposition( 0 )
{
  setupUi( this );
}

QgsCompositionWidget::~QgsCompositionWidget()
{

}

void QgsCompositionWidget::createPaperEntries()
{
  QList<QgsCompositionPaper> formats;

  formats
  // ISO formats
  << QgsCompositionPaper( tr( "A5 (148x210 mm)" ), 148, 210 )
  << QgsCompositionPaper( tr( "A4 (210x297 mm)" ), 210, 297 )
  << QgsCompositionPaper( tr( "A3 (297x420 mm)" ), 297, 420 )
  << QgsCompositionPaper( tr( "A2 (420x594 mm)" ), 420, 594 )
  << QgsCompositionPaper( tr( "A1 (594x841 mm)" ), 594, 841 )
  << QgsCompositionPaper( tr( "A0 (841x1189 mm)" ), 841, 1189 )
  << QgsCompositionPaper( tr( "B5 (176 x 250 mm)" ), 176, 250 )
  << QgsCompositionPaper( tr( "B4 (250 x 353 mm)" ), 250, 353 )
  << QgsCompositionPaper( tr( "B3 (353 x 500 mm)" ), 353, 500 )
  << QgsCompositionPaper( tr( "B2 (500 x 707 mm)" ), 500, 707 )
  << QgsCompositionPaper( tr( "B1 (707 x 1000 mm)" ), 707, 1000 )
  << QgsCompositionPaper( tr( "B0 (1000 x 1414 mm)" ), 1000, 1414 )
  // North american formats
  << QgsCompositionPaper( tr( "Legal (8.5x14 inches)" ), 215.9, 355.6 )
  << QgsCompositionPaper( tr( "ANSI A (Letter; 8.5x11 inches)" ), 215.9, 279.4 )
  << QgsCompositionPaper( tr( "ANSI B (Tabloid; 11x17 inches)" ), 279.4, 431.8 )
  << QgsCompositionPaper( tr( "ANSI C (17x22 inches)" ), 431.8, 558.8 )
  << QgsCompositionPaper( tr( "ANSI D (22x34 inches)" ), 558.8, 863.6 )
  << QgsCompositionPaper( tr( "ANSI E (34x44 inches)" ), 863.6, 1117.6 )
  << QgsCompositionPaper( tr( "Arch A (9x12 inches)" ), 228.6, 304.8 )
  << QgsCompositionPaper( tr( "Arch B (12x18 inches)" ), 304.8, 457.2 )
  << QgsCompositionPaper( tr( "Arch C (18x24 inches)" ), 457.2, 609.6 )
  << QgsCompositionPaper( tr( "Arch D (24x36 inches)" ), 609.6, 914.4 )
  << QgsCompositionPaper( tr( "Arch E (36x48 inches)" ), 914.4, 1219.2 )
  << QgsCompositionPaper( tr( "Arch E1 (30x42 inches)" ), 762, 1066.8 )
  ;
  mPaperSizeComboBox->addItem( tr( "Custom" ) );

  for ( QList<QgsCompositionPaper>::const_iterator it = formats.begin(); it != formats.end(); it++ )
  {
    mPaperSizeComboBox->addItem( it->mName );
    mPaperMap.insert( it->mName, *it );
  }
  mPaperSizeComboBox->setCurrentIndex( 2 ); //A4
}

void QgsCompositionWidget::on_mPaperSizeComboBox_currentIndexChanged( const QString& text )
{
  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    mPaperWidthDoubleSpinBox->setEnabled( true );
    mPaperHeightDoubleSpinBox->setEnabled( true );
    mPaperUnitsComboBox->setEnabled( true );
  }
  else
  {
    mPaperWidthDoubleSpinBox->setEnabled( false );
    mPaperHeightDoubleSpinBox->setEnabled( false );
    mPaperUnitsComboBox->setEnabled( false );
  }
  applyCurrentPaperSettings();
}

void QgsCompositionWidget::on_mPaperOrientationComboBox_currentIndexChanged( const QString& text )
{
  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    adjustOrientation();
    applyWidthHeight();
  }
  else
  {
    adjustOrientation();
    applyCurrentPaperSettings();
  }
}

void QgsCompositionWidget::on_mPaperUnitsComboBox_currentIndexChanged( const QString& text )
{
  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm, value was inch
    width *= 25.4;
    height *= 25.4;
  }
  else
  {
    // inch, values was mm,
    width /= 25.4;
    height /= 25.4;
  }

  setSize( mPaperWidthDoubleSpinBox, width );
  setSize( mPaperHeightDoubleSpinBox, height );

  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    adjustOrientation();
    applyWidthHeight();
  }
  else
  {
    adjustOrientation();
    applyCurrentPaperSettings();
  }
}

void QgsCompositionWidget::adjustOrientation()
{
  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( width < 0 || height < 0 )
  {
    return;
  }

  if ( height > width ) //change values such that width > height
  {
    double tmp = width;
    width = height;
    height = tmp;
  }

  bool lineEditsEnabled = mPaperWidthDoubleSpinBox->isEnabled();

  mPaperWidthDoubleSpinBox->setEnabled( true );
  mPaperHeightDoubleSpinBox->setEnabled( true );
  if ( mPaperOrientationComboBox->currentText() == tr( "Landscape" ) )
  {
    setSize( mPaperWidthDoubleSpinBox, width );
    setSize( mPaperHeightDoubleSpinBox, height );
  }
  else
  {
    setSize( mPaperWidthDoubleSpinBox, height );
    setSize( mPaperHeightDoubleSpinBox, width );
  }
  mPaperWidthDoubleSpinBox->setEnabled( lineEditsEnabled );
  mPaperHeightDoubleSpinBox->setEnabled( lineEditsEnabled );
}

void QgsCompositionWidget::setSize( QDoubleSpinBox *spin, double v )
{
  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm
    spin->setValue( v );
  }
  else
  {
    // inch (show width in inch)
    spin->setValue( v / 25.4 );
  }
}

double QgsCompositionWidget::size( QDoubleSpinBox *spin )
{
  double size = spin->value();

  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm
    return size;
  }
  else
  {
    // inch return in mm
    return size * 25.4;
  }
}

void QgsCompositionWidget::applyCurrentPaperSettings()
{
  if ( mComposition )
  {
    //find entry in mPaper map to set width and height
    QMap<QString, QgsCompositionPaper>::iterator it = mPaperMap.find( mPaperSizeComboBox->currentText() );
    if ( it == mPaperMap.end() )
    {
      return;
    }

    mPaperWidthDoubleSpinBox->setEnabled( true );
    mPaperHeightDoubleSpinBox->setEnabled( true );
    setSize( mPaperWidthDoubleSpinBox, it->mWidth );
    setSize( mPaperHeightDoubleSpinBox, it->mHeight );
    mPaperWidthDoubleSpinBox->setEnabled( false );
    mPaperHeightDoubleSpinBox->setEnabled( false );

    adjustOrientation();
    applyWidthHeight();
  }
}

void QgsCompositionWidget::applyWidthHeight()
{
  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( width < 0 || height < 0 )
    return;

  mComposition->setPaperSize( width, height );
}

void QgsCompositionWidget::on_mPaperWidthDoubleSpinBox_editingFinished()
{
  applyWidthHeight();
}

void QgsCompositionWidget::on_mPaperHeightDoubleSpinBox_editingFinished()
{
  applyWidthHeight();
}

void QgsCompositionWidget::displayCompositionWidthHeight()
{
  if ( !mComposition )
  {
    return;
  }

  double paperWidth = mComposition->paperWidth();
  setSize( mPaperWidthDoubleSpinBox, paperWidth );

  double paperHeight = mComposition->paperHeight();
  setSize( mPaperHeightDoubleSpinBox, paperHeight );

  //set orientation
  if ( paperWidth > paperHeight )
  {
    mPaperOrientationComboBox->setCurrentIndex( mPaperOrientationComboBox->findText( tr( "Landscape" ) ) );
  }
  else
  {
    mPaperOrientationComboBox->setCurrentIndex( mPaperOrientationComboBox->findText( tr( "Portrait" ) ) );
  }

  //set paper name
  bool found = false;
  QMap<QString, QgsCompositionPaper>::const_iterator paper_it = mPaperMap.constBegin();
  for ( ; paper_it != mPaperMap.constEnd(); ++paper_it )
  {
    QgsCompositionPaper currentPaper = paper_it.value();

    //consider width and height values may be exchanged
    if (( doubleNear( currentPaper.mWidth, paperWidth ) && doubleNear( currentPaper.mHeight, paperHeight ) )
        || ( doubleNear( currentPaper.mWidth, paperHeight ) && doubleNear( currentPaper.mHeight, paperWidth ) ) )
    {
      mPaperSizeComboBox->setCurrentIndex( mPaperSizeComboBox->findText( paper_it.key() ) );
      found = true;
      break;
    }
  }

  if ( !found )
  {
    //custom
    mPaperSizeComboBox->setCurrentIndex( 0 );
  }
}

void QgsCompositionWidget::displaySnapingSettings()
{
  if ( !mComposition )
  {
    return;
  }

  if ( mComposition->snapToGridEnabled() )
  {
    mSnapToGridCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mSnapToGridCheckBox->setCheckState( Qt::Unchecked );
  }

  mGridResolutionSpinBox->setValue( mComposition->snapGridResolution() );
  mOffsetXSpinBox->setValue( mComposition->snapGridOffsetX() );
  mOffsetYSpinBox->setValue( mComposition->snapGridOffsetY() );
}

void QgsCompositionWidget::on_mResolutionSpinBox_valueChanged( const int value )
{
  mComposition->setPrintResolution( value );
}

void QgsCompositionWidget::on_mPrintAsRasterCheckBox_stateChanged( int state )
{
  if ( !mComposition )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposition->setPrintAsRaster( true );
  }
  else
  {
    mComposition->setPrintAsRaster( false );
  }
}

void QgsCompositionWidget::on_mSnapToGridCheckBox_stateChanged( int state )
{
  if ( mComposition )
  {
    if ( state == Qt::Checked )
    {
      mComposition->setSnapToGridEnabled( true );
    }
    else
    {
      mComposition->setSnapToGridEnabled( false );
    }
  }
}

void QgsCompositionWidget::on_mGridResolutionSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridResolution( d );
  }
}

void QgsCompositionWidget::on_mOffsetXSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridOffsetX( d );
  }
}

void QgsCompositionWidget::on_mOffsetYSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridOffsetY( d );
  }
}

void QgsCompositionWidget::on_mGridColorButton_clicked()
{
  QColor newColor = QColorDialog::getColor( mGridColorButton->color() );
  if ( !newColor.isValid() )
  {
    return ; //dialog canceled by user
  }
  mGridColorButton->setColor( newColor );

  if ( mComposition )
  {
    QPen pen = mComposition->gridPen();
    pen.setColor( newColor );
    mComposition->setGridPen( pen );
  }
}

void QgsCompositionWidget::on_mGridStyleComboBox_currentIndexChanged( const QString& text )
{
  if ( mComposition )
  {
    if ( mGridStyleComboBox->currentText() == tr( "Solid" ) )
    {
      mComposition->setGridStyle( QgsComposition::Solid );
    }
    else if ( mGridStyleComboBox->currentText() == tr( "Dots" ) )
    {
      mComposition->setGridStyle( QgsComposition::Dots );
    }
    else if ( mGridStyleComboBox->currentText() == tr( "Crosses" ) )
    {
      mComposition->setGridStyle( QgsComposition::Crosses );
    }
  }
}

void QgsCompositionWidget::on_mPenWidthSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    QPen pen = mComposition->gridPen();
    pen.setWidthF( d );
    mComposition->setGridPen( pen );
  }
}

void QgsCompositionWidget::blockSignals( bool block )
{
  mPaperSizeComboBox->blockSignals( block );
  mPaperUnitsComboBox->blockSignals( block );
  mPaperWidthDoubleSpinBox->blockSignals( block );
  mPaperHeightDoubleSpinBox->blockSignals( block );
  mPaperOrientationComboBox->blockSignals( block );
  mResolutionSpinBox->blockSignals( block );
  mPrintAsRasterCheckBox->blockSignals( block );
  mSnapToGridCheckBox->blockSignals( block );
  mGridResolutionSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mPenWidthSpinBox->blockSignals( block );
  mGridColorButton->blockSignals( block );
  mGridStyleComboBox->blockSignals( block );
}
