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

#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QImageWriter>
#include <QTableWidget>

QgsAttributeActionDialog::QgsAttributeActionDialog( const QgsActionManager& actions, QWidget* parent )
    : QWidget( parent )
    , mLayer( actions.layer() )
{
  setupUi( this );
  QHeaderView* header = mAttributeActionTable->horizontalHeader();
  header->setHighlightSections( false );
  header->setStretchLastSection( true );
  mAttributeActionTable->setColumnWidth( 0, 100 );
  mAttributeActionTable->setColumnWidth( 1, 230 );
  mAttributeActionTable->setCornerButtonEnabled( false );
  mAttributeActionTable->setEditTriggers( QAbstractItemView::AnyKeyPressed | QAbstractItemView::SelectedClicked );

  connect( mAttributeActionTable, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ), this, SLOT( itemDoubleClicked( QTableWidgetItem* ) ) );
  connect( mAttributeActionTable, SIGNAL( itemSelectionChanged() ), this, SLOT( updateButtons() ) );
  connect( mMoveUpButton, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
  connect( mMoveDownButton, SIGNAL( clicked() ), this, SLOT( moveDown() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( remove() ) );
  connect( mAddButton, SIGNAL( clicked( bool ) ), this, SLOT( insert() ) );
  connect( mAddDefaultActionsButton, SIGNAL( clicked() ), this, SLOT( addDefaultActions() ) );

  init( actions, mLayer->attributeTableConfig() );
}

void QgsAttributeActionDialog::init( const QgsActionManager& actions, const QgsAttributeTableConfig& attributeTableConfig )
{
  // Start from a fresh slate.
  mAttributeActionTable->setRowCount( 0 );

  int i = 0;
  // Populate with our actions.
  Q_FOREACH ( const QgsAction& action, actions.listActions() )
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

void QgsAttributeActionDialog::insertRow( int row, const QgsAction& action )
{
  QTableWidgetItem* item;
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
  mAttributeActionTable->setItem( row, ActionText, new QTableWidgetItem( action.action() ) );

  // Capture output
  item = new QTableWidgetItem();
  item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
  item->setCheckState( action.capture() ? Qt::Checked : Qt::Unchecked );
  mAttributeActionTable->setItem( row, Capture, item );

  // Capture output
  item = new QTableWidgetItem();
  item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
  item->setCheckState( action.showInAttributeTable() ? Qt::Checked : Qt::Unchecked );
  mAttributeActionTable->setItem( row, ShowInAttributeTable, item );

  // Icon
  QIcon icon = action.icon();
  QTableWidgetItem* headerItem = new QTableWidgetItem( icon, "" );
  headerItem->setData( Qt::UserRole, action.iconPath() );
  mAttributeActionTable->setVerticalHeaderItem( row, headerItem );

  updateButtons();
}

void QgsAttributeActionDialog::insertRow( int row, QgsAction::ActionType type, const QString& name, const QString& actionText, const QString& iconPath, bool capture )
{
  insertRow( row, QgsAction( type, name, actionText, iconPath, capture ) );
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
  QTableWidgetItem* header = mAttributeActionTable->takeVerticalHeaderItem( row1 );
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
                    mAttributeActionTable->item( row, ShowInAttributeTable )->checkState() == Qt::Checked,
                    mAttributeActionTable->item( row, ShortTitle )->text() );
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
  dlg.setWindowTitle( tr( "Add new action" ) );

  if ( dlg.exec() )
  {
    QString name = uniqueName( dlg.description() );

    insertRow( pos, dlg.type(), name, dlg.actionText(), dlg.iconPath(), dlg.capture() );
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
  insertRow( pos++, QgsAction::Generic, tr( "Echo attribute's value" ), "echo \"[% \"MY_FIELD\" %]\"", "", true );
  insertRow( pos++, QgsAction::Generic, tr( "Run an application" ), "ogr2ogr -f \"ESRI Shapefile\" \"[% \"OUTPUT_PATH\" %]\" \"[% \"INPUT_FILE\" %]\"", "", true );
  insertRow( pos++, QgsAction::GenericPython, tr( "Get feature id" ), "QtGui.QMessageBox.information(None, \"Feature id\", \"feature id is [% $id %]\")", "", false );
  insertRow( pos++, QgsAction::GenericPython, tr( "Selected field's value (Identify features tool)" ), "QtGui.QMessageBox.information(None, \"Current field's value\", \"[% @current_field %]\")", "", false );
  insertRow( pos++, QgsAction::GenericPython, tr( "Clicked coordinates (Run feature actions tool)" ), "QtGui.QMessageBox.information(None, \"Clicked coords\", \"layer: [% @layer_id %]\\ncoords: ([% @click_x %],[% @click_y %])\")", "", false );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Open file" ), "[% \"PATH\" %]", "", false );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Search on web based on attribute's value" ), "http://www.google.com/search?q=[% \"ATTRIBUTE\" %]", "", false );
}

void QgsAttributeActionDialog::itemDoubleClicked( QTableWidgetItem* item )
{
  int row = item->row();

  QgsAttributeActionPropertiesDialog actionProperties(
    static_cast<QgsAction::ActionType>( mAttributeActionTable->item( row, Type )->data( Qt::UserRole ).toInt() ),
    mAttributeActionTable->item( row, Description )->text(),
    mAttributeActionTable->item( row, ShortTitle )->text(),
    mAttributeActionTable->verticalHeaderItem( row )->data( Qt::UserRole ).toString(),
    mAttributeActionTable->item( row, ActionText )->text(),
    mAttributeActionTable->item( row, Capture )->checkState() == Qt::Checked,
    mAttributeActionTable->item( row, ShowInAttributeTable )->checkState() == Qt::Checked,
    mLayer
  );

  actionProperties.setWindowTitle( tr( "Edit action" ) );

  if ( actionProperties.exec() )
  {
    mAttributeActionTable->item( row, Type )->setData( Qt::UserRole, actionProperties.type() );
    mAttributeActionTable->item( row, Type )->setText( textForType( actionProperties.type() ) );
    mAttributeActionTable->item( row, Description )->setText( actionProperties.description() );
    mAttributeActionTable->item( row, ShortTitle )->setText( actionProperties.shortTitle() );
    mAttributeActionTable->item( row, ActionText )->setText( actionProperties.actionText() );
    mAttributeActionTable->item( row, Capture )->setCheckState( actionProperties.capture() ? Qt::Checked : Qt::Unchecked );
    mAttributeActionTable->item( row, ShowInAttributeTable )->setCheckState( actionProperties.showInAttributeTable() ? Qt::Checked : Qt::Unchecked );
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
    if ( mAttributeActionTable->item( i, 0 )->text() == name )
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
