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

#include "qgsdecorationtilegriddialog.h"

#include "qgsdecorationtilegrid.h"

#include "qgslogger.h"
#include "qgshelp.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgisapp.h"
#include "qgsguiutils.h"
#include "qgsgui.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsdoublevalidator.h"

QgsDecorationTileGridDialog::QgsDecorationTileGridDialog( QgsDecorationTileGrid &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationTileGridDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationTileGridDialog::buttonBox_rejected );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { updateSymbolButtons(); } );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationTileGridDialog::showHelp );

  mMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
  mLineSymbolButton->setSymbolType( Qgis::SymbolType::Line );

  grpEnable->setChecked( mDeco.enabled() );
  connect( grpEnable, &QGroupBox::toggled, this, [ = ] { updateSymbolButtons(); } );

  mGridTypeComboBox->addItem( tr( "Line" ), QgsDecorationTileGrid::Line );
  mGridTypeComboBox->addItem( tr( "Marker" ), QgsDecorationTileGrid::Marker );

  mDynamicGrid->setEnabled( true );
  mStaticGrid->setEnabled( true );
  mZoomLevelSpinBox->setEnabled( false );
  connect( mDynamicGrid, &QRadioButton::toggled, this, [ = ] { updateGridButtons(); } );
  connect( mStaticGrid, &QRadioButton::toggled, this, [ = ] { updateGridButtons(); } );
  updateGuiElements();

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsDecorationTileGridDialog::apply );

  // font settings
  mAnnotationFontButton->setDialogTitle( tr( "Annotation Text Format" ) );
  mAnnotationFontButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mAnnotationFontButton->setTextFormat( mDeco.textFormat() );

  mMarkerSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mMarkerSymbolButton->setMessageBar( QgisApp::instance()->messageBar() );
  mLineSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mLineSymbolButton->setMessageBar( QgisApp::instance()->messageBar() );
}

void QgsDecorationTileGridDialog::updateGuiElements()
{

  grpEnable->setChecked( mDeco.enabled() );

  mDynamicGrid->setChecked( mDeco.dynamicOrStaticGrid() == QgsDecorationTileGrid::DynamicOrStaticGrid::Dynamic );
  mStaticGrid->setChecked( mDeco.dynamicOrStaticGrid() == QgsDecorationTileGrid::DynamicOrStaticGrid::Static );

  mZoomLevelSpinBox->setValue( mDeco.zoomLevel() );
  mZoomFactorSlider->setValue( mDeco.zoomFactor() * 100 );

  mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findData( mDeco.gridStyle() ) );
  mDrawAnnotationCheckBox->setChecked( mDeco.showGridAnnotation() );

  mAnnotationFontButton->setTextFormat( mDeco.textFormat() );

  mLineSymbolButton->setSymbol( mDeco.lineSymbol()->clone() );
  mMarkerSymbolButton->setSymbol( mDeco.markerSymbol()->clone() );

  whileBlocking( mAnnotationFontButton )->setCurrentFont( mDeco.gridAnnotationFont() );

  // blockAllSignals( false );
}

void QgsDecorationTileGridDialog::updateDecoFromGui()
{
  mDeco.setDirty( false );
  mDeco.setEnabled( grpEnable->isChecked() );

  if ( mDynamicGrid->isChecked() )
  {
    mDeco.setDynamicOrStaticGrid( QgsDecorationTileGrid::DynamicOrStaticGrid::Dynamic );
  }
  else if ( mStaticGrid->isChecked() )
  {
    mDeco.setDynamicOrStaticGrid( QgsDecorationTileGrid::DynamicOrStaticGrid::Static );
  }

  mDeco.setZoomLevel( mZoomLevelSpinBox->value() );
  mDeco.setZoomFactor( mZoomFactorSlider->value() / 100 );

  mDeco.setGridStyle( static_cast< QgsDecorationTileGrid::GridStyle >( mGridTypeComboBox->currentData().toInt() ) );

  mDeco.setTextFormat( mAnnotationFontButton->textFormat() );
  mDeco.setShowGridAnnotation( mDrawAnnotationCheckBox->isChecked() );
  mDeco.setLineSymbol( mLineSymbolButton->clonedSymbol< QgsLineSymbol >() );
  mDeco.setMarkerSymbol( mMarkerSymbolButton->clonedSymbol< QgsMarkerSymbol >() );
}

void QgsDecorationTileGridDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#tilegrid" ) ); // XXX
}

void QgsDecorationTileGridDialog::buttonBox_accepted()
{
  updateDecoFromGui();
  // mDeco.update();
  accept();
}

void QgsDecorationTileGridDialog::apply()
{
  updateDecoFromGui();
  mDeco.update();
  //accept();
}

void QgsDecorationTileGridDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationTileGridDialog::updateSymbolButtons()
{
  switch ( mGridTypeComboBox->currentData().toInt() )
  {
    case ( QgsDecorationTileGrid::Marker ):
    {
      mMarkerSymbolButton->setVisible( true );
      mMarkerSymbolButton->setEnabled( grpEnable->isChecked() );
      mMarkerSymbolLabel->setVisible( true );
      mLineSymbolButton->setVisible( false );
      mLineSymbolButton->setEnabled( false );
      mLineSymbolLabel->setVisible( false );
      break;
    }
    case ( QgsDecorationTileGrid::Line ):
    {
      mLineSymbolButton->setVisible( true );
      mLineSymbolButton->setEnabled( grpEnable->isChecked() );
      mLineSymbolLabel->setVisible( true );
      mMarkerSymbolButton->setVisible( false );
      mMarkerSymbolButton->setEnabled( false );
      mMarkerSymbolLabel->setVisible( false );
      break;
    }
  }
}

void QgsDecorationTileGridDialog::updateGridButtons()
{
  if ( mDynamicGrid->isChecked() )
  {
    mZoomFactorSlider->setEnabled( true );
    mZoomLevelSpinBox->setEnabled( false );
  }
  else if ( mStaticGrid->isChecked() )
  {
    mZoomFactorSlider->setEnabled( false );
    mZoomLevelSpinBox->setEnabled( true );
  }
}
