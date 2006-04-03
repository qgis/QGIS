/***************************************************************************
    qgssearchquerybuilder.cpp  - Query builder for search strings
    ----------------------
    begin                : March 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <iostream>
#include <q3listbox.h>
#include <QMessageBox>
#include "qgsfeature.h"
#include "qgssearchquerybuilder.h"
#include "qgssearchstring.h"
#include "qgssearchtreenode.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


QgsSearchQueryBuilder::QgsSearchQueryBuilder(QgsVectorLayer* layer,
                                             QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl), mLayer(layer)
{
  setupUi(this);
  
  setWindowTitle("Search query builder");
  
  // disable unsupported operators
  btnIn->setEnabled(false);
  btnNotIn->setEnabled(false);
  btnPct->setEnabled(false);
  
  // change to ~
  btnILike->setText("~");
  
  lblDataUri->setText(layer->name());
  populateFields();
}

QgsSearchQueryBuilder::~QgsSearchQueryBuilder()
{
}


void QgsSearchQueryBuilder::populateFields()
{
  const std::vector<QgsField>& fields = mLayer->fields();
  for (uint i = 0; i < fields.size(); i++)
  {
    QgsField f = fields[i];
    QString fieldName = f.name();
    mFieldMap[fieldName] = f;
    lstFields->insertItem(fieldName);
  }
}

void QgsSearchQueryBuilder::getFieldValues(uint limit)
{
  // clear the values list 
  lstValues->clear();
  
  // determine the field type
  QgsField field = mFieldMap[lstFields->currentText()];
  QString fieldName = field.name().lower();
  bool numeric = field.isNumeric();
  
  // TODO: need optimized getNextFeature which won't extract geometry
  QgsFeature* fet;
  QString value;
  std::vector<QgsFeatureAttribute>::const_iterator it;
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  provider->reset();
  while ((fet = provider->getNextFeature(true)) &&
         (limit == 0 || lstValues->count() != limit))
  {
    const std::vector<QgsFeatureAttribute>& attributes = fet->attributeMap();
    for (it = attributes.begin(); it != attributes.end(); it++)
    {
      if ( (*it).fieldName().lower() == fieldName)
      {
        value = (*it).fieldValue();
        break;
      }
    }
     
    if (!numeric)
    {
      // put string in single quotes
      value = "'" + value + "'";
    }
    
    // add item only if it's not there already
    if (lstValues->findItem(value) == 0)
      lstValues->insertItem(value);
    
    delete fet;   
  }
  provider->reset();
  
}

void QgsSearchQueryBuilder::on_btnSampleValues_clicked()
{
  getFieldValues(25);
}

void QgsSearchQueryBuilder::on_btnGetAllValues_clicked()
{
  getFieldValues(0);
}

void QgsSearchQueryBuilder::on_btnTest_clicked()
{
  long count = countRecords(txtSQL->text());
  
  // error?
  if (count == -1)
    return;

  QString str;
  if (count)
    str.sprintf(tr("Found %d matching features."), count);
  else
    str = tr("No matching features found.");
  QMessageBox::information(this, tr("Search results"), str);
}

// This method tests the number of records that would be returned
long QgsSearchQueryBuilder::countRecords(QString searchString) 
{
  QgsSearchString search;
  if (!search.setString(searchString))
  {
    QMessageBox::critical(this, tr("Search string parsing error"), search.parserErrorMsg());
    return -1;
  }
  
  QgsSearchTreeNode* searchTree = search.tree();
  if (searchTree == NULL)
  {
    // entered empty search string
    return mLayer->featureCount();
  }
  
  QApplication::setOverrideCursor(Qt::waitCursor);
  
  int count = 0;

  // TODO: need optimized getNextFeature which won't extract geometry
  QgsFeature* fet;
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  provider->reset();
  while ((fet = provider->getNextFeature(true)))
  {
    if (searchTree->checkAgainst(fet->attributeMap()))
    {
      count++;
    }
    delete fet;
    
    // check if there were errors during evaulating
    if (searchTree->hasError())
      break;
  }
  provider->reset();

  QApplication::restoreOverrideCursor();
  
  return count;
}


void QgsSearchQueryBuilder::on_btnOk_clicked()
{
  // if user hits Ok and there is no query, skip the validation
  if(txtSQL->text().stripWhiteSpace().length() > 0)
  {
    accept();
    return;
  }

  // test the query to see if it will result in a valid layer
  long numRecs = countRecords(txtSQL->text());
  if (numRecs == -1)
  {
    // error shown in countRecords
  }
  else if (numRecs == 0)
  {
    QMessageBox::warning(this, tr("No Records"), tr("The query you specified results in zero records being returned."));
  }
  else
  {
    accept();
  }

}

void QgsSearchQueryBuilder::on_btnEqual_clicked()
{
  txtSQL->insert(" = ");
}

void QgsSearchQueryBuilder::on_btnLessThan_clicked()
{
  txtSQL->insert(" < ");
}

void QgsSearchQueryBuilder::on_btnGreaterThan_clicked()
{
  txtSQL->insert(" > ");
}

void QgsSearchQueryBuilder::on_btnPct_clicked()
{
  txtSQL->insert(" % ");
}

void QgsSearchQueryBuilder::on_btnIn_clicked()
{
  txtSQL->insert(" IN ");
}

void QgsSearchQueryBuilder::on_btnNotIn_clicked()
{
  txtSQL->insert(" NOT IN ");
}

void QgsSearchQueryBuilder::on_btnLike_clicked()
{
  txtSQL->insert(" LIKE ");
}

QString QgsSearchQueryBuilder::searchString()
{
  return txtSQL->text();
}

void QgsSearchQueryBuilder::setSearchString(QString searchString)
{
  txtSQL->setText(searchString);
}

void QgsSearchQueryBuilder::on_lstFields_doubleClicked( Q3ListBoxItem *item )
{
  txtSQL->insert(item->text());
}

void QgsSearchQueryBuilder::on_lstValues_doubleClicked( Q3ListBoxItem *item )
{
  txtSQL->insert(item->text());
}

void QgsSearchQueryBuilder::on_btnLessEqual_clicked()
{
  txtSQL->insert(" <= ");
}

void QgsSearchQueryBuilder::on_btnGreaterEqual_clicked()
{
  txtSQL->insert(" >= ");
}

void QgsSearchQueryBuilder::on_btnNotEqual_clicked()
{
  txtSQL->insert(" != ");
}

void QgsSearchQueryBuilder::on_btnAnd_clicked()
{
  txtSQL->insert(" AND ");
}

void QgsSearchQueryBuilder::on_btnNot_clicked()
{
  txtSQL->insert(" NOT ");
}

void QgsSearchQueryBuilder::on_btnOr_clicked()
{
  txtSQL->insert(" OR ");
}

void QgsSearchQueryBuilder::on_btnClear_clicked()
{
  txtSQL->clear();
}

void QgsSearchQueryBuilder::on_btnILike_clicked()
{
  //txtSQL->insert(" ILIKE ");
  txtSQL->insert(" ~ ");
}

