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


QgsSublayersDialog::QgsSublayersDialog( ProviderType providerType, const QString& name,
                                        QWidget* parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
    , mName( name )
    , mShowCount( false )
    , mShowType( false )
{
  setupUi( this );

  if ( providerType == QgsSublayersDialog::Ogr )
  {
    setWindowTitle( tr( "Select vector layers to add..." ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Number of features" ) << tr( "Geometry type" ) );
    mShowCount = true;
    mShowType = true;
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
    mShowType = true;
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

static bool _isLayerIdUnique( int layerId, QTreeWidget* layersTable )
{
  int count = 0;
  for ( int j = 0; j < layersTable->topLevelItemCount(); j++ )
  {
    if ( layersTable->topLevelItem( j )->text( 0 ).toInt() == layerId )
    {
      count++;
    }
  }
  return count == 1;
}

QgsSublayersDialog::LayerDefinitionList QgsSublayersDialog::selection()
{
  LayerDefinitionList list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    QTreeWidgetItem* item = layersTable->selectedItems().at( i );

    LayerDefinition def;
    def.layerId = item->text( 0 ).toInt();
    def.layerName = item->text( 1 );
    if ( mShowType )
    {
      // If there are more sub layers of the same name (virtual for geometry types),
      // add geometry type
      if ( !_isLayerIdUnique( def.layerId, layersTable ) )
        def.type = item->text( mShowCount ? 3 : 2 );
    }

    list << def;
  }
  return list;
}


void QgsSublayersDialog::populateLayerTable( const QgsSublayersDialog::LayerDefinitionList& list )
{
  Q_FOREACH ( const LayerDefinition& item, list )
  {
    QStringList elements;
    elements << QString::number( item.layerId ) << item.layerName;
    if ( mShowCount )
      elements << QString::number( item.count );
    if ( mShowType )
      elements << item.type;
    layersTable->addTopLevelItem( new QTreeWidgetItem( elements ) );
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


QStringList QgsSublayersDialog::selectionNames()
{
  QStringList list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    // If there are more sub layers of the same name (virtual for geometry types),
    // add geometry type

    QString name = layersTable->selectedItems().at( i )->text( 1 );
    int count = 0;
    for ( int j = 0; j < layersTable->topLevelItemCount(); j++ )
    {
      if ( layersTable->topLevelItem( j )->text( 1 ) == name )
      {
        count++;
      }
    }

    if ( count > 1 )
    {
      name += ':' + layersTable->selectedItems().at( i )->text( 3 );
    }
    else
    {
      name += ":any";
    }

    list << name;
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

void QgsSublayersDialog::populateLayerTable( const QStringList& theList, const QString& delim )
{
  Q_FOREACH ( const QString& item, theList )
  {
    QStringList elements = item.split( delim );
    while ( elements.size() > 4 )
    {
      elements[1] += delim + elements[2];
      elements.removeAt( 2 );
    }
    layersTable->addTopLevelItem( new QTreeWidgetItem( elements ) );
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

  layersTable->sortByColumn( 1, Qt::AscendingOrder );
  layersTable->setSortingEnabled( true );

  // if we got here, disable override cursor, open dialog and return result
  // TODO add override cursor where it is missing (e.g. when opening via "Add Raster")
  QCursor cursor;
  bool overrideCursor = nullptr != QApplication::overrideCursor();
  if ( overrideCursor )
  {
    cursor = QCursor( * QApplication::overrideCursor() );
    QApplication::restoreOverrideCursor();
  }
  int ret = QDialog::exec();
  if ( overrideCursor )
    QApplication::setOverrideCursor( cursor );
  return ret;
}
