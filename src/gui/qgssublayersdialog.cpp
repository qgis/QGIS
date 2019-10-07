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
#include "qgssettings.h"
#include "qgsgui.h"

#include <QTableWidgetItem>
#include <QPushButton>

//! @cond
class SubLayerItem : public QTreeWidgetItem
{
  public:
    SubLayerItem( const QStringList &strings, int type = QTreeWidgetItem::Type )
      :  QTreeWidgetItem( strings, type )
    {}

    bool operator <( const QTreeWidgetItem &other ) const override
    {
      QgsSublayersDialog *d = qobject_cast<QgsSublayersDialog *>( treeWidget()->parent() );
      int col = treeWidget()->sortColumn();

      if ( col == 0 || ( col > 0 && d->countColumn() == col ) )
        return text( col ).toInt() < other.text( col ).toInt();
      else
        return text( col ) < other.text( col );
    }
};
//! @endcond

QgsSublayersDialog::QgsSublayersDialog( ProviderType providerType, const QString &name,
                                        QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mName( name )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  if ( providerType == QgsSublayersDialog::Ogr )
  {
    setWindowTitle( tr( "Select Vector Layers to Add…" ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Number of features" ) << tr( "Geometry type" ) << tr( "Description" ) );
    mShowCount = true;
    mShowType = true;
    mShowDescription = true;
  }
  else if ( providerType == QgsSublayersDialog::Gdal )
  {
    setWindowTitle( tr( "Select Raster Layers to Add…" ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) );
  }
  else
  {
    setWindowTitle( tr( "Select Layers to Add…" ) );
    layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                  << tr( "Type" ) );
    mShowType = true;
  }

  // add a "Select All" button - would be nicer with an icon
  QPushButton *button = new QPushButton( tr( "Select All" ) );
  buttonBox->addButton( button, QDialogButtonBox::ActionRole );
  connect( button, &QAbstractButton::pressed, layersTable, &QTreeView::selectAll );
  // connect( pbnSelectNone, SIGNAL( pressed() ), SLOT( layersTable->selectNone() ) );

  // Checkbox about adding sublayers to a group
  mCheckboxAddToGroup = new QCheckBox( tr( "Add layers to a group" ), this );
  buttonBox->addButton( mCheckboxAddToGroup, QDialogButtonBox::ActionRole );
  mCheckboxAddToGroup->setVisible( false );
}

QgsSublayersDialog::~QgsSublayersDialog()
{
  QgsSettings settings;
  settings.setValue( "/Windows/" + mName + "SubLayers/headerColumnCount",
                     layersTable->columnCount() );
  settings.setValue( "/Windows/" + mName + "SubLayers/headerState",
                     layersTable->header()->saveState() );
}

static bool _isLayerIdUnique( int layerId, QTreeWidget *layersTable )
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
    QTreeWidgetItem *item = layersTable->selectedItems().at( i );

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


void QgsSublayersDialog::populateLayerTable( const QgsSublayersDialog::LayerDefinitionList &list )
{
  const auto constList = list;
  for ( const LayerDefinition &item : constList )
  {
    QStringList elements;
    elements << QString::number( item.layerId ) << item.layerName;
    if ( mShowCount )
      elements << ( item.count == -1 ? tr( "Unknown" ) : QString::number( item.count ) );
    if ( mShowType )
      elements << item.type;
    if ( mShowDescription )
      elements << item.description;
    layersTable->addTopLevelItem( new SubLayerItem( elements ) );
  }

  // resize columns
  QgsSettings settings;
  QByteArray ba = settings.value( "/Windows/" + mName + "SubLayers/headerState" ).toByteArray();
  int savedColumnCount = settings.value( "/Windows/" + mName + "SubLayers/headerColumnCount" ).toInt();
  if ( ! ba.isNull() && savedColumnCount == layersTable->columnCount() )
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
  QgsSettings settings;
  QString promptLayers = settings.value( QStringLiteral( "qgis/promptForSublayers" ), 1 ).toString();

  // make sure three are sublayers to choose
  if ( layersTable->topLevelItemCount() == 0 )
    return QDialog::Rejected;

  // check promptForSublayers settings - perhaps this should be in QgsDataSource instead?
  if ( promptLayers == QLatin1String( "no" ) )
    return QDialog::Rejected;
  else if ( promptLayers == QLatin1String( "all" ) )
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

  // Checkbox about adding sublayers to a group
  if ( mShowAddToGroupCheckbox )
  {
    mCheckboxAddToGroup->setVisible( true );
    bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), false ).toBool();
    mCheckboxAddToGroup->setChecked( addToGroup );
  }

  int ret = QDialog::exec();
  if ( overrideCursor )
    QApplication::setOverrideCursor( cursor );

  if ( mShowAddToGroupCheckbox )
    settings.setValue( QStringLiteral( "/qgis/openSublayersInGroup" ), mCheckboxAddToGroup->isChecked() );
  return ret;
}
