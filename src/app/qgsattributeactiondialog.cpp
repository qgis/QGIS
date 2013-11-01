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
#include "qgsattributeaction.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>

QgsAttributeActionDialog::QgsAttributeActionDialog( QgsAttributeAction* actions,
    const QgsFields& fields,
    QWidget* parent ):
    QWidget( parent ), mActions( actions )
{
  setupUi( this );
  QHeaderView *header = attributeActionTable->horizontalHeader();
  header->setHighlightSections( false );
  header->setStretchLastSection( true );
  attributeActionTable->setColumnWidth( 0, 100 );
  attributeActionTable->setColumnWidth( 1, 230 );
  attributeActionTable->setCornerButtonEnabled( false );

  connect( attributeActionTable, SIGNAL( itemSelectionChanged() ),
           this, SLOT( itemSelectionChanged() ) );
  connect( actionName, SIGNAL( textChanged( QString ) ), this, SLOT( updateButtons() ) );
  connect( actionAction, SIGNAL( textChanged() ), this, SLOT( updateButtons() ) );

  connect( moveUpButton, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
  connect( moveDownButton, SIGNAL( clicked() ), this, SLOT( moveDown() ) );
  connect( removeButton, SIGNAL( clicked() ), this, SLOT( remove() ) );
  connect( addDefaultActionsButton, SIGNAL( clicked() ), this, SLOT( addDefaultActions() ) );

  connect( browseButton, SIGNAL( clicked() ), this, SLOT( browse() ) );
  connect( insertButton, SIGNAL( clicked() ), this, SLOT( insert() ) );
  connect( updateButton, SIGNAL( clicked() ), this, SLOT( update() ) );
  connect( insertFieldButton, SIGNAL( clicked() ), this, SLOT( insertField() ) );
  connect( insertExpressionButton, SIGNAL( clicked() ), this, SLOT( insertExpression() ) );

  init();
  // Populate the combo box with the field names. Will the field names
  // change? If so, they need to be passed into the init() call, or
  // some access to them retained in this class.
  for ( int idx = 0; idx < fields.count(); ++idx )
    fieldComboBox->addItem( fields[idx].name() );
}

void QgsAttributeActionDialog::init()
{
  // Start from a fresh slate.
  attributeActionTable->setRowCount( 0 );

  // Populate with our actions.
  for ( int i = 0; i < mActions->size(); i++ )
  {
    const QgsAction action = ( *mActions )[i];
    insertRow( i, action.type(), action.name(), action.action(), action.capture() );
  }

  updateButtons();
}

void QgsAttributeActionDialog::insertRow( int row, QgsAction::ActionType type, const QString &name, const QString &action, bool capture )
{
  QTableWidgetItem* item;
  attributeActionTable->insertRow( row );
  item = new QTableWidgetItem( actionType->itemText( type ) );
  item->setFlags( item->flags() & ~Qt::ItemIsEditable );
  attributeActionTable->setItem( row, 0, item );
  attributeActionTable->setItem( row, 1, new QTableWidgetItem( name ) );
  attributeActionTable->setItem( row, 2, new QTableWidgetItem( action ) );
  item = new QTableWidgetItem();
  item->setFlags( item->flags() & ~( Qt::ItemIsEditable | Qt::ItemIsUserCheckable ) );
  item->setCheckState( capture ? Qt::Checked : Qt::Unchecked );
  attributeActionTable->setItem( row, 3, item );

  updateButtons();
}

void QgsAttributeActionDialog::moveUp()
{
  // Swap the selected row with the one above

  int row1 = -1, row2 = -1;
  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
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
    attributeActionTable->selectRow( row2 );
  }
}

void QgsAttributeActionDialog::moveDown()
{
  // Swap the selected row with the one below
  int row1 = -1, row2 = -1;
  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    row1 = selection.first()->row();
  }

  if ( row1 < attributeActionTable->rowCount() - 1 )
    row2 = row1 + 1;

  if ( row1 != -1 && row2 != -1 )
  {
    swapRows( row1, row2 );
    // Move the selection to follow
    attributeActionTable->selectRow( row2 );
  }
}

void QgsAttributeActionDialog::swapRows( int row1, int row2 )
{
  int colCount = attributeActionTable->columnCount();
  for ( int col = 0; col < colCount; col++ )
  {
    QTableWidgetItem *item = attributeActionTable->takeItem( row1, col );
    attributeActionTable->setItem( row1, col, attributeActionTable->takeItem( row2, col ) );
    attributeActionTable->setItem( row2, col, item );
  }
}

void QgsAttributeActionDialog::browse()
{
  // Popup a file browser and place the results into the action widget
  QString action = QFileDialog::getOpenFileName(
                     this, tr( "Select an action", "File dialog window title" ) );

  if ( !action.isNull() )
    actionAction->insertPlainText( action );
}

void QgsAttributeActionDialog::insertExpression()
{
  QString selText = actionAction->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // display the expression builder
  QgsExpressionBuilderDialog dlg( mActions->layer(), selText, this );
  dlg.setWindowTitle( tr( "Insert expression" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mActions->layer()->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression =  dlg.expressionBuilder()->expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      actionAction->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

void QgsAttributeActionDialog::remove()
{
  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    // Remove the selected row.
    int row = selection.first()->row();
    attributeActionTable->removeRow( row );

    // And select the row below the one that was selected or the last one.
    if ( row >= attributeActionTable->rowCount() )
      row = attributeActionTable->rowCount() - 1;
    attributeActionTable->selectRow( row );

    updateButtons();
  }
}

void QgsAttributeActionDialog::insert()
{
  // Add the action details as a new row in the table.
  int pos = attributeActionTable->rowCount();
  insert( pos );
}

void QgsAttributeActionDialog::insert( int pos )
{
  // Check to see if the action name and the action have been specified
  // before proceeding

  if ( actionName->text().isEmpty() || actionAction->toPlainText().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Missing Information" ),
                          tr( "To create an attribute action, you must provide both a name and the action to perform." ) );

  }
  else
  {

    // Get the action details and insert into the table at the given
    // position. Name needs to be unique, so make it so if required.

    // If the new action name is the same as the action name in the
    // given pos, don't make the new name unique (because we're
    // replacing it).

    int numRows = attributeActionTable->rowCount();
    QString name;
    if ( pos < numRows && actionName->text() == attributeActionTable->item( pos, 0 )->text() )
      name = actionName->text();
    else
      name = uniqueName( actionName->text() );

    if ( pos >= numRows )
    {
      // Expand the table to have a row with index pos
      insertRow( pos, ( QgsAction::ActionType ) actionType->currentIndex(), name, actionAction->toPlainText(), captureCB->isChecked() );
    }
    else
    {
      // Update existing row
      attributeActionTable->item( pos, 0 )->setText( actionType->currentText() );
      attributeActionTable->item( pos, 1 )->setText( name );
      attributeActionTable->item( pos, 2 )->setText( actionAction->toPlainText() );
      attributeActionTable->item( pos, 3 )->setCheckState( captureCB->isChecked() ? Qt::Checked : Qt::Unchecked );
    }
  }
}

void QgsAttributeActionDialog::update()
{
  // Updates the action that is selected with the
  // action details.
  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
  if ( !selection.isEmpty() )
  {
    insert( selection.first()->row() );
  }
}

void QgsAttributeActionDialog::updateButtons()
{
  bool validNewAction = !actionName->text().isEmpty() && !actionAction->toPlainText().isEmpty();

  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
  bool hasSelection = !selection.isEmpty();

  if ( hasSelection )
  {
    int row = selection.first()->row();
    moveUpButton->setEnabled( row >= 1 );
    moveDownButton->setEnabled( row >= 0 && row < attributeActionTable->rowCount() - 1 );
  }
  else
  {
    moveUpButton->setEnabled( false );
    moveDownButton->setEnabled( false );
  }

  removeButton->setEnabled( hasSelection );

  insertButton->setEnabled( validNewAction );
  updateButton->setEnabled( hasSelection && validNewAction );
}

void QgsAttributeActionDialog::insertField()
{
  // Convert the selected field to an expression and
  // insert it into the action at the cursor position

  if ( !fieldComboBox->currentText().isNull() )
  {
    QString field = "[% \"";
    field += fieldComboBox->currentText();
    field += "\" %]";
    actionAction->insertPlainText( field );
  }
}

void QgsAttributeActionDialog::apply()
{
  // Update the contents of mActions from the UI.

  mActions->clearActions();
  for ( int i = 0; i < attributeActionTable->rowCount(); ++i )
  {
    const QgsAction::ActionType type = ( QgsAction::ActionType ) actionType->findText( attributeActionTable->item( i, 0 )->text() );
    const QString &name = attributeActionTable->item( i, 1 )->text();
    const QString &action = attributeActionTable->item( i, 2 )->text();
    if ( !name.isEmpty() && !action.isEmpty() )
    {
      QTableWidgetItem *item = attributeActionTable->item( i, 3 );
      mActions->addAction( type, name, action, item->checkState() == Qt::Checked );
    }
  }
}

void QgsAttributeActionDialog::addDefaultActions()
{
  int pos = 0;
  insertRow( pos++, QgsAction::Generic, tr( "Echo attribute's value" ), "echo \"[% \"MY_FIELD\" %]\"", true );
  insertRow( pos++, QgsAction::Generic, tr( "Run an application" ), "ogr2ogr -f \"ESRI Shapefile\" \"[% \"OUTPUT_PATH\" %]\" \"[% \"INPUT_FILE\" %]\"", true );
  insertRow( pos++, QgsAction::GenericPython, tr( "Get feature id" ), "QtGui.QMessageBox.information(None, \"Feature id\", \"feature id is [% $id %]\")", false );
  insertRow( pos++, QgsAction::GenericPython, tr( "Selected field's value (Identify features tool)" ), "QtGui.QMessageBox.information(None, \"Current field's value\", \"[% $currentfield %]\")", false );
  insertRow( pos++, QgsAction::GenericPython, tr( "Clicked coordinates (Run feature actions tool)" ), "QtGui.QMessageBox.information(None, \"Clicked coords\", \"layer: [% $layerid %]\\ncoords: ([% $clickx %],[% $clicky %])\")", false );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Open file" ), "[% \"PATH\" %]", false );
  insertRow( pos++, QgsAction::OpenUrl, tr( "Search on web based on attribute's value" ), "http://www.google.com/search?q=[% \"ATTRIBUTE\" %]", false );
}

void QgsAttributeActionDialog::itemSelectionChanged()
{
  QList<QTableWidgetItem *> selection = attributeActionTable->selectedItems();
  bool hasSelection = !selection.isEmpty();
  if ( hasSelection )
  {
    int row = selection.first()->row();
    rowSelected( row );
  }

  updateButtons();
}

void QgsAttributeActionDialog::rowSelected( int row )
{
  // The user has selected a row. We take the contents of that row and
  // populate the edit section of the dialog so that they can change
  // the row if desired.

  QTableWidgetItem *item = attributeActionTable->item( row, 2 );
  if ( item )
  {
    // Only if a populated row was selected
    actionType->setCurrentIndex( actionType->findText( attributeActionTable->item( row, 0 )->text() ) );
    actionName->setText( attributeActionTable->item( row, 1 )->text() );
    actionAction->setPlainText( attributeActionTable->item( row, 2 )->text() );
    captureCB->setChecked( attributeActionTable->item( row, 3 )->checkState() == Qt::Checked );
  }
}

QString QgsAttributeActionDialog::uniqueName( QString name )
{
  // Make sure that the given name is unique, adding a numerical
  // suffix if necessary.

  int pos = attributeActionTable->rowCount();
  bool unique = true;

  for ( int i = 0; i < pos; ++i )
  {
    if ( attributeActionTable->item( i, 0 )->text() == name )
      unique = false;
  }

  if ( !unique )
  {
    int suffix_num = 1;
    QString new_name;
    while ( !unique )
    {
      QString suffix = QString::number( suffix_num );
      new_name = name + "_" + suffix;
      unique = true;
      for ( int i = 0; i < pos; ++i )
        if ( attributeActionTable->item( i, 0 )->text() == new_name )
          unique = false;
      ++suffix_num;
    }
    name = new_name;
  }
  return name;
}
