/***************************************************************************
     qgsopenrasterdialog.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "qgsrasterlayer.h"
#include "qgsproject.h"

#include "qgsopenrasterdialog.h"

QgsOpenRasterDialog::QgsOpenRasterDialog( QWidget *parent ) :
    QDialog( parent )
{
  setupUi( this );

  QPushButton *okPushButton = buttonBox->button( QDialogButtonBox::Ok );
  okPushButton->setEnabled( false );
}

// ------------------------------- public ---------------------------------- //
void QgsOpenRasterDialog::getRasterOptions( QString &rasterFileName, QString &modifiedFileName, QString &worldFileName )
{
  rasterFileName = leRasterFileName->text();
  modifiedFileName = leModifiedRasterFileName->text();
  worldFileName = mWorldFileName;
}

// ------------------------------ protected -------------------------------- //
void QgsOpenRasterDialog::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  switch ( e->type() )
  {
    case QEvent::LanguageChange:
      retranslateUi( this );
      break;
    default:
      break;
  }
}

// --------------------------- private slots ------------------------------- //
void QgsOpenRasterDialog::on_tbnSelectRaster_clicked()
{
  QSettings settings;
  QString dir = settings.value( "/Plugin-GeoReferencer/rasterdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";

  QString lastUsedFilter = settings.value( "/Plugin-GeoReferencer/lastusedfilter" ).toString();

  QString filters;
  QgsRasterLayer::buildSupportedRasterFileFilter( filters );
  filters.prepend( "(*.*);;" );
  QString rasterFileName = QFileDialog::getOpenFileName( this, tr( "Choose a name of the raster" ), dir,
                           filters, &lastUsedFilter );

  if ( rasterFileName.isEmpty() )
  {
    return;
  }
  leRasterFileName->setText( rasterFileName );

  // do we think that this is a valid raster?
  if ( !QgsRasterLayer::isValidRasterFileName( rasterFileName ) )
  {
    QMessageBox::critical( this, tr( "Error" ),
                           tr( "The selected file is not a valid raster file." ) );
    return;
  }

  QFileInfo fileInfo( rasterFileName );
  settings.setValue( "/Plugin-GeoReferencer/rasterdirectory", fileInfo.path() );
  settings.setValue( "/Plugin-GeoReferencer/lastusedfilter", lastUsedFilter );

  QString modifiedFileName = generateModifiedRasterFileName();
  leModifiedRasterFileName->setText( modifiedFileName );

  // What DOING this code?
  QgsProject* prj = QgsProject::instance();
  QString projBehaviour = settings.value( "/Projections/defaultBehaviour", "prompt" ).toString();
  QString projectCRS = prj->readEntry( "SpatialRefSys", "/ProjectCRSProj4String" );
  int projectCrsId = prj->readNumEntry( "SpatialRefSys", "/ProjectCrsId" );

  settings.setValue( "/Projections/defaultBehaviour", "useProject" );
  prj->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", GEOPROJ4 );
  prj->writeEntry( "SpatialRefSys", "/ProjectCrsId", int( GEOCRS_ID ) );

  settings.setValue( "/Projections/defaultBehaviour", projBehaviour );
  prj->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", projectCRS );
  prj->writeEntry( "SpatialRefSys", "/ProjectCrsId", projectCrsId );
}

void QgsOpenRasterDialog::on_tbnSelectModifiedRaster_clicked()
{
  QSettings settings;
  QString dir = settings.value( "/Plugin-GeoReferencer/rasterdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";

  QString modifiedFileName = QFileDialog::getSaveFileName( this, tr( "Choose a name for the modified raster" ), dir );
  if ( modifiedFileName.right( 4 ) != ".tif" )
    modifiedFileName += ".tif";

  // do we think that this is a valid raster?
  if ( !QgsRasterLayer::isValidRasterFileName( modifiedFileName ) )
  {
    QMessageBox::critical( this, tr( "Error" ),
                           tr( "The selected file is not a valid raster file." ) );
    return;
  }

  QFileInfo fileInfo( modifiedFileName );
  settings.setValue( "/Plugin-GeoReferencer/rasterdirectory", fileInfo.path() );

  leModifiedRasterFileName->setText( modifiedFileName );
}

void QgsOpenRasterDialog::on_leModifiedRasterFileName_textChanged( const QString name )
{
  mWorldFileName = guessWorldFileName( name );

  bool enable = ( leModifiedRasterFileName->text().size() != 0 && leRasterFileName->text().size() != 0 );
  QPushButton *okPushButton = buttonBox->button( QDialogButtonBox::Ok );
  okPushButton->setEnabled( enable );
}

// ------------------------------ private ---------------------------------- //
QString QgsOpenRasterDialog::generateModifiedRasterFileName()
{
  QString modifiedFileName = leRasterFileName->text();
  QFileInfo modifiedFileInfo( modifiedFileName );
  int pos = modifiedFileName.size() - modifiedFileInfo.suffix().size() - 1;
  modifiedFileName.insert( pos, tr( "-modified", "Georeferencer:QgsOpenRasterDialog.cpp - used to modify a user given file name" ) );

  pos = modifiedFileName.size() - modifiedFileInfo.suffix().size();
  modifiedFileName.replace( pos, modifiedFileName.size(), "tif" );

  return modifiedFileName;
}

QString QgsOpenRasterDialog::guessWorldFileName( const QString rasterFileName )
{
  QString worldFileName = "";
  int point = rasterFileName.lastIndexOf( '.' );
  if ( point != -1 && point != rasterFileName.length() - 1 )
    worldFileName = rasterFileName.left( point + 1 ) + "wld";

  return worldFileName;
}
