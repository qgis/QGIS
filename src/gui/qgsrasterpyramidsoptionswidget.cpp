/***************************************************************************
                          qgsrasterpyramidsoptionswidget.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterpyramidsoptionswidget.h"
#include "qgslogger.h"
#include "qgsdialog.h"

#include "gdal.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_minixml.h"

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QCheckBox>


QgsRasterPyramidsOptionsWidget::QgsRasterPyramidsOptionsWidget( QWidget* parent, QString provider )
  : QWidget( parent ), mProvider( provider )
{
  setupUi( this );
 
  mPyramidsOptionsWidget->setProvider( provider );
  mPyramidsOptionsWidget->setFormat( "_pyramids" );
  // mPyramidsOptionsWidget->swapOptionsUI( 1 );

  updateUi();
}

QgsRasterPyramidsOptionsWidget::~QgsRasterPyramidsOptionsWidget()
{
}


void QgsRasterPyramidsOptionsWidget::updateUi()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // cbxPyramidsInternal->setChecked( mySettings.value( prefix + "internal", false ).toBool() );
  tmpStr = mySettings.value( prefix + "format", "gtiff" ).toString();
  if ( tmpStr == "internal" )
    cboPyramidsFormat->setCurrentIndex( 0 );
  else if ( tmpStr == "erdas" )
    cboPyramidsFormat->setCurrentIndex( 2 );
  else
    cboPyramidsFormat->setCurrentIndex( 1 );
  cboResamplingMethod->setCurrentIndex( cboResamplingMethod->findText( 
    mySettings.value( prefix + "resampling", "Average" ).toString() ) );
  lePyramidsLevels->setText( mySettings.value( prefix + "overviewStr", "bla" ).toString() );

  // overview list
  if ( mOverviewCheckBoxes.isEmpty() )
  {
    QList<int> overviewList;
    overviewList << 2 << 4 << 8 << 16 << 32 << 64;
    mOverviewCheckBoxes.clear();
    foreach( int i, overviewList )
    {
      mOverviewCheckBoxes[ i ] = new QCheckBox( QString::number( i ), this );
      layoutPyramidLevels->addWidget( mOverviewCheckBoxes[ i ] );
    }
  }
  else
  {
    foreach( int i, mOverviewCheckBoxes.keys() )
      mOverviewCheckBoxes[ i ]->setChecked( false );
  }
  tmpStr = mySettings.value( prefix + "overviewList", "" ).toString();
  foreach( QString lev, tmpStr.split( " ", QString::SkipEmptyParts ) )
  {
    if( mOverviewCheckBoxes.contains( lev.toInt() ) )
      mOverviewCheckBoxes[ lev.toInt() ]->setChecked( true );
  }
  on_lePyramidsLevels_editingFinished();
  
  mPyramidsOptionsWidget->updateProfiles();
}

void QgsRasterPyramidsOptionsWidget::apply()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;
 
  // mySettings.setValue( prefix + "internal", cbxPyramidsInternal->isChecked() );
  if ( cboPyramidsFormat->currentIndex() == 0 )
    tmpStr = "internal";
  else if ( cboPyramidsFormat->currentIndex() == 2 )
    tmpStr = "erdas";
  else 
    tmpStr = "tiff";
  mySettings.setValue( prefix + "format", tmpStr );
  mySettings.setValue( prefix + "resampling", cboResamplingMethod->currentText().trimmed() );
  mySettings.setValue( prefix + "overviewStr", lePyramidsLevels->text().trimmed() );

  // overview list
  tmpStr = "";
  foreach( int i, mOverviewCheckBoxes.keys() )
  {
    if ( mOverviewCheckBoxes[ i ]->isChecked() )
      tmpStr += QString::number( i ) + " ";
  }
  mySettings.setValue( prefix + "overviewList", tmpStr.trimmed() );

  mPyramidsOptionsWidget->apply();
}

void QgsRasterPyramidsOptionsWidget::on_lePyramidsLevels_editingFinished()
{
  // validate string, only space-separated positive integers are allowed
  // should we also validate that numbers are increasing?
  QString tmpStr;
  int tmpInt;
  foreach( QString lev, lePyramidsLevels->text().trimmed().split( " ", QString::SkipEmptyParts ) )
  {
    tmpInt = lev.toInt();
    if ( tmpInt > 0 )
      tmpStr += QString::number( tmpInt ) + " ";
  }
  lePyramidsLevels->setText( tmpStr.trimmed() );

  // if text is non-empty, disable checkboxes
  if ( lePyramidsLevels->text() == "" )
  {
    foreach( int i, mOverviewCheckBoxes.keys() )
      mOverviewCheckBoxes[ i ]->setEnabled( true );
  }
  else
  {
    foreach( int i, mOverviewCheckBoxes.keys() )
      mOverviewCheckBoxes[ i ]->setEnabled( false );
  }
}
