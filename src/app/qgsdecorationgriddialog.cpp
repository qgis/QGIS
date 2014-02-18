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
#include "qgscontexthelp.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgisapp.h"

#include <QFontDialog>
#include <QColorDialog>
#include <QSettings>

QgsDecorationGridDialog::QgsDecorationGridDialog( QgsDecorationGrid& deco, QWidget* parent )
    : QDialog( parent ), mDeco( deco ), mLineSymbol( 0 ), mMarkerSymbol( 0 )
{
  setupUi( this );

  QSettings settings;
  //  restoreGeometry( settings.value( "/Windows/DecorationGrid/geometry" ).toByteArray() );

  chkEnable->setChecked( mDeco.enabled() );

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

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

}

void QgsDecorationGridDialog::updateGuiElements()
{
  // blockAllSignals( true );

  chkEnable->setChecked( mDeco.enabled() );

  mIntervalXEdit->setText( QString::number( mDeco.gridIntervalX() ) );
  mIntervalYEdit->setText( QString::number( mDeco.gridIntervalY() ) );
  mOffsetXEdit->setText( QString::number( mDeco.gridOffsetX() ) );
  mOffsetYEdit->setText( QString::number( mDeco.gridOffsetY() ) );

  mGridTypeComboBox->setCurrentIndex(( int ) mDeco.gridStyle() );
  mDrawAnnotationCheckBox->setChecked( mDeco.showGridAnnotation() );
  mAnnotationDirectionComboBox->setCurrentIndex(( int ) mDeco.gridAnnotationDirection() );
  mCoordinatePrecisionSpinBox->setValue( mDeco.gridAnnotationPrecision() );

  // QPen gridPen = mDeco.gridPen();
  // mLineWidthSpinBox->setValue( gridPen.widthF() );
  // mLineColorButton->setColor( gridPen.color() );

  if ( mLineSymbol )
    delete mLineSymbol;
  if ( mDeco.lineSymbol() )
  {
    mLineSymbol = dynamic_cast<QgsLineSymbolV2*>( mDeco.lineSymbol()->clone() );
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLineSymbol, mLineSymbolButton->iconSize() );
    mLineSymbolButton->setIcon( icon );
  }
  if ( mMarkerSymbol )
    delete mMarkerSymbol;
  if ( mDeco.markerSymbol() )
  {
    mMarkerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( mDeco.markerSymbol()->clone() );
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mMarkerSymbol, mMarkerSymbolButton->iconSize() );
    mMarkerSymbolButton->setIcon( icon );
  }

  updateInterval( false );

  // blockAllSignals( false );
}

void QgsDecorationGridDialog::updateDecoFromGui()
{
  mDeco.setDirty( false );
  mDeco.setEnabled( chkEnable->isChecked() );

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
  if ( mLineSymbol )
  {
    mDeco.setLineSymbol( mLineSymbol );
    mLineSymbol = dynamic_cast<QgsLineSymbolV2*>( mDeco.lineSymbol()->clone() );
  }
  if ( mMarkerSymbol )
  {
    mDeco.setMarkerSymbol( mMarkerSymbol );
    mMarkerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( mDeco.markerSymbol()->clone() );
  }
}

QgsDecorationGridDialog::~QgsDecorationGridDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/DecorationGrid/geometry", saveGeometry() );
  if ( mLineSymbol )
    delete mLineSymbol;
  if ( mMarkerSymbol )
    delete mMarkerSymbol;
}

void QgsDecorationGridDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void QgsDecorationGridDialog::on_buttonBox_accepted()
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

void QgsDecorationGridDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsDecorationGridDialog::on_mGridTypeComboBox_currentIndexChanged( int index )
{
  mLineSymbolButton->setEnabled( index == QgsDecorationGrid::Line );
  // mCrossWidthSpinBox->setEnabled( index == QgsDecorationGrid::Cross );
  mMarkerSymbolButton->setEnabled( index == QgsDecorationGrid::Marker );
}


void QgsDecorationGridDialog::on_mLineSymbolButton_clicked()
{
  if ( ! mLineSymbol )
    return;

  QgsLineSymbolV2* lineSymbol = dynamic_cast<QgsLineSymbolV2*>( mLineSymbol->clone() );
  QgsSymbolV2SelectorDialog dlg( lineSymbol, QgsStyleV2::defaultStyle(), 0, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete lineSymbol;
  }
  else
  {
    delete mLineSymbol;
    mLineSymbol = lineSymbol;
    if ( mLineSymbol )
    {
      QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLineSymbol, mLineSymbolButton->iconSize() );
      mLineSymbolButton->setIcon( icon );
    }
  }
}

void QgsDecorationGridDialog::on_mMarkerSymbolButton_clicked()
{
  if ( ! mMarkerSymbol )
    return;

  QgsMarkerSymbolV2* markerSymbol = dynamic_cast<QgsMarkerSymbolV2*>( mMarkerSymbol->clone() );
  QgsSymbolV2SelectorDialog dlg( markerSymbol, QgsStyleV2::defaultStyle(), 0, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    delete markerSymbol;
  }
  else
  {
    delete mMarkerSymbol;
    mMarkerSymbol = markerSymbol;
    if ( mMarkerSymbol )
    {
      QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mMarkerSymbol, mMarkerSymbolButton->iconSize() );
      mMarkerSymbolButton->setIcon( icon );
    }
  }
}

void QgsDecorationGridDialog::on_mPbtnUpdateFromExtents_clicked()
{
  updateInterval( true );
}

void QgsDecorationGridDialog::on_mPbtnUpdateFromLayer_clicked()
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

void QgsDecorationGridDialog::on_mAnnotationFontButton_clicked()
{
  bool ok;
#if defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mDeco.gridAnnotationFont(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mDeco.gridAnnotationFont() );
#endif
  if ( ok )
  {
    mDeco.setGridAnnotationFont( newFont );
  }
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
