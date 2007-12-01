/***************************************************************************
                         qgsattributedialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
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
#include "qgsattributedialog.h"
#include "qgsfield.h"
#include "qgslogger.h"

#include <QTableWidgetItem>
#include <QSettings>

QgsAttributeDialog::QgsAttributeDialog(const QgsFieldMap& fields, const QgsAttributeMap& attributes)
  : QDialog(),
    _settingsPath("/Windows/AttributeDialog/"),
    mRowIsDirty(attributes.size(), FALSE)
{
    restorePositionAndColumnWidth();

    setupUi(this);
    mTable->setRowCount(attributes.size());

    int index=0;
    for (QgsAttributeMap::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
    {
      // set attribute name
      
      QString fieldName = fields[it.key()].name();

      QTableWidgetItem * myFieldItem = new QTableWidgetItem(fieldName);
      myFieldItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      mTable->setItem(index, 0, myFieldItem);

      // set attribute value

      QTableWidgetItem * myValueItem = new QTableWidgetItem((*it).toString());
      mTable->setItem(index, 1, myValueItem);

      ++index;
    }

    // setup the mechanism to track edited attribute values
    // if we do it this way, only edited attributes will
    // be attempted to be saved when the editing session stops.
    connect(mTable, SIGNAL(cellChanged(int, int)),
            this,   SLOT  (setAttributeValueChanged(int, int)));

    mTable->resizeColumnsToContents();
}

QgsAttributeDialog::~QgsAttributeDialog()
{

}

QString QgsAttributeDialog::value(int row)
{
  return mTable->item(row,1)->text();
}

bool QgsAttributeDialog::isDirty(int row)
{
  return mRowIsDirty.at(row);
}

bool QgsAttributeDialog::queryAttributes(const QgsFieldMap& fields, QgsFeature& f)
{
  QgsAttributeMap featureAttributes = f.attributeMap();
  QgsAttributeDialog attdialog(fields, featureAttributes);

  if (attdialog.exec() == QDialog::Accepted)
  { 
    int i=0;
    for (QgsAttributeMap::const_iterator it = featureAttributes.begin(); it != featureAttributes.end(); ++it)
    {
      f.changeAttribute(it.key(), QVariant(attdialog.value(i++)) );
    }
    return true;
  }
  else
  {
    return false;
  }
}

void QgsAttributeDialog::savePositionAndColumnWidth()
{
  QSettings settings;
  settings.setValue(_settingsPath+"geometry", saveGeometry());
}

void QgsAttributeDialog::resizeEvent(QResizeEvent *event)
 {
  savePositionAndColumnWidth();
  QWidget::resizeEvent(event);
 }

void QgsAttributeDialog::moveEvent(QMoveEvent *event)
 {
  savePositionAndColumnWidth();
  QWidget::moveEvent(event);
 }

void QgsAttributeDialog::restorePositionAndColumnWidth()
{
  QSettings settings;
  restoreGeometry(settings.value(_settingsPath+"geometry").toByteArray());
}

void QgsAttributeDialog::setAttributeValueChanged(int row, int column)
{
  QgsDebugMsg("Entered with row " + QString::number(row) +
              ", column " + QString::number(column) + ".");

  mRowIsDirty.at(row) = TRUE;
}
