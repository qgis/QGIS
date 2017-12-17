/***************************************************************************
                qgsattributeactiondialog.cpp  -  attribute action dialog
                             -------------------

This class creates and manages the Action tab of the Vector Layer
Properties dialog box. Changes made in the dialog box are propagated
back to QgsVectorLayer.

    begin                : October 2004
    copyright            : (C) 2004 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeactiondialog.h"
#include "qgsactionmanager.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsattributeactionpropertiesdialog.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QImageWriter>
#include <QTableWidget>

QgsAttributeActionDialog::QgsAttributeActionDialog( const QgsActionManager &actions, QWidget *parent )
  : QWidget( parent )
  , mLayer( actions.layer() )
{
  setupUi( this );
  QHeaderView *header = mAttributeActionTable->horizontalHeader();
  header->setHighlightSections( false );
  header->setStretchLastSection( true );
  mAttributeActionTable->setColumnWidth( 0, 100 );
  mAttributeActionTable->setColumnWidth( 1, 230 );
  mAttributeActionTable->setCornerButtonEnabled( false );
  mAttributeActionTable->setEditTriggers( QAbstractItemView::AnyKeyPressed | QAbstractItemView::SelectedClicked );

  connect( mAttributeActionTable, &QTableWidget::itemDoubleClicked, this, &QgsAttributeActionDialog::itemDoubleClicked );
  connect( mAttributeActionTable, &QTableWidget::itemSelectionChanged, this, &QgsAttributeActionDialog::updateButtons );
  connect( mMoveUpButton, &QAbstractButton::clicked, this, &QgsAttributeActionDialog::moveUp );
  connect( mMoveDownButton, &QAbstractButton::clicked, this, &QgsAttributeActionDialog::moveDown );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsAttributeActionDialog::remove );
  connect( mAddButton, &QAbstractButton::clicked, this, &QgsAttributeActionDialog::insert );
  connect( mAddDefaultActionsButton, &QAbstractButton::clicked, this, &QgsAttributeActionDialog::addDefaultActions );

  init( actions, mLayer->attributeTableConfig() );
}

void QgsAttributeActionDialog::init( const QgsActionManager &actions, const QgsAttributeTableConfig &attributeTableConfig )
{
  // Start from a fresh slate.
  mAttributeActionTable->setRowCount( 0 );

  int i = 0;
  // Populate with our actions.
  Q_FOREACH ( const QgsAction &action, actions.actions() )
  {
    insertRow( i++, action );
  }

  updateButtons();

  QgsAttributeTableConfig::ColumnConfig visibleActionWidgetConfig = QgsAttributeTableConfig::ColumnConfig();
  visibleActionWidgetConfig.type = QgsAttributeTableConfig::Action;
  visibleActionWidgetConfig.hidden = false;

  mShowInAttributeTable->setChecked( attributeTableConfig.actionWidgetVisible() );
  mAttributeTableWidgetType->setCurrentIndex( attributeTableConfig.actionWidgetStyle() );
}

QList<QgsAction> QgsAttributeActionDialog::actions() const
{
  QList<QgsAction> actions;

  for ( int i = 0; i < mAttributeActionTable->rowCount(); ++i )
  {
    actions.append( rowToAction( i ) );
  }

  return actions;
}

bool QgsAttributeActionDialog::showWidgetInAttributeTable() const
{
  return mShowInAttributeTable->isChecked();
}

QgsAttributeTableConfig::ActionWidgetStyle QgsAttributeActionDialog::attributeTableWidgetStyle() const
{
  return static_cast<QgsAttributeTableConfig::ActionWidgetStyle>( mAttributeTableWidgetType->currentIndex() );
}

void QgsAttributeActionDialog::insertRow( int row, const QgsAction &action )
{
  QTableWidgetItem *item = nullptr;
  mAttributeActionTable->insertRow( row );

  // Type
  item = new QTableWidgetItem( textForType( action.type() ) );
  item->setData( Qt::UserRole, action.type() );
  item->setFlags( item->flags() & ~Qt::ItemIsEditable );
  mAttributeActionTable->setItem( row, Type, item );

  // Description
  mAttributeActionTable->setItem( row, Description, new QTableWidgetItem( action.name() ) );

  // Short Title
  mAttributeActionTable->setItem( row, ShortTitle, new QTableWidgetItem( action.shortTitle() ) );

  // Action text
  mAttributeActionTable->setItem( row, ActionText, new QTableWidgetItem( action.command() ) );

  // Capture output
  item = new QTableWidgetItem();
  item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
  item->setCheckState( action.capture() ? Qt::Checked : Qt::Unchecked );
  mAttributeActionTable->setItem( row, Capture, item );

  // Scopes
  item = new QTableWidgetItem();
  item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
  QStringList actionScopes = action.actionScopes().toList();
  std::sort( actionScopes.begin(), actionScopes.end() );
  item->setText( actionScopes.join( QStringLiteral( ", " ) ) );
  item->setData( Qt::UserRole, QVariant::fromValue<QSet<QString>>( action.actionScopes() ) );
  mAttributeActionTable->setItem( row, ActionScopes, item );

  // Icon
  QIcon icon = action.icon();
  QTableWidgetItem *headerItem = new QTableWidgetItem( icon, QLatin1String( "" ) );
  headerItem->setData( Qt::UserRole, action.iconPath() );
  mAttributeActionTable->setVerticalHeaderItem( row, headerItem );

  // Notification message
  mAttributeActionTable->setItem( row, NotificationMessage, new QTableWidgetItem( action.notificationMessage() ) );

  updateButtons();
}

void QgsAttributeActionDialog::insertRow( int row, QgsAction::ActionType type, const QString &name, const QString &actionText, const QString &iconPath, bool capture, const QString &shortTitle, const QSet<QString> &actionScopes, const QString &notificationMessage )
{
  if ( uniqueName( name ) == name )
    insertRow( row, QgsAction( type, name, actionText, iconPath, capture, shortTitle, actionScopes, notificationMessage ) );
}

void QgsAttributeActionDialog::moveUp()
{
  // Swap the selected row with the one above

  int row1 = -1, row2 = -1;
  QList<QTableWidgetItem *> selection = mAttributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    row1 = selection.first()->row();
  }

  if ( row1 > 0 )
    row2 = row1 - 1;

  if ( row1 != -1 && row2 != -1 )
  {
    swapRows( row1, row2 );
    // Move the selection to follow
    mAttributeActionTable->selectRow( row2 );
  }
}

void QgsAttributeActionDialog::moveDown()
{
  // Swap the selected row with the one below
  int row1 = -1, row2 = -1;
  QList<QTableWidgetItem *> selection = mAttributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    row1 = selection.first()->row();
  }

  if ( row1 < mAttributeActionTable->rowCount() - 1 )
    row2 = row1 + 1;

  if ( row1 != -1 && row2 != -1 )
  {
    swapRows( row1, row2 );
    // Move the selection to follow
    mAttributeActionTable->selectRow( row2 );
  }
}

void QgsAttributeActionDialog::swapRows( int row1, int row2 )
{
  int colCount = mAttributeActionTable->columnCount();
  for ( int col = 0; col < colCount; col++ )
  {
    QTableWidgetItem *item = mAttributeActionTable->takeItem( row1, col );
    mAttributeActionTable->setItem( row1, col, mAttributeActionTable->takeItem( row2, col ) );
    mAttributeActionTable->setItem( row2, col, item );
  }
  QTableWidgetItem *header = mAttributeActionTable->takeVerticalHeaderItem( row1 );
  mAttributeActionTable->setVerticalHeaderItem( row1, mAttributeActionTable->takeVerticalHeaderItem( row2 ) );
  mAttributeActionTable->setVerticalHeaderItem( row2, header );
}

QgsAction QgsAttributeActionDialog::rowToAction( int row ) const
{
  QgsAction action( static_cast<QgsAction::ActionType>( mAttributeActionTable->item( row, Type )->data( Qt::UserRole ).toInt() ),
                    mAttributeActionTable->item( row, Description )->text(),
                    mAttributeActionTable->item( row, ActionText )->text(),
                    mAttributeActionTable->verticalHeaderItem( row )->data( Qt::UserRole ).toString(),
                    mAttributeActionTable->item( row, Capture )->checkState() == Qt::Checked,
                    mAttributeActionTable->item( row, ShortTitle )->text(),
                    mAttributeActionTable->item( row, ActionScopes )->data( Qt::UserRole ).value<QSet<QString>>(),
                    mAttributeActionTable->item( row, NotificationMessage )->text()
                  );
  return action;
}

QString QgsAttributeActionDialog::textForType( QgsAction::ActionType type )
{
  switch ( type )
  {
    case QgsAction::Generic:
      return tr( "Generic" );
    case QgsAction::GenericPython:
      return tr( "Python" );
    case QgsAction::Mac:
      return tr( "Mac" );
    case QgsAction::Windows:
      return tr( "Windows" );
    case QgsAction::Unix:
      return tr( "Unix" );
    case QgsAction::OpenUrl:
      return tr( "Open URL" );
  }
  return QString();
}

void QgsAttributeActionDialog::remove()
{
  QList<QTableWidgetItem *> selection = mAttributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    // Remove the selected row.
    int row = selection.first()->row();
    mAttributeActionTable->removeRow( row );

    // And select the row below the one that was selected or the last one.
    if ( row >= mAttributeActionTable->rowCount() )
      row = mAttributeActionTable->rowCount() - 1;
    mAttributeActionTable->selectRow( row );

    updateButtons();
  }
}

void QgsAttributeActionDialog::insert()
{
  // Add the action details as a new row in the table.
  int pos = mAttributeActionTable->rowCount();

  QgsAttributeActionPropertiesDialog dlg( mLayer, this );
  dlg.setWindowTitle( tr( "Add New Action" ) );

  if ( dlg.exec() )
  {
    QString name = uniqueName( dlg.description() );

    insertRow( pos, dlg.type(), name, dlg.actionText(), dlg.iconPath(), dlg.capture(), dlg.shortTitle(), dlg.actionScopes(), dlg.notificationMessage() );
  }
}

void QgsAttributeActionDialog::updateButtons()
{
  QList<QTableWidgetItem *> selection = mAttributeActionTable->selectedItems();
  bool hasSelection = !selection.isEmpty();

  if ( hasSelection )
  {
    int row = selection.first()->row();
    mMoveUpButton->setEnabled( row >= 1 );
    mMoveDownButton->setEnabled( row >= 0 && row < mAttributeActionTable->rowCount() - 1 );
  }
  else
  {
    mMoveUpButton->setEnabled( false );
    mMoveDownButton->setEnabled( false );
  }

  mRemoveButton->setEnabled( hasSelection );
}

void QgsAttributeActionDialog::addDefaultActions()
{
  int pos = 0;
  insertRow( pos++, QgsAction::Generic, tr( "Echo attribute's value" ), QStringLiteral( "echo \"[% \"MY_FIELD\" %]\"" ), QLatin1String( "" ), true, tr( "Attribute Value" ), QSet<QString>() << QStringLiteral( "Field" ), QString() );
  insertRow( pos++, QgsAction::Generic, tr( "Run an application" ), QStringLiteral( "ogr2ogr -f \"GPKG\" \"[% \"OUTPUT_PATH\" %]\" \"[% \"INPUT_FILE\" %]\"" ), QLatin1String( "" ), true, tr( "Run application" ), QSet<QString>() << QStringLiteral( "Feature" ) << QStringLiteral( "Canvas" ), QString() );
  insertRow( pos++, QgsAction::GenericPython, tr( "Get feature id" ), QStringLiteral( "from qgis.PyQt import QtWidgets\n\nQtWidgets.QMessageBox.information(None, \"Feature id\", \"feature id is [% $id %]\")" ), QLatin1String( "" ), false, tr( "Feature ID" ), QSet<QString>() << QStringLiteral( "Feature" ) << QStringLiteral( "Canvas" ), QString() );
  insertRow( pos++, QgsAction::GenericPython, tr( "Selected field's value (Identify features tool)" ), QStringLiteral( "from qgis.PyQt import QtWidgets\n\nQtWidgets.QMessageBox.information(None, \"Current field's value\", \"[% @current_field %]\")" ), QLatin1String( "" ), false, tr( "Field Value" ), QSet<QString>() << QStringLiteral( "Field" ), QString() );
  insertRow( pos++, QgsAction::GenericPython, tr( "Clicked coordinates (Run feature actions tool)" ), QStringLiteral( "from qgis.PyQt import QtWidgets\n\nQtWidgets.QMessageBox.information(None, \"Clicked coords\", \"layer: [% @layer_id %]\\ncoords: ([% @click_x %],[% @click_y %])\")" ), QLatin1String( "" ), false, tr( "Clicked Coordinate" ), QSet<QString>() << QStringLiteral( "Canvas" ), QString() );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Open file" ), QStringLiteral( "[% \"PATH\" %]" ), QLatin1String( "" ), false, tr( "Open file" ), QSet<QString>() << QStringLiteral( "Feature" ) << QStringLiteral( "Canvas" ), QString() );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Search on web based on attribute's value" ), QStringLiteral( "http://www.google.com/search?q=[% \"ATTRIBUTE\" %]" ), QLatin1String( "" ), false, tr( "Search Web" ), QSet<QString>() << QStringLiteral( "Field" ), QString() );
  insertRow( pos++, QgsAction::GenericPython, tr( "List feature ids" ), QStringLiteral( "from qgis.PyQt import QtWidgets\n\nlayer = QgsProject.instance().mapLayer('[% @layer_id %]')\nif layer.selectedFeatureCount():\n    ids = layer.selectedFeatureIds()\nelse:\n    ids = [f.id() for f in layer.getFeatures()]\n\nQtWidgets.QMessageBox.information(None, \"Feature ids\", ', '.join([str(id) for id in ids]))" ), QLatin1String( "" ), false, tr( "List feature ids" ), QSet<QString>() << QStringLiteral( "Layer" ), QString() );
  insertRow( pos++, QgsAction::GenericPython, tr( "Duplicate features" ), QStringLiteral( "project = QgsProject.instance()\nlayer = QgsProject.instance().mapLayer('[% @layer_id %]')\nlayer.startEditing()\nfeatures=[]\nif len('[% $id %]')>0:\n    features.append( layer.getFeature( [% $id %] ) )\nelse:\n    for x in layer.selectedFeatures():\n        features.append( x )\nfeature_count=0\nchildren_info=''\nfeatureids=[]\nfor f in features:\n    result=QgsVectorLayerUtils.duplicateFeature(layer, f, project, 0 )\n    featureids.append( result[0].id() )\n    feature_count+=1\n    for ch_layer in result[1].layers():\n        children_info+='{number_of_children} children on layer {children_layer}\\n'.format( number_of_children=str( len( result[1].duplicatedFeatures(ch_layer) ) ), children_layer=ch_layer.name() )\n        ch_layer.selectByIds( result[1].duplicatedFeatures(ch_layer) )\nlayer.selectByIds( featureids )\nqgis.utils.iface.messageBar().pushMessage( '{number_of_features} features on layer {layer} duplicated with\\n{children_info}'.format( number_of_features=str( feature_count ), layer=layer.name(), children_info=children_info ) )" ), QLatin1String( "" ), false, tr( "Duplicate" ), QSet<QString>() << QStringLiteral( "Canvas" ) << QStringLiteral( "Feature" ) << QStringLiteral( "Layer" ), QString() );

}

void QgsAttributeActionDialog::itemDoubleClicked( QTableWidgetItem *item )
{
  int row = item->row();

  QgsAttributeActionPropertiesDialog actionProperties(
    static_cast<QgsAction::ActionType>( mAttributeActionTable->item( row, Type )->data( Qt::UserRole ).toInt() ),
    mAttributeActionTable->item( row, Description )->text(),
    mAttributeActionTable->item( row, ShortTitle )->text(),
    mAttributeActionTable->verticalHeaderItem( row )->data( Qt::UserRole ).toString(),
    mAttributeActionTable->item( row, ActionText )->text(),
    mAttributeActionTable->item( row, Capture )->checkState() == Qt::Checked,
    mAttributeActionTable->item( row, ActionScopes )->data( Qt::UserRole ).value<QSet<QString>>(),
    mAttributeActionTable->item( row, NotificationMessage )->text(),
    mLayer
  );

  actionProperties.setWindowTitle( tr( "Edit Action" ) );

  if ( actionProperties.exec() )
  {
    mAttributeActionTable->item( row, Type )->setData( Qt::UserRole, actionProperties.type() );
    mAttributeActionTable->item( row, Type )->setText( textForType( actionProperties.type() ) );
    mAttributeActionTable->item( row, Description )->setText( actionProperties.description() );
    mAttributeActionTable->item( row, ShortTitle )->setText( actionProperties.shortTitle() );
    mAttributeActionTable->item( row, ActionText )->setText( actionProperties.actionText() );
    mAttributeActionTable->item( row, Capture )->setCheckState( actionProperties.capture() ? Qt::Checked : Qt::Unchecked );
    mAttributeActionTable->item( row, NotificationMessage )->setText( actionProperties.notificationMessage() );

    QTableWidgetItem *item = mAttributeActionTable->item( row, ActionScopes );
    QStringList actionScopes = actionProperties.actionScopes().toList();
    std::sort( actionScopes.begin(), actionScopes.end() );
    item->setText( actionScopes.join( QStringLiteral( ", " ) ) );
    item->setData( Qt::UserRole, QVariant::fromValue<QSet<QString>>( actionProperties.actionScopes() ) );

    mAttributeActionTable->verticalHeaderItem( row )->setData( Qt::UserRole, actionProperties.iconPath() );
    mAttributeActionTable->verticalHeaderItem( row )->setIcon( QIcon( actionProperties.iconPath() ) );
  }
}

QString QgsAttributeActionDialog::uniqueName( QString name )
{
  // Make sure that the given name is unique, adding a numerical
  // suffix if necessary.

  int pos = mAttributeActionTable->rowCount();
  bool unique = true;

  for ( int i = 0; i < pos; ++i )
  {
    if ( mAttributeActionTable->item( i, Description )->text() == name )
      unique = false;
  }

  if ( !unique )
  {
    int suffix_num = 1;
    QString new_name;
    while ( !unique )
    {
      QString suffix = QString::number( suffix_num );
      new_name = name + '_' + suffix;
      unique = true;
      for ( int i = 0; i < pos; ++i )
        if ( mAttributeActionTable->item( i, 0 )->text() == new_name )
          unique = false;
      ++suffix_num;
    }
    name = new_name;
  }
  return name;
}
