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
/* $Id$ */

#include <iostream>
#include <vector>

#include <qtable.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>

#include "qgsattributeactiondialog.h"
#include "qgsattributeaction.h"
#include "qgsfield.h"

QgsAttributeActionDialog::QgsAttributeActionDialog(QgsAttributeAction* actions,
						   const std::vector<QgsField>& fields,
						   QWidget* parent):
  QgsAttributeActionDialogBase(parent), mActions(actions)
{
  init();
  // Populate the combo box with the field names. Will the field names
  // change? If so, they need to be passed into the init() call, or
  // some access to them retained in this class.
  for (int i = 0; i < fields.size(); ++i)
    fieldComboBox->insertItem(fields[i].name());
}

void QgsAttributeActionDialog::init()
{
  // Can these be moved to the .ui file?
  attributeActionTable->setColumnStretchable(0, true);
  attributeActionTable->setColumnStretchable(1, true);

  // Start from a fresh slate.
  for (int i = attributeActionTable->numRows()-1; i >= 0; --i)
    attributeActionTable->removeRow(i);

  // Populate with our actions.
  QgsAttributeAction::AttributeActions::const_iterator 
    iter = mActions->begin();
  int i = 0;
  for (; iter != mActions->end(); ++iter, ++i)
  {
    attributeActionTable->insertRows(i);
    attributeActionTable->setText(i, 0, iter->name());
    attributeActionTable->setText(i, 1, iter->action());
  }
}

void QgsAttributeActionDialog::moveUp()
{
  // Swap the selected row with the one above

  int row1 = -1, row2 = -1;
  for (int i = 0; i < attributeActionTable->numRows(); ++i)
    if (attributeActionTable->isRowSelected(i))
      row1 = i;

  if (row1 > 0)
    row2 = row1 - 1;
  
  if (row1 != -1 && row2 != -1)
  {
    for (int i = 0; i < attributeActionTable->numSelections(); ++i)
      attributeActionTable->removeSelection(i);

    attributeActionTable->swapRows(row1, row2);
    attributeActionTable->updateContents();
    // Move the selection to follow
    attributeActionTable->selectRow(row2);
  }
}

void QgsAttributeActionDialog::moveDown()
{
  // Swap the selected row with the one below
  int row1 = -1, row2 = -1;
  for (int i = 0; i < attributeActionTable->numRows(); ++i)
    if (attributeActionTable->isRowSelected(i))
      row1 = i;

  if (row1 < attributeActionTable->numRows()-1)
    row2 = row1 + 1;
  
  if (row1 != -1 && row2 != -1)
  {
    for (int i = 0; i < attributeActionTable->numSelections(); ++i)
      attributeActionTable->removeSelection(i);

    attributeActionTable->swapRows(row1, row2);
    attributeActionTable->updateContents();
    // Move the selection to follow
    attributeActionTable->selectRow(row2);
  }
}

void QgsAttributeActionDialog::browse()
{
  // Popup a file browser and place the results into the actionName
  // widget 

  QString action = QFileDialog::getOpenFileName(
	QString::null, QString::null, this, 
	"Select action dialog", "Select an action");

  if (!action.isNull())
    actionAction->insert(action);    
}

void QgsAttributeActionDialog::remove()
{
  // Remove the selected row. Remember which row was selected.
  int row = -1;
  for (int i = 0; i < attributeActionTable->numRows(); ++i)
    if (attributeActionTable->isRowSelected(i))
    {
      row = i;
      break;
    }

  if (row != -1)
  {
    attributeActionTable->removeRow(row);
    attributeActionTable->clearSelection();

    // And select the row below the one that was selected or the last
    // one, or none.
    // Note something is not quite right here. The highlight in the
    // QTable isn't turning on when a row is selected. Don't
    // understand why. Needs a bit more investigation.
    if (row < attributeActionTable->numRows())
      attributeActionTable->selectRow(row);
    else if (attributeActionTable->numRows() > 0)
      attributeActionTable->selectRow(attributeActionTable->numRows()-1);
  }
}

void QgsAttributeActionDialog::insert()
{
  // Add the action details as a new row in the table. 

  int pos = attributeActionTable->numRows();
  insert(pos);
}

void QgsAttributeActionDialog::insert(int pos)
{
  // Get the action details and insert into the table at the given
  // position. Name needs to be unique, so make it so if required. 

  // If the new action name is the same as the action name in the
  // given pos, don't make the new name unique (because we're
  // replacing it).

  QString name;
  if (actionName->text() == attributeActionTable->text(pos, 0))
    name = actionName->text();
  else
    name = uniqueName(actionName->text());

  QString action = actionAction->text();

  // Expand the table to have a row with index pos
  int numRows = attributeActionTable->numRows();
  if (pos >= numRows)
    attributeActionTable->insertRows(numRows, pos-numRows+1);

  attributeActionTable->setText(pos, 0, name);
  attributeActionTable->setText(pos, 1, action);
}

void QgsAttributeActionDialog::update()
{
  // Updates the action that is selected with the
  // action details.
  for (int i = 0; i < attributeActionTable->numRows(); ++i)
    if (attributeActionTable->isRowSelected(i))
    {
      insert(i);
      break;
    }
}

void QgsAttributeActionDialog::insertField()
{
  // Take the selected field, preprend a % and insert into the action
  // field at the cursor position

  if (!fieldComboBox->currentText().isNull())
  {
    QString field("%");
    field += fieldComboBox->currentText();
    actionAction->insert(field);
  }
}

void QgsAttributeActionDialog::apply()
{
  // Update the contents of mActions from the UI.

  mActions->clearActions();
  for (int i = 0; i < attributeActionTable->numRows(); ++i)
  {
    if (!attributeActionTable->text(i, 0).isEmpty() &&
	!attributeActionTable->text(i, 1).isEmpty())
    {
      mActions->addAction(attributeActionTable->text(i, 0),
			  attributeActionTable->text(i, 1));
    }
  }
}

void QgsAttributeActionDialog::rowSelected(int row, int col, int button, 
					   const QPoint& pos)
{
  // The user has selected a row. We take the contents of that row and
  // populate the edit section of the dialog so that they can change
  // the row if desired.

  actionName->setText(attributeActionTable->text(row, 0));
  actionAction->setText(attributeActionTable->text(row, 1));
}

QString QgsAttributeActionDialog::uniqueName(QString name)
{
  // Make sure that the given name is unique, adding a numerical
  // suffix if necessary.

  int pos = attributeActionTable->numRows();
  bool unique = true;

  for (int i = 0; i < pos; ++i)
  {
    if (attributeActionTable->text(i, 0) == name)
      unique = false;
  }

  if (!unique)
  {
    int suffix_num = 1;
    QString new_name;
    while (!unique)
    {
      QString suffix = QString::number(suffix_num);
      new_name = name + "_" + suffix;
      unique = true;
      for (int i = 0; i < pos; ++i)
	if (attributeActionTable->text(i, 0) == new_name)
	  unique = false;
      ++suffix_num;
    }
    name = new_name;
  }
  return name;
}

