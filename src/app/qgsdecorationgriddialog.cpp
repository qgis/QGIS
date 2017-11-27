/***************************************************************************
                         qgsdecorationgriddialog.cpp
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationgriddialog.h"

#include "qgsdecorationgrid.h"

#include "qgslogger.h"
#include "qgshelp.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgisapp.h"
#include "qgsguiutils.h"
#include "qgssettings.h"

QgsDecorationGridDialog::QgsDecorationGridDialog( QgsDecorationGrid &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationGridDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationGridDialog::buttonBox_rejected );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDecorationGridDialog::mGridTypeComboBox_currentIndexChanged );
  connect( mPbtnUpdateFromExtents, &QPushButton::clicked, this, &QgsDecorationGridDialog::mPbtnUpdateFromExtents_clicked );
  connect( mPbtnUpdateFromLayer, &QPushButton::clicked, this, &QgsDecorationGridDialog::mPbtnUpdateFromLayer_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationGridDialog::showHelp );

  mMarkerSymbolButton->setSymbolType( QgsSymbol::Marker );
  mLineSymbolButton->setSymbolType( QgsSymbol::Line );

  mAnnotationFontButton->setMode( QgsFontButton::ModeQFont );

  QgsSettings settings;
  //  restoreGeometry( settings.value( "/Windows/DecorationGrid/geometry" ).toByteArray() );

  grpEnable->setChecked( mDeco.enabled() );

  // mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );

  mGridTypeComboBox->insertItem( QgsDecorationGrid::Line, tr( "Line" ) );
  // mGridTypeComboBox->insertItem( QgsDecorationGrid::Cross, tr( "Cross" ) );
  mGridTypeComboBox->insertItem( QgsDecorationGrid::Marker, tr( "Marker" ) );

  // mAnnotationPositionComboBox->insertItem( QgsDecorationGrid::InsideMapFrame, tr( "Inside frame" ) );
  // mAnnotationPositionComboBox->insertItem( QgsDecorationGrid::OutsideMapFrame, tr( "Outside frame" ) );

  mAnnotationDirectionComboBox->insertItem( QgsDecorationGrid::Horizontal,
      tr( "Horizontal" ) );
  mAnnotationDirectionComboBox->insertItem( QgsDecorationGrid::Vertical,
      tr( "Vertical" ) );
  mAnnotationDirectionComboBox->insertItem( QgsDecorationGrid::HorizontalAndVertical,
      tr( "Horizontal and Vertical" ) );
  mAnnotationDirectionComboBox->insertItem( QgsDecorationGrid::BoundaryDirection,
      tr( "Boundary direction" ) );

  updateGuiElements();

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsDecorationGridDialog::apply );
  connect( mAnnotationFontButton, &QgsFontButton::changed, this, &QgsDecorationGridDialog::annotationFontChanged );

  mMarkerSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mLineSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
}

void QgsDecorationGridDialog::updateGuiElements()
{
  // blockAllSignals( true );

  grpEnable->setChecked( mDeco.enabled() );

  mIntervalXEdit->setText( QString::number( mDeco.gridIntervalX() ) );
  mIntervalYEdit->setText( QString::number( mDeco.gridIntervalY() ) );
  mOffsetXEdit->setText( QString::number( mDeco.gridOffsetX() ) );
  mOffsetYEdit->setText( QString::number( mDeco.gridOffsetY() ) );

  mGridTypeComboBox->setCurrentIndex( static_cast< int >( mDeco.gridStyle() ) );
  mDrawAnnotationCheckBox->setChecked( mDeco.showGridAnnotation() );
  mAnnotationDirectionComboBox->setCurrentIndex( static_cast< int >( mDeco.gridAnnotationDirection() ) );
  mCoordinatePrecisionSpinBox->setValue( mDeco.gridAnnotationPrecision() );

  mDistanceToMapFrameSpinBox->setValue( mDeco.annotationFrameDistance() );
  // QPen gridPen = mDeco.gridPen();
  // mLineWidthSpinBox->setValue( gridPen.widthF() );
  // mLineColorButton->setColor( gridPen.color() );

  mLineSymbolButton->setSymbol( mDeco.lineSymbol()->clone() );
  mMarkerSymbolButton->setSymbol( mDeco.markerSymbol()->clone() );

  whileBlocking( mAnnotationFontButton )->setCurrentFont( mDeco.gridAnnotationFont() );

  updateInterval( false );

  // blockAllSignals( false );
}

void QgsDecorationGridDialog::updateDecoFromGui()
{
  mDeco.setDirty( false );
  mDeco.setEnabled( grpEnable->isChecked() );

  mDeco.setGridIntervalX( mIntervalXEdit->text().toDouble() );
  mDeco.setGridIntervalY( mIntervalYEdit->text().toDouble() );
  mDeco.setGridOffsetX( mOffsetXEdit->text().toDouble() );
  mDeco.setGridOffsetY( mOffsetYEdit->text().toDouble() );
  if ( mGridTypeComboBox->currentText() == tr( "Marker" ) )
  {
    mDeco.setGridStyle( QgsDecorationGrid::Marker );
  }
  else if ( mGridTypeComboBox->currentText() == tr( "Line" ) )
  {
    mDeco.setGridStyle( QgsDecorationGrid::Line );
  }
  mDeco.setAnnotationFrameDistance( mDistanceToMapFrameSpinBox->value() );
  // if ( mAnnotationPositionComboBox->currentText() == tr( "Inside frame" ) )
  // {
  //   mDeco.setGridAnnotationPosition( QgsDecorationGrid::InsideMapFrame );
  // }
  // else
  // {
  //   mDeco.setGridAnnotationPosition( QgsDecorationGrid::OutsideMapFrame );
  // }
  mDeco.setShowGridAnnotation( mDrawAnnotationCheckBox->isChecked() );
  QString text = mAnnotationDirectionComboBox->currentText();
  if ( text == tr( "Horizontal" ) )
  {
    mDeco.setGridAnnotationDirection( QgsDecorationGrid::Horizontal );
  }
  else if ( text == tr( "Vertical" ) )
  {
    mDeco.setGridAnnotationDirection( QgsDecorationGrid::Vertical );
  }
  else if ( text == tr( "Horizontal and Vertical" ) )
  {
    mDeco.setGridAnnotationDirection( QgsDecorationGrid::HorizontalAndVertical );
  }
  else //BoundaryDirection
  {
    mDeco.setGridAnnotationDirection( QgsDecorationGrid::BoundaryDirection );
  }
  mDeco.setGridAnnotationPrecision( mCoordinatePrecisionSpinBox->value() );
  mDeco.setLineSymbol( mLineSymbolButton->clonedSymbol< QgsLineSymbol >() );
  mDeco.setMarkerSymbol( mMarkerSymbolButton->clonedSymbol< QgsMarkerSymbol >() );
}

QgsDecorationGridDialog::~QgsDecorationGridDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Windows/DecorationGrid/geometry" ), saveGeometry() );
}

void QgsDecorationGridDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#grid" ) );
}

void QgsDecorationGridDialog::buttonBox_accepted()
{
  updateDecoFromGui();
  // mDeco.update();
  accept();
}

void QgsDecorationGridDialog::apply()
{
  updateDecoFromGui();
  mDeco.update();
  //accept();
}

void QgsDecorationGridDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationGridDialog::mGridTypeComboBox_currentIndexChanged( int index )
{
  mLineSymbolButton->setEnabled( index == QgsDecorationGrid::Line );
  // mCrossWidthSpinBox->setEnabled( index == QgsDecorationGrid::Cross );
  mMarkerSymbolButton->setEnabled( index == QgsDecorationGrid::Marker );
}

void QgsDecorationGridDialog::mPbtnUpdateFromExtents_clicked()
{
  updateInterval( true );
}

void QgsDecorationGridDialog::mPbtnUpdateFromLayer_clicked()
{
  double values[4];
  if ( mDeco.getIntervalFromCurrentLayer( values ) )
  {
    mIntervalXEdit->setText( QString::number( values[0] ) );
    mIntervalYEdit->setText( QString::number( values[1] ) );
    mOffsetXEdit->setText( QString::number( values[2] ) );
    mOffsetYEdit->setText( QString::number( values[3] ) );
    if ( values[0] >= 1 )
      mCoordinatePrecisionSpinBox->setValue( 0 );
    else
      mCoordinatePrecisionSpinBox->setValue( 3 );
  }
}

void QgsDecorationGridDialog::annotationFontChanged()
{
  mDeco.setGridAnnotationFont( mAnnotationFontButton->currentFont() );
}

void QgsDecorationGridDialog::updateInterval( bool force )
{
  if ( force || mDeco.isDirty() )
  {
    double values[4];
    if ( mDeco.getIntervalFromExtent( values, true ) )
    {
      mIntervalXEdit->setText( QString::number( values[0] ) );
      mIntervalYEdit->setText( QString::number( values[1] ) );
      mOffsetXEdit->setText( QString::number( values[2] ) );
      mOffsetYEdit->setText( QString::number( values[3] ) );
      // also update coord. precision
      // if interval >= 1, set precision=0 because we have a rounded value
      // else set it to previous default of 3
      if ( values[0] >= 1 )
        mCoordinatePrecisionSpinBox->setValue( 0 );
      else
        mCoordinatePrecisionSpinBox->setValue( 3 );
    }
  }
}
