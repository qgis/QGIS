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
#include "qgsproviderregistry.h"

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
      const int col = treeWidget()->sortColumn();

      if ( col == 0 || ( col > 0 && d->countColumn() == col ) )
        return text( col ).toInt() < other.text( col ).toInt();
      else
        return text( col ) < other.text( col );
    }
};
//! @endcond

QgsSublayersDialog::QgsSublayersDialog( ProviderType providerType,
                                        const QString &name,
                                        QWidget *parent,
                                        Qt::WindowFlags fl,
                                        const QString &dataSourceUri )
  : QDialog( parent, fl )
  , mName( name )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  QString title;
  switch ( providerType )
  {
    case QgsSublayersDialog::Ogr :
      title = tr( "Select Vector Layers to Add…" );
      layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                    << tr( "Number of features" ) << tr( "Geometry type" ) << tr( "Description" ) );
      mShowCount = true;
      mShowType = true;
      mShowDescription = true;
      break;
    case QgsSublayersDialog::Gdal:
      title = tr( "Select Raster Layers to Add…" );
      layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) );
      break;
    case QgsSublayersDialog::Mdal:
      title = tr( "Select Mesh Layers to Add…" );
      layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Mesh name" ) );
      break;
    default:
      title = tr( "Select Layers to Add…" );
      layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" )
                                    << tr( "Type" ) );
      mShowType = true;
  }

  const QVariantMap dataSourceUriParsed = QgsProviderRegistry::instance()->decodeUri( name, dataSourceUri );
  const QString dataSourceFilePath = dataSourceUriParsed.value( QStringLiteral( "path" ) ).toString();
  const QString filePath = dataSourceFilePath.isEmpty() ? dataSourceUri : dataSourceFilePath;
  const QString fileName = QFileInfo( filePath ).fileName();

  setWindowTitle( fileName.isEmpty() ? title : QStringLiteral( "%1 | %2" ).arg( title, fileName ) );
  mLblFilePath->setText( QDir::toNativeSeparators( QFileInfo( filePath ).canonicalFilePath() ) );
  mLblFilePath->setVisible( ! fileName.isEmpty() );

  // add a "Select All" button - would be nicer with an icon
  connect( mBtnSelectAll, &QAbstractButton::pressed, layersTable, &QTreeView::selectAll );
  connect( mBtnDeselectAll, &QAbstractButton::pressed, this, &QgsSublayersDialog::mBtnDeselectAll_pressed );
  connect( layersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsSublayersDialog::layersTable_selectionChanged );

  mCbxAddToGroup->setVisible( false );
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
      elements << ( item.count == static_cast< int >( Qgis::FeatureCountState::Uncounted ) ||
                    item.count == static_cast< int >( Qgis::FeatureCountState::UnknownCount )
                    ? tr( "Unknown" ) : QString::number( item.count ) );
    if ( mShowType )
      elements << item.type;
    if ( mShowDescription )
      elements << item.description;
    layersTable->addTopLevelItem( new SubLayerItem( elements ) );
  }

  // resize columns
  const QgsSettings settings;
  const QByteArray ba = settings.value( "/Windows/" + mName + "SubLayers/headerState" ).toByteArray();
  const int savedColumnCount = settings.value( "/Windows/" + mName + "SubLayers/headerColumnCount" ).toInt();
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
  const Qgis::SublayerPromptMode promptLayers = settings.enumValue( QStringLiteral( "qgis/promptForSublayers" ), Qgis::SublayerPromptMode::AlwaysAsk );

  // make sure three are sublayers to choose
  if ( layersTable->topLevelItemCount() == 0 )
    return QDialog::Rejected;

  layersTable->selectAll();

  // check promptForSublayers settings - perhaps this should be in QgsDataSource instead?
  if ( promptLayers == Qgis::SublayerPromptMode::NeverAskSkip )
    return QDialog::Rejected;
  else if ( promptLayers == Qgis::SublayerPromptMode::NeverAskLoadAll )
    return QDialog::Accepted;

  // if there is only 1 sublayer (probably the main layer), just select that one and return
  if ( layersTable->topLevelItemCount() == 1 )
    return QDialog::Accepted;

  layersTable->sortByColumn( 1, Qt::AscendingOrder );
  layersTable->setSortingEnabled( true );

  // if we got here, disable override cursor, open dialog and return result
  // TODO add override cursor where it is missing (e.g. when opening via "Add Raster")
  QCursor cursor;
  const bool overrideCursor = nullptr != QApplication::overrideCursor();
  if ( overrideCursor )
  {
    cursor = QCursor( * QApplication::overrideCursor() );
    QApplication::restoreOverrideCursor();
  }

  // Checkbox about adding sublayers to a group
  if ( mShowAddToGroupCheckbox )
  {
    mCbxAddToGroup->setVisible( true );
    const bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), false ).toBool();
    mCbxAddToGroup->setChecked( addToGroup );
  }

  const int ret = QDialog::exec();
  if ( overrideCursor )
    QApplication::setOverrideCursor( cursor );

  if ( mShowAddToGroupCheckbox )
    settings.setValue( QStringLiteral( "/qgis/openSublayersInGroup" ), mCbxAddToGroup->isChecked() );
  return ret;
}

void QgsSublayersDialog::layersTable_selectionChanged( const QItemSelection &, const QItemSelection & )
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( layersTable->selectedItems().length() > 0 );
}

void QgsSublayersDialog::mBtnDeselectAll_pressed()
{
  layersTable->selectionModel()->clear();
}
