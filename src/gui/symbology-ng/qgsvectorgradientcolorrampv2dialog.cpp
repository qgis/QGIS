/***************************************************************************
    qgsvectorgradientcolorrampv2dialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"
#include "qgsdialog.h"
#include "qgscolordialog.h"
#include "qgscptcityarchive.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QPainter>
#include <QSettings>
#include <QTableWidget>
#include <QTextEdit>

QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( ramp )
    , mCurrentItem( nullptr )
{
  setupUi( this );
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  btnColor1->setAllowAlpha( true );
  btnColor1->setColorDialogTitle( tr( "Select ramp color" ) );
  btnColor1->setContext( "symbology" );
  btnColor1->setShowNoColor( true );
  btnColor1->setNoColorString( tr( "Transparent" ) );
  btnColor2->setAllowAlpha( true );
  btnColor2->setColorDialogTitle( tr( "Select ramp color" ) );
  btnColor2->setContext( "symbology" );
  btnColor2->setShowNoColor( true );
  btnColor2->setNoColorString( tr( "Transparent" ) );
  updateColorButtons();
  connect( btnColor1, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor1( const QColor& ) ) );
  connect( btnColor2, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor2( const QColor& ) ) );

  // fill type combobox
  cboType->blockSignals( true );
  cboType->addItem( tr( "Discrete" ) );
  cboType->addItem( tr( "Continuous" ) );
  if ( mRamp->isDiscrete() )
    cboType->setCurrentIndex( 0 );
  else
    cboType->setCurrentIndex( 1 );
  cboType->blockSignals( false );

  if ( mRamp->info().isEmpty() )
    btnInformation->setEnabled( false );

  mStopEditor->setGradientRamp( *mRamp );
  connect( mStopEditor, SIGNAL( changed() ), this, SLOT( updateRampFromStopEditor() ) );
  connect( mStopEditor, SIGNAL( selectedStopChanged( QgsGradientStop ) ), this, SLOT( selectedStopChanged( QgsGradientStop ) ) );
  mStopEditor->selectStop( 0 );

  connect( mColorWidget, SIGNAL( currentColorChanged( QColor ) ), this, SLOT( colorWidgetChanged( QColor ) ) );
  connect( mDeleteStopButton, SIGNAL( clicked() ), mStopEditor, SLOT( deleteSelectedStop() ) );
}

void QgsVectorGradientColorRampV2Dialog::on_cboType_currentIndexChanged( int index )
{
  if (( index == 0 && mRamp->isDiscrete() ) ||
      ( index == 1 && !mRamp->isDiscrete() ) )
    return;
  mRamp->convertToDiscrete( index == 0 );
  updateColorButtons();
  updateStopEditor();
}

void QgsVectorGradientColorRampV2Dialog::on_btnInformation_pressed()
{
  if ( mRamp->info().isEmpty() )
    return;

  QgsDialog *dlg = new QgsDialog( this );
  QLabel *label = nullptr;

  // information table
  QTableWidget *tableInfo = new QTableWidget( dlg );
  tableInfo->verticalHeader()->hide();
  tableInfo->horizontalHeader()->hide();
  tableInfo->setRowCount( mRamp->info().count() );
  tableInfo->setColumnCount( 2 );
  int i = 0;
  QgsStringMap rampInfo = mRamp->info();
  for ( QgsStringMap::const_iterator it = rampInfo.constBegin();
        it != rampInfo.constEnd(); ++it )
  {
    if ( it.key().startsWith( "cpt-city" ) )
      continue;
    tableInfo->setItem( i, 0, new QTableWidgetItem( it.key() ) );
    tableInfo->setItem( i, 1, new QTableWidgetItem( it.value() ) );
    tableInfo->resizeRowToContents( i );
    i++;
  }
  tableInfo->resizeColumnToContents( 0 );
  tableInfo->horizontalHeader()->setStretchLastSection( true );
  tableInfo->setRowCount( i );
  tableInfo->setFixedHeight( tableInfo->rowHeight( 0 ) * i + 5 );
  dlg->layout()->addWidget( tableInfo );
  dlg->resize( 600, 250 );

  dlg->layout()->addSpacing( 5 );

  // gradient file
  QString gradientFile = mRamp->info().value( "cpt-city-gradient" );
  if ( ! gradientFile.isNull() )
  {
    QString fileName = gradientFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = gradientFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
    }
    label = new QLabel( tr( "Gradient file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
  }

  // license file
  QString licenseFile = mRamp->info().value( "cpt-city-license" );
  if ( !licenseFile.isNull() )
  {
    QString fileName = licenseFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = licenseFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
    }
    label = new QLabel( tr( "License file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
    if ( QFile::exists( fileName ) )
    {
      QTextEdit *textEdit = new QTextEdit( dlg );
      textEdit->setReadOnly( true );
      QFile file( fileName );
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        textEdit->setText( file.readAll() );
        file.close();
        dlg->layout()->addSpacing( 5 );
        dlg->layout()->addWidget( textEdit );
        dlg->resize( 600, 500 );
      }
    }
  }

  dlg->show(); //non modal
}

void QgsVectorGradientColorRampV2Dialog::updateColorButtons()
{
  btnColor1->blockSignals( true );
  btnColor1->setColor( mRamp->color1() );
  btnColor1->blockSignals( false );
  btnColor2->blockSignals( true );
  btnColor2->setColor( mRamp->color2() );
  btnColor2->blockSignals( false );
}

void QgsVectorGradientColorRampV2Dialog::updateStopEditor()
{
  mStopEditor->blockSignals( true );
  mStopEditor->setGradientRamp( *mRamp );
  mStopEditor->blockSignals( false );
}

void QgsVectorGradientColorRampV2Dialog::selectedStopChanged( const QgsGradientStop& stop )
{
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( stop.color );
  mColorWidget->blockSignals( false );
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( stop.offset * 100 );
  mPositionSpinBox->blockSignals( false );

  if (( stop.offset == 0 && stop.color == mRamp->color1() ) || ( stop.offset == 1.0 && stop.color == mRamp->color2() ) )
  {
    //first/last stop can't be repositioned
    mPositionSpinBox->setDisabled( true );
    mDeleteStopButton->setDisabled( true );
  }
  else
  {
    mPositionSpinBox->setDisabled( false );
    mDeleteStopButton->setDisabled( false );
  }
}

void QgsVectorGradientColorRampV2Dialog::colorWidgetChanged( const QColor &color )
{
  mStopEditor->setSelectedStopColor( color );
}

void QgsVectorGradientColorRampV2Dialog::on_mPositionSpinBox_valueChanged( double val )
{
  mStopEditor->setSelectedStopOffset( val / 100.0 );
}

void QgsVectorGradientColorRampV2Dialog::updateRampFromStopEditor()
{
  *mRamp = mStopEditor->gradientRamp();
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( mStopEditor->selectedStop().offset * 100 );
  mPositionSpinBox->blockSignals( false );
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( mStopEditor->selectedStop().color );
  mColorWidget->blockSignals( false );
}

void QgsVectorGradientColorRampV2Dialog::setColor1( const QColor& color )
{
  mStopEditor->setColor1( color );
  updateColorButtons();
}

void QgsVectorGradientColorRampV2Dialog::setColor2( const QColor& color )
{
  mStopEditor->setColor2( color );
  updateColorButtons();
}
