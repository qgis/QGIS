/***************************************************************************
    qgssublayersdialog.cpp  - dialog for selecting sublayers
    ---------------------
    begin                : January 2009
    copyright            : (C) 2009 by Florian El Ahdab
    email                : felahdab at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssublayersdialog.h"

#include "qgslogger.h"

#include <QSettings>
#include <QTableWidgetItem>
#include <QPushButton>


QgsSublayersDialog::QgsSublayersDialog( ProviderType providerType, QString name,
                                        QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mName( name )
{
  setupUi( this );

  if ( providerType == QgsSublayersDialog::Ogr )
  {
    setWindowTitle( tr( "Select vector layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Nb of features" ) << tr( "Geometry type" ) );
  }
  else if ( providerType == QgsSublayersDialog::Gdal )
  {
    setWindowTitle( tr( "Select raster layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) );
  }
  else
  {
    setWindowTitle( tr( "Select layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Type" ) );
  }

  // add a "Select All" button - would be nicer with an icon
  QPushButton* button = new QPushButton( tr( "Select All" ) );
  buttonBox->addButton( button, QDialogButtonBox::ActionRole );
  connect( button, SIGNAL( pressed() ), layersTable, SLOT( selectAll() ) );
  // connect( pbnSelectNone, SIGNAL( pressed() ), SLOT( layersTable->selectNone() ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/" + mName + "SubLayers/geometry" ).toByteArray() );
}

QgsSublayersDialog::~QgsSublayersDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/" + mName + "SubLayers/geometry", saveGeometry() );
  settings.setValue( "/Windows/" + mName + "SubLayers/headerState",
                     layersTable->header()->saveState() );
}

QStringList QgsSublayersDialog::selectionNames()
{
  QStringList list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    list << layersTable->selectedItems().at( i )->text( 1 );
  }
  return list;
}

QList<int> QgsSublayersDialog::selectionIndexes()
{
  QList<int> list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    list << layersTable->selectedItems().at( i )->text( 0 ).toInt();
  }
  return list;
}

void QgsSublayersDialog::populateLayerTable( QStringList theList, QString delim )
{
  foreach ( QString item, theList )
  {
    layersTable->addTopLevelItem( new QTreeWidgetItem( item.split( delim ) ) );
  }

  // resize columns
  QSettings settings;
  QByteArray ba = settings.value( "/Windows/" + mName + "SubLayers/headerState" ).toByteArray();
  if ( ! ba.isNull() )
  {
    layersTable->header()->restoreState( ba );
  }
  else
  {
    for ( int i = 0; i < layersTable->columnCount(); i++ )
      layersTable->resizeColumnToContents( i );
    layersTable->setColumnWidth( 1, layersTable->columnWidth( 1 ) + 10 );
  }
}

// override exec() instead of using showEvent()
// because in some case we don't want the dialog to appear (depending on user settings)
// TODO alert the user when dialog is not opened
int QgsSublayersDialog::exec()
{
  QSettings settings;
  QString promptLayers = settings.value( "/qgis/promptForSublayers", 1 ).toString();

  // make sure three are sublayers to choose
  if ( layersTable->topLevelItemCount() == 0 )
    return QDialog::Rejected;

  // check promptForSublayers settings - perhaps this should be in QgsDataSource instead?
  if ( promptLayers == "no" )
    return QDialog::Rejected;
  else if ( promptLayers == "all" )
  {
    layersTable->selectAll();
    return QDialog::Accepted;
  }

  // if there is only 1 sublayer (probably the main layer), just select that one and return
  if ( layersTable->topLevelItemCount() == 1 )
  {
    layersTable->selectAll();
    return QDialog::Accepted;
  }

  // if we got here, disable override cursor, open dialog and return result
  // TODO add override cursor where it is missing (e.g. when opening via "Add Raster")
  QCursor cursor;
  bool override = ( QApplication::overrideCursor() != 0 );
  if ( override )
  {
    cursor = QCursor( * QApplication::overrideCursor() );
    QApplication::restoreOverrideCursor();
  }
  int ret = QDialog::exec();
  if ( override )
    QApplication::setOverrideCursor( cursor );
  return ret;
}
