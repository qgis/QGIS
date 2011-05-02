/***************************************************************************
  oracleselectgui.cpp
  -------------------
    begin                : Oracle Spatial Plugin
    copyright            : (C) Ivan Lucena
    email                : ivan.lucena@pmldnet.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsselectgeoraster_ui.h"
#include "qgsoracleconnect_ui.h"

//GDAL includes

#include "gdal.h"
#include "ogr_api.h"
#include "ogrsf_frmts.h"
#include <cpl_string.h>

#include "qgsvectorlayer.h"

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsOracleSelectGeoraster::QgsOracleSelectGeoraster( QWidget* parent,
    QgisInterface* iface,
    Qt::WFlags fl ) : QDialog( parent, fl ), mIface( iface )
{
  setupUi( this );

  /*
   *  Load the list of connection from the registry
   */

  populateConnectionList();

  /*
   *  Repeat last selected connection
   */

  QSettings settings;
  QString selected = settings.value( "/Oracle/connections/selected" ).toString();
  if ( selected == cmbConnections->currentText() )
  {
    connectToServer();
  }
}

QgsOracleSelectGeoraster::~QgsOracleSelectGeoraster()
{
}

void QgsOracleSelectGeoraster::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/Oracle/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();

  /*
   *  Fillup the combobox with connection names
   */

  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();

  /*
   *  Update the status of several buttons
   */

  if ( keys.begin() == keys.end() )
  {
    btnConnect->setEnabled( false );
    btnEdit->setEnabled( false );
    btnDelete->setEnabled( false );
  }
  else
  {
    btnConnect->setEnabled( true );
    btnEdit->setEnabled( true );
    btnDelete->setEnabled( true );
  }
}

void QgsOracleSelectGeoraster::on_btnNew_clicked()
{
  QgsOracleConnect *oc = new QgsOracleConnect( this, "New Connection" );
  if ( oc->exec() )
  {
    populateConnectionList();
  }
}

void QgsOracleSelectGeoraster::on_btnEdit_clicked()
{
  QgsOracleConnect *oc = new QgsOracleConnect( this, cmbConnections->currentText() );
  if ( oc->exec() )
  {
    populateConnectionList();
  }
}

void QgsOracleSelectGeoraster::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/Oracle/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this,
                                       tr( "Confirm Delete" ),
                                       msg,
                                       QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key + "/database" );
    settings.remove( key + "/username" );
    settings.remove( key + "/password" );
    settings.remove( key + "/savepass" );
    settings.remove( key + "/subdtset" );
    settings.remove( key );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    setConnectionListPosition();
    lineEdit->setText( "" );
    listWidget->clear();
  }
}

void QgsOracleSelectGeoraster::connectToServer()
{
  if ( cmbConnections->currentText().isEmpty() )
    return;

  QSettings settings;
  QString key = "/Oracle/connections/" + cmbConnections->currentText();
  QString username = settings.value( key + "/username" ).toString();
  QString password = settings.value( key + "/password" ).toString();
  QString savepass = settings.value( key + "/savepass" ).toString();
  QString database = settings.value( key + "/database" ).toString();
  QString subdtset = settings.value( key + "/subdtset" ).toString();
  bool makeConnection = true;
  if ( savepass == "false" )
  {
    makeConnection = false;
    QString password = QInputDialog::getText( this,
                       tr( "Password for %1/<password>@%2" ).arg( username ).arg( database ),
                       tr( "Please enter your password:" ),
                       QLineEdit::Password,
                       QString::null,
                       &makeConnection );
  }
  if ( makeConnection )
  {
    settings.setValue( "/Oracle/connections/selected",
                       cmbConnections->currentText() );
    showSelection( subdtset );
    lineEdit->setText( subdtset );
  }
}

void QgsOracleSelectGeoraster::showSelection( const QString & line )
{
  QString identification = line;

  GDALDatasetH hDS = NULL;
  GDALAccess eAccess = GA_ReadOnly;

  /*
   *  Set access mode
   */

  if ( checkBox->checkState() == Qt::Checked )
  {
    eAccess = GA_Update;
  }

  /*
   *  Try to open georaster dataset
   */

  hDS = GDALOpenShared( TO8( identification ), eAccess );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  if ( hDS == NULL )
  {
    QMessageBox::information( this,
                              tr( "Open failed" ),
                              tr( "The connection to %1 failed. Please verify your connection parameters. Make sure you have the GDAL GeoRaster plugin installed." )
                              .arg( identification ) );
    return;
  }
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );

  /*
   *  Get subdataset list
   */

  char **papszMetadata = NULL;
  papszMetadata = GDALGetMetadata( hDS, "SUBDATASETS" );
  int nSubDatasets = CSLCount( papszMetadata );

  /*
   *  Add GeoRaster Layer
   */

  if ( ! nSubDatasets )
  {
    mIface->addRasterLayer( identification );
    GDALClose( hDS );
    return;
  }

  /*
   *  Save subdataset
   */

  QSettings settings;
  settings.setValue( "/Oracle/connections/" +
                     cmbConnections->currentText() + "/subdtset", identification );

  /*
   *  List subdatasets
   */

  QStringList fields = identification.split( ',' );
  QString count = QString::number( nSubDatasets / 2 );

  QString plural = "s";

  if ( count == "1" )
  {
    plural = "";
  }

  if ( fields.size() < 4 )
  {
    labelStatus->setText( QString( "%1 GeoRaster table%2" )
                          .arg( count ).arg( plural ) );
    checkBox->setEnabled( false );
  }
  else if ( fields.size() == 4 )
  {
    labelStatus->setText( QString( "%1 GeoRaster column%2 on table %3" )
                          .arg( count ).arg( plural ).arg( fields[3] ) );
    checkBox->setEnabled( false );
  }
  else if ( fields.size() == 5 )
  {
    labelStatus->setText( QString( "%1 GeoRaster object%2 on table %3 column %4" )
                          .arg( count ).arg( plural ).arg( fields[3] ).arg( fields[4] ) );
    checkBox->setEnabled( true );
  }
  else
  {
    labelStatus->setText( QString( "%1 GeoRaster object%2 on table %3 column %4 where %5" )
                          .arg( count ).arg( plural ).arg( fields[3] ).arg( fields[4] ).arg( fields[5] ) );
    checkBox->setEnabled( true );
  }

  /*
   *  Populate selection list based on subdataset names
   */

  listWidget->clear();
  QListWidgetItem *textItem;

  for ( int i = 0; i < nSubDatasets; i += 2 )
  {
    QString metadata = papszMetadata[i];
    QStringList subdataset = metadata.split( '=' );
    textItem = new QListWidgetItem( subdataset[1] );
    listWidget->addItem( textItem );
  }

  GDALClose( hDS );
}

void QgsOracleSelectGeoraster::on_listWidget_clicked( QModelIndex Index )
{
  if ( lineEdit->text() == listWidget->currentItem()->text() )
  {
    showSelection( lineEdit->text() );
  }
  else
  {
    lineEdit->setText( listWidget->currentItem()->text() );
  }
}

void QgsOracleSelectGeoraster::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QSettings settings;
  QString toSelect = settings.value( "/Oracle/connections/selected" ).toString();
  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  // If we couldn't find the stored item, but there are some,
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if ( cmbConnections->currentIndex() == -1 && cmbConnections->count() > 0 )
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}
