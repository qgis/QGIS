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
#include "qgssymbolv2propertiesdialog.h"

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

  mGridTypeComboBox->insertItem( QgsDecorationGrid::Solid, tr( "Lines" ) );
  mGridTypeComboBox->insertItem( QgsDecorationGrid::Cross, tr( "Cross" ) );
  mGridTypeComboBox->insertItem( QgsDecorationGrid::Marker, tr( "Marker" ) );

  mAnnotationPositionComboBox->insertItem( QgsDecorationGrid::InsideMapFrame, tr( "Inside frame" ) );
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

  mIntervalXSpinBox->setValue( mDeco.gridIntervalX() );
  mIntervalYSpinBox->setValue( mDeco.gridIntervalY() );
  mOffsetXSpinBox->setValue( mDeco.gridOffsetX() );
  mOffsetYSpinBox->setValue( mDeco.gridOffsetY() );

  mGridTypeComboBox->setCurrentIndex(( int ) mDeco.gridStyle() );

  mCrossWidthSpinBox->setValue( mDeco.crossLength() );

  mAnnotationPositionComboBox->setCurrentIndex(( int ) mDeco.gridAnnotationPosition() );

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

  // blockAllSignals( false );
}

void QgsDecorationGridDialog::updateDecoFromGui()
{
  mDeco.setEnabled( chkEnable->isChecked() );

  mDeco.setGridIntervalX( mIntervalXSpinBox->value() );
  mDeco.setGridIntervalY( mIntervalYSpinBox->value() );
  mDeco.setGridOffsetX( mOffsetXSpinBox->value() );
  mDeco.setGridOffsetY( mOffsetYSpinBox->value() );
  // mDeco.setGridPenWidth( mLineWidthSpinBox->value() );
  if ( mGridTypeComboBox->currentText() == tr( "Cross" ) )
  {
    mDeco.setGridStyle( QgsDecorationGrid::Cross );
  }
  else if ( mGridTypeComboBox->currentText() == tr( "Marker" ) )
  {
    mDeco.setGridStyle( QgsDecorationGrid::Marker );
  }
  else
  {
    mDeco.setGridStyle( QgsDecorationGrid::Solid );
  }
  mDeco.setCrossLength( mCrossWidthSpinBox->value() );
  mDeco.setAnnotationFrameDistance( mDistanceToMapFrameSpinBox->value() );
  if ( mAnnotationPositionComboBox->currentText() == tr( "Inside frame" ) )
  {
    mDeco.setGridAnnotationPosition( QgsDecorationGrid::InsideMapFrame );
  }
  else
  {
    mDeco.setGridAnnotationPosition( QgsDecorationGrid::OutsideMapFrame );
  }
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


// void QgsDecorationGridDialog::on_mLineColorButton_clicked()
// {
// QColor newColor = QColorDialog::getColor( mLineColorButton->color() );
// if ( newColor.isValid() )
// {
//   mLineColorButton->setColor( newColor );
//   mDeco.setGridPenColor( newColor );
// }
// }

void QgsDecorationGridDialog::on_mGridTypeComboBox_currentIndexChanged( int index )
{
  mLineSymbolButton->setEnabled( index == QgsDecorationGrid::Solid );
  mCrossWidthSpinBox->setEnabled( index == QgsDecorationGrid::Cross );
  mMarkerSymbolButton->setEnabled( index == QgsDecorationGrid::Marker );
}


void QgsDecorationGridDialog::on_mLineSymbolButton_clicked()
{
  if ( ! mLineSymbol )
    return;

  QgsLineSymbolV2* lineSymbol = dynamic_cast<QgsLineSymbolV2*>( mLineSymbol->clone() );
  QgsSymbolV2PropertiesDialog dlg( lineSymbol, 0, this );
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
  QgsSymbolV2PropertiesDialog dlg( markerSymbol, 0, this );
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

void QgsDecorationGridDialog::on_mAnnotationFontButton_clicked()
{
  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mDeco.gridAnnotationFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mDeco.gridAnnotationFont() );
#endif
  if ( ok )
  {
    mDeco.setGridAnnotationFont( newFont );
  }
}
