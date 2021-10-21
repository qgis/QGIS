/***************************************************************************
 qgsdecorationtilegriddialog.cpp
 -------------------------------
 Date: 24-Nov-2021
 Copyright: (C) 2021 by Jochen Topf
 Email: jochen@topf.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationtilegriddialog.h"

#include "qgsdecorationtilegrid.h"

#include "qgshelp.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgisapp.h"
#include "qgsguiutils.h"
#include "qgsgui.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

QgsDecorationTileGridDialog::QgsDecorationTileGridDialog( QgsDecorationTileGrid &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationTileGridDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationTileGridDialog::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationTileGridDialog::showHelp );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsDecorationTileGridDialog::apply );

  mGridTypeComboBox->addItem( tr( "Line" ), QgsDecorationTileGrid::GridStyle::Line );
  mGridTypeComboBox->addItem( tr( "Marker" ), QgsDecorationTileGrid::GridStyle::Marker );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { updateSymbolButtons(); } );

  mMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
  mMarkerSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mMarkerSymbolButton->setMessageBar( QgisApp::instance()->messageBar() );

  mLineSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  mLineSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mLineSymbolButton->setMessageBar( QgisApp::instance()->messageBar() );

  mDynamicGrid->setEnabled( true );
  mStaticGrid->setEnabled( true );
  mZoomLevelSpinBox->setEnabled( false );
  connect( mDynamicGrid, &QRadioButton::toggled, this, [ = ] { updateGridButtons(); } );

  buttonGroupCoordinatesDisplay->setId( mCoordinatesNone, static_cast<int>( QgsDecorationTileGrid::GridAnnotationStyle::None ) );
  buttonGroupCoordinatesDisplay->setId( mCoordinatesCenter, static_cast<int>( QgsDecorationTileGrid::GridAnnotationStyle::Center ) );
  buttonGroupCoordinatesDisplay->setId( mCoordinatesBorder, static_cast<int>( QgsDecorationTileGrid::GridAnnotationStyle::Border ) );
  connect( mCoordinatesNone, qOverload<bool>( &QAbstractButton::toggled ), [ = ]( bool checked ) { mAnnotationFontButton->setEnabled( ! checked ); } );

  mAnnotationFontButton->setEnabled( ! mCoordinatesNone->isChecked() );
  mAnnotationFontButton->setDialogTitle( tr( "Tile Coordinates Text Format" ) );
  mAnnotationFontButton->setMapCanvas( QgisApp::instance()->mapCanvas() );

  updateGuiElements();
  updateSymbolButtons();
  updateGridButtons();
}

void QgsDecorationTileGridDialog::updateGuiElements()
{
  grpEnable->setChecked( mDeco.enabled() );

  mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findData( mDeco.gridStyle() ) );

  mDynamicGrid->setChecked( mDeco.dynamicOrStaticGrid() == QgsDecorationTileGrid::DynamicOrStaticGrid::Dynamic );
  mStaticGrid->setChecked( mDeco.dynamicOrStaticGrid() == QgsDecorationTileGrid::DynamicOrStaticGrid::Static );

  mZoomLevelSpinBox->setValue( mDeco.zoomLevel() );
  mZoomFactorSlider->setValue( mDeco.zoomFactor() * 100 );

  buttonGroupCoordinatesDisplay->button( static_cast<int>( mDeco.gridAnnotationStyle() ) )->setChecked( true );

  mAnnotationFontButton->setTextFormat( mDeco.textFormat() );

  mLineSymbolButton->setSymbol( mDeco.lineSymbol()->clone() );
  mMarkerSymbolButton->setSymbol( mDeco.markerSymbol()->clone() );

  whileBlocking( mAnnotationFontButton )->setCurrentFont( mDeco.gridAnnotationFont() );
}

void QgsDecorationTileGridDialog::updateDecoFromGui()
{
  mDeco.setEnabled( grpEnable->isChecked() );

  mDeco.setGridStyle( static_cast< QgsDecorationTileGrid::GridStyle >( mGridTypeComboBox->currentData().toInt() ) );
  mDeco.setLineSymbol( mLineSymbolButton->clonedSymbol< QgsLineSymbol >() );
  mDeco.setMarkerSymbol( mMarkerSymbolButton->clonedSymbol< QgsMarkerSymbol >() );

  if ( mDynamicGrid->isChecked() )
    mDeco.setDynamicOrStaticGrid( QgsDecorationTileGrid::DynamicOrStaticGrid::Dynamic );
  else
    mDeco.setDynamicOrStaticGrid( QgsDecorationTileGrid::DynamicOrStaticGrid::Static );
  mDeco.setZoomFactor( mZoomFactorSlider->value() / 100 );
  mDeco.setZoomLevel( mZoomLevelSpinBox->value() );

  mDeco.setGridAnnotationStyle( static_cast<QgsDecorationTileGrid::GridAnnotationStyle>( buttonGroupCoordinatesDisplay->checkedId() ) );
  mDeco.setTextFormat( mAnnotationFontButton->textFormat() );
}

void QgsDecorationTileGridDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#tilegrid" ) ); // XXX
}

void QgsDecorationTileGridDialog::buttonBox_accepted()
{
  updateDecoFromGui();
  accept();
}

void QgsDecorationTileGridDialog::apply()
{
  updateDecoFromGui();
  mDeco.update();
}

void QgsDecorationTileGridDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationTileGridDialog::updateSymbolButtons()
{
  switch ( mGridTypeComboBox->currentData().toInt() )
  {
    case ( QgsDecorationTileGrid::GridStyle::Marker ):
    {
      mMarkerSymbolButton->setVisible( true );
      mMarkerSymbolButton->setEnabled( grpEnable->isChecked() );
      mMarkerSymbolLabel->setVisible( true );
      mLineSymbolButton->setVisible( false );
      mLineSymbolButton->setEnabled( false );
      mLineSymbolLabel->setVisible( false );
      break;
    }
    case ( QgsDecorationTileGrid::GridStyle::Line ):
    {
      mMarkerSymbolButton->setVisible( false );
      mMarkerSymbolButton->setEnabled( false );
      mMarkerSymbolLabel->setVisible( false );
      mLineSymbolButton->setVisible( true );
      mLineSymbolButton->setEnabled( grpEnable->isChecked() );
      mLineSymbolLabel->setVisible( true );
      break;
    }
  }
}

void QgsDecorationTileGridDialog::updateGridButtons()
{
  const bool is_dynamic = mDynamicGrid->isChecked();
  mZoomFactorLabel->setEnabled( is_dynamic );
  mZoomFactorSlider->setEnabled( is_dynamic );
  mZoomLevelLabel->setEnabled( ! is_dynamic );
  mZoomLevelSpinBox->setEnabled( ! is_dynamic );
}
