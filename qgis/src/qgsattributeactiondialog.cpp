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

#include "qgsattributeactiondialog.h"
#include "qgsattributeaction.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"

QgsAttributeActionDialog::QgsAttributeActionDialog(QgsAttributeAction* actions, QWidget* parent):
  QgsAttributeActionDialogBase(parent), mActions(actions)
{
}

void QgsAttributeActionDialog::init()
{
  // Can these be moved to the .ui file?
  attributeActionTable->setColumnStretchable(0, true);
  attributeActionTable->setColumnStretchable(1, true);

  // Start from a fresh slate. (or more efficiently, remove or insert
  // rows to have one more than the number of actions).
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
  // Always have a blank row at the end for the user to enter new
  // actions. 
  attributeActionTable->insertRows(i);
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
      mActions->addAction(attributeActionTable->text(i,0),
			  attributeActionTable->text(i,1));
    }
  }
  // Regenerate the actions items. This effectively gets rid of blank
  // lines, making things nice and tidy for when the user comes back
  // to this dialog box.
  init();
}


void QgsAttributeActionDialog::add()
{
  // Put a new row in the table
  attributeActionTable->insertRows(attributeActionTable->numRows(), 1);
}


void QgsAttributeActionDialog::remove()
{
  // Remove the contents from the selected rows
  int nRows = attributeActionTable->numRows();
  for (int i = 0; i < nRows; ++i)
  {
    if (attributeActionTable->isRowSelected(i))
    {
      attributeActionTable->setText(i,0,"");
      attributeActionTable->setText(i,1,"");
    }
  }
}

void QgsAttributeActionDialog::clearAll()
{
  // Remove the contents from all rows in the table
  int nRows = attributeActionTable->numRows();
  for (int i = 0; i < nRows; ++i)
  {
    attributeActionTable->setText(i,0,"");
    attributeActionTable->setText(i,1,"");
  }
}
