/***************************************************************************
                          QgsAttributeTableDisplay.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
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

#include "qgsattributetabledisplay.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgssearchquerybuilder.h"
#include "qgssearchstring.h"
#include "qgssearchtreenode.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscontexthelp.h"

#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QSettings>
#include <QToolButton>

QgsAttributeTableDisplay::QgsAttributeTableDisplay(QgsVectorLayer* layer, QgisApp * qgisApp)
: QDialog(0, Qt::Window),
  mLayer(layer),
  mQgisApp(qgisApp)
{
  setupUi(this);
  restorePosition();
  setTheme();
  connect(mRemoveSelectionButton, SIGNAL(clicked()), this, SLOT(removeSelection()));
  connect(mSelectedToTopButton, SIGNAL(clicked()), this, SLOT(selectedToTop()));
  connect(mInvertSelectionButton, SIGNAL(clicked()), this, SLOT(invertSelection()));
  connect(mCopySelectedRowsButton, SIGNAL(clicked()), this, SLOT(copySelectedRowsToClipboard()));
  connect(mZoomMapToSelectedRowsButton, SIGNAL(clicked()), this, SLOT(zoomMapToSelectedRows()));
  connect(mAddAttributeButton, SIGNAL(clicked()), this, SLOT(addAttribute()));
  connect(mDeleteAttributeButton, SIGNAL(clicked()), this, SLOT(deleteAttributes()));
  connect(btnStartEditing, SIGNAL(clicked()), this, SLOT(startEditing()));
  connect(btnStopEditing, SIGNAL(clicked()), this, SLOT(stopEditing()));
  connect(mSearchButton, SIGNAL(clicked()), this, SLOT(search()));
  connect(mSearchShowResults, SIGNAL(activated(int)), this, SLOT(searchShowResultsChanged(int)));
  connect(btnAdvancedSearch, SIGNAL(clicked()), this, SLOT(advancedSearch()));
  connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
  connect(tblAttributes, SIGNAL(featureAttributeChanged(int,int)), this, SLOT(changeFeatureAttribute(int,int)));
  
  //disable those buttons until start editing has been pressed and provider supports it
  mAddAttributeButton->setEnabled(false);
  mDeleteAttributeButton->setEnabled(false);

  btnStopEditing->setEnabled(false);
  int cap=layer->getDataProvider()->capabilities();
  if((cap&QgsVectorDataProvider::ChangeAttributeValues)
      ||(cap&QgsVectorDataProvider::AddAttributes)
      ||(cap&QgsVectorDataProvider::DeleteAttributes))
  {
    btnStartEditing->setEnabled(true);
  }
  else
  {
    btnStartEditing->setEnabled(false);
  }

  // fill in mSearchColumns with available columns
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  if (provider)
  {
    const QgsFieldMap& xfields = provider->fields();
    QgsFieldMap::const_iterator fldIt;
    for (fldIt = xfields.constBegin(); fldIt != xfields.constEnd(); ++fldIt)
    {
      mSearchColumns->insertItem(fldIt->name());
    }
  }
  
  // TODO: create better labels
  mSearchShowResults->insertItem(tr("select"));
  mSearchShowResults->insertItem(tr("select and bring to top"));
  mSearchShowResults->insertItem(tr("show only matching"));
}

QgsAttributeTableDisplay::~QgsAttributeTableDisplay()
{
}
QgsAttributeTable *QgsAttributeTableDisplay::table()
{
  return tblAttributes;
}
void QgsAttributeTableDisplay::setTheme()
{
  QString myIconPath = QgsApplication::themePath();
  mAddAttributeButton->setPixmap(QPixmap(myIconPath+"/mActionNewAttribute.png"));
  mRemoveSelectionButton->setPixmap(QPixmap(myIconPath+"/mActionUnselectAttributes.png"));
  mSelectedToTopButton->setPixmap(QPixmap(myIconPath+"/mActionSelectedToTop.png"));
  mInvertSelectionButton->setPixmap(QPixmap(myIconPath+"/mActionInvertSelection.png"));
  mCopySelectedRowsButton->setPixmap(QPixmap(myIconPath+"/mActionCopySelected.png"));
  mZoomMapToSelectedRowsButton->setPixmap(QPixmap(myIconPath+"/mActionZoomToSelected.png"));
  mAddAttributeButton->setPixmap(QPixmap(myIconPath+"/mActionNewAttribute.png"));
  mDeleteAttributeButton->setPixmap(QPixmap(myIconPath+"/mActionDeleteAttribute.png"));
}

void QgsAttributeTableDisplay::setTitle(QString title)
{
  setCaption(title);
}

void QgsAttributeTableDisplay::deleteAttributes()
{
  QgsDelAttrDialog dialog(table()->horizontalHeader());
  if(dialog.exec()==QDialog::Accepted)
  {
    const std::list<QString>* attlist=dialog.selectedAttributes();
    for(std::list<QString>::const_iterator iter=attlist->begin();iter!=attlist->end();++iter)
    {
      table()->deleteAttribute(*iter);
    }
  }
}

void QgsAttributeTableDisplay::addAttribute()
{
  QgsAddAttrDialog dialog(mLayer->getDataProvider(), this);
  if(dialog.exec()==QDialog::Accepted)
  {
    if(!table()->addAttribute(dialog.name(),dialog.type()))
    {
      QMessageBox::information(this,tr("Name conflict"),tr("The attribute could not be inserted. The name already exists in the table."));
    }
  }
}

void QgsAttributeTableDisplay::startEditing()
{
  QgsVectorDataProvider* provider=mLayer->getDataProvider();
  bool editing=false; 

  if(provider)
  {
    if(provider->capabilities()&QgsVectorDataProvider::AddAttributes)
    {
      mAddAttributeButton->setEnabled(true);
      editing=true;
    }
    if(provider->capabilities()&QgsVectorDataProvider::DeleteAttributes)
    {
      
      mDeleteAttributeButton->setEnabled(true);
      editing=true;
    }
    if(provider->capabilities()&QgsVectorDataProvider::ChangeAttributeValues)
    {
      table()->setReadOnly(false);
      table()->setColumnReadOnly(0,true);//id column is not editable
      editing=true;
    }
    if(editing)
    {
      btnStartEditing->setEnabled(false);
      btnStopEditing->setEnabled(true);
      btnClose->setEnabled(false);
      //make the dialog modal when in editable
      //otherwise map editing and table editing
      //may disturb each other
      hide();
      setModal(true);
      show();
    }
  }
}

void QgsAttributeTableDisplay::stopEditing()
{
  if(table()->edited())
  {
    //commit or roll back?
    QMessageBox::StandardButton commit=QMessageBox::information(this,tr("Stop editing"),
      tr("Do you want to save the changes?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if(commit==QMessageBox::Save)
    {
      if(!table()->commitChanges(mLayer))
      {
        QMessageBox::information(this,tr("Error"),tr("Could not commit changes - changes are still pending"));
        return;
      }
    }
    else if(commit == QMessageBox::Discard)
    {
      table()->rollBack(mLayer);
    }
    else //cancel
    {
      return;
    }
  }
  btnStartEditing->setEnabled(true);
  btnStopEditing->setEnabled(false);
  btnClose->setEnabled(true);
  mAddAttributeButton->setEnabled(false);
  mDeleteAttributeButton->setEnabled(false);
  table()->setReadOnly(true);
  //make this dialog modeless again
  hide();
  setModal(false);
  show();
}

void QgsAttributeTableDisplay::selectedToTop()
{
  table()->bringSelectedToTop();
}

void QgsAttributeTableDisplay::invertSelection()
{
  if(mLayer)
  {
    QApplication::setOverrideCursor(Qt::waitCursor);
    mLayer->invertSelection();
    QApplication::restoreOverrideCursor();
  }
}

void QgsAttributeTableDisplay::removeSelection()
{
    table()->clearSelection();
    mLayer->triggerRepaint();
}

void QgsAttributeTableDisplay::copySelectedRowsToClipboard()
{
  // Deprecated
  // table()->copySelectedRows();

  // Use the Application's copy method instead
  mQgisApp->editCopy(mLayer);
}

void QgsAttributeTableDisplay::zoomMapToSelectedRows()
{
  mQgisApp->zoomToSelected();
}

void QgsAttributeTableDisplay::search()
{
  // if selected field is numeric, numeric comparison will be used
  // else attributes containing entered text will be matched

  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  int item = mSearchColumns->currentItem();
  QVariant::Type type = provider->fields()[item].type();
  bool numeric = (type == QVariant::Int || type == QVariant::Double);
  
  QString str;
  str = mSearchColumns->currentText();
  if (numeric)
    str += " = '";
  else
    str += " ~ '";
  str += mSearchText->text();
  str += "'";

  doSearch(str);
}


void QgsAttributeTableDisplay::advancedSearch()
{
  QgsSearchQueryBuilder dlg(mLayer, this);
  dlg.setSearchString(mSearchString);
  if (dlg.exec())
  {
    doSearch(dlg.searchString());
  }
}


void QgsAttributeTableDisplay::searchShowResultsChanged(int item)
{
  QApplication::setOverrideCursor(Qt::waitCursor);

  if (item == 2) // show only matching
  {
    table()->showRowsWithId(mSearchIds);
  }
  else
  {    
    // make sure that all rows are shown
    table()->showAllRows();
    
    // select matching
    mLayer->setSelectedFeatures(mSearchIds);
  
    if (item == 1) // select matching and bring to top
      table()->bringSelectedToTop();
  }

  QApplication::restoreOverrideCursor();
}


void QgsAttributeTableDisplay::doSearch(const QString& searchString)
{
  mSearchString = searchString;

  // parse search string (and build parsed tree)
  QgsSearchString search;
  if (!search.setString(searchString))
  {
    QMessageBox::critical(this, tr("Search string parsing error"), search.parserErrorMsg());
    return;
  }
  QgsSearchTreeNode* searchTree = search.tree();
  if (searchTree == NULL)
  {
    QMessageBox::information(this, tr("Search results"), tr("You've supplied an empty search string."));
    return;
  }

  QgsDebugMsg("Search by attribute: " + searchString + " parsed as: " + search.tree()->makeSearchString());

  QApplication::setOverrideCursor(Qt::waitCursor);

  // TODO: need optimized getNextFeature which won't extract geometry
  // or search by traversing table ... which one is quicker?
  QgsFeature fet;
  QgsVectorDataProvider* provider = mLayer->getDataProvider();
  mSearchIds.clear();
  const QgsFieldMap& fields = provider->fields();
  QgsAttributeList all = provider->allAttributesList();
  
  provider->select(all, QgsRect(), false);

  while (provider->getNextFeature(fet))
  {
    if (searchTree->checkAgainst(fields, fet.attributeMap()))
    {
      mSearchIds.insert(fet.featureId());
    }
    
    // check if there were errors during evaulating
    if (searchTree->hasError())
      break;
  }

  QApplication::restoreOverrideCursor();

  if (searchTree->hasError())
  {
    QMessageBox::critical(this, tr("Error during search"), searchTree->errorMsg());
    return;
  }

  // update table
  searchShowResultsChanged(mSearchShowResults->currentItem());
   
  QString str;
  if (mSearchIds.size())
    str.sprintf(tr("Found %d matching features.","", mSearchIds.size()), mSearchIds.size());
  else
    str = tr("No matching features found.");
  QMessageBox::information(this, tr("Search results"), str);

}

void QgsAttributeTableDisplay::closeEvent(QCloseEvent* ev)
{
  saveWindowLocation();
  ev->ignore();
  emit deleted();
  delete this;
}

void QgsAttributeTableDisplay::restorePosition()
{
  QSettings settings;
  restoreGeometry(settings.value("/Windows/AttributeTable/geometry").toByteArray());
}

void QgsAttributeTableDisplay::saveWindowLocation()
{
  QSettings settings;
  settings.setValue("/Windows/AttributeTable/geometry", saveGeometry());
} 

void QgsAttributeTableDisplay::on_btnHelp_clicked()
{
  showHelp();
}
void QgsAttributeTableDisplay::showHelp()
{
  QgsContextHelp::run(context_id);
}

void QgsAttributeTableDisplay::changeFeatureAttribute(int row, int column)
{
  QgsFeatureList &flist = mLayer->addedFeatures();

  int id = table()->text(row,0).toInt();

  int i;
  for(i=0; i<flist.size() && flist[i].featureId()!=id; i++)
    ;

  if(i==flist.size())
    return;

  flist[i].changeAttribute(column-1, table()->text(row,column));
}
