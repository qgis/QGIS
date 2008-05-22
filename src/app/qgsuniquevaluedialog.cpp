/***************************************************************************
                         qgsuniquevaluedialog.cpp  -  description
                             -------------------
    begin                : July 2004
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

#include "qgsuniquevaluedialog.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgsuniquevaluerenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include "qgslogger.h"

QgsUniqueValueDialog::QgsUniqueValueDialog(QgsVectorLayer* vl): QDialog(), mVectorLayer(vl), sydialog(vl, true)
{
  setupUi(this);
  setOrientation(Qt::Vertical);
  //find out the fields of mVectorLayer
  QgsVectorDataProvider *provider;
  if ((provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider())))
  {
    const QgsFieldMap & fields = provider->fields();
    QString str;

    for (QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it)
    {
      str = (*it).name();
      mClassificationComboBox->insertItem(str);
    }
  }
  else
  {
    qWarning("Warning, data provider is null in QgsUniqueValueDialog::QgsUniqueValueDialog");
    return;
  }

  mClassListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  mClassListWidget->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::AnyKeyPressed);
  mClassListWidget->setSortingEnabled(true);

  const QgsUniqueValueRenderer* renderer = dynamic_cast < const QgsUniqueValueRenderer * >(mVectorLayer->renderer());

  if (renderer)
  {
    mClassListWidget->clear();

    // XXX - mloskot - fix for Ticket #31 (bug)
    //QgsAttributeList attributes = renderer->classificationAttributes();
    //QgsAttributeList::iterator iter = attributes.begin();
    //int classattr = *iter;
    //QString field = provider->fields()[ classattr ].name();
    QString field = provider->fields()[ renderer->classificationField() ].name();
    mClassificationComboBox->setCurrentItem( mClassificationComboBox->findText(field) );

    const QList<QgsSymbol*> list = renderer->symbols();
    //fill the items of the renderer into mValues
    for(QList<QgsSymbol*>::const_iterator iter=list.begin(); iter!=list.end(); ++iter)
    {
      QgsSymbol* symbol=(*iter);
      QString symbolvalue=symbol->lowerValue();
      QgsSymbol* sym=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
      sym->setPen(symbol->pen());
      sym->setCustomTexture(symbol->customTexture());
      sym->setBrush(symbol->brush());
      sym->setNamedPointSymbol(symbol->pointSymbolName());
      sym->setPointSize(symbol->pointSize());
      sym->setScaleClassificationField(symbol->scaleClassificationField());
      sym->setRotationClassificationField(symbol->rotationClassificationField());
      mValues.insert(symbolvalue, sym);

      QListWidgetItem *item = new QListWidgetItem(symbolvalue);
      mClassListWidget->addItem(item);
      item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
      item->setData( Qt::UserRole, symbol->lowerValue() );
      item->setToolTip(symbol->label());
    }
  }

  mDeletePushButton->setEnabled(false);

  QObject::connect(mClassifyButton, SIGNAL(clicked()), this, SLOT(changeClassificationAttribute()));
  QObject::connect(mAddButton, SIGNAL(clicked()), this, SLOT(addClass()));
  QObject::connect(mDeletePushButton, SIGNAL(clicked()), this, SLOT(deleteSelectedClasses()));
  QObject::connect(mRandomizeColors, SIGNAL(clicked()), this, SLOT(randomizeColors()));
  QObject::connect(mResetColors, SIGNAL(clicked()), this, SLOT(resetColors()));
  QObject::connect(mClassListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
  QObject::connect(mClassListWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(itemChanged(QListWidgetItem *)));
  QObject::connect(&sydialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
  mSymbolWidgetStack->addWidget(&sydialog);
  mSymbolWidgetStack->setCurrentWidget(&sydialog);
}

QgsUniqueValueDialog::~QgsUniqueValueDialog()
{
  QgsDebugMsg("called.");
  QMap<QString, QgsSymbol *>::iterator myValueIterator = mValues.begin();
  while ( myValueIterator != mValues.end() )
  {
    delete myValueIterator.value();

    mValues.erase( myValueIterator );

    myValueIterator = mValues.begin(); // since iterator invalidated due to
    // erase(), reset to new first element
  }
  mClassListWidget->setCurrentItem(0);
}

void QgsUniqueValueDialog::apply()
{
  QgsDebugMsg("called.");
  QgsUniqueValueRenderer *renderer = new QgsUniqueValueRenderer(mVectorLayer->vectorType());

  //go through mValues and add the entries to the renderer
  for(QMap<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
  {
    QgsSymbol* symbol=it.value();
    QgsSymbol* newsymbol=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
    newsymbol->setPen(symbol->pen());
    newsymbol->setCustomTexture(symbol->customTexture());
    newsymbol->setBrush(symbol->brush());
    newsymbol->setNamedPointSymbol(symbol->pointSymbolName());
    newsymbol->setPointSize(symbol->pointSize());
    newsymbol->setScaleClassificationField(symbol->scaleClassificationField());
    newsymbol->setRotationClassificationField(symbol->rotationClassificationField());
    renderer->insertValue(it.key(), newsymbol);
  }
  renderer->updateSymbolAttributes();

  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
  if (provider)
  {
    int fieldIndex = provider->indexFromFieldName(mClassificationComboBox->currentText());
    if(fieldIndex != -1)
    {
      renderer->setClassificationField(fieldIndex);
      mVectorLayer->setRenderer(renderer);
      return;
    }
  }

  delete renderer; //something went wrong
}


QColor QgsUniqueValueDialog::randomColor()
{
  QColor thecolor;

  //insert a random color
  int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
  int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
  int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
  thecolor.setRgb(red, green, blue);

  return thecolor;
}

void QgsUniqueValueDialog::setSymbolColor(QgsSymbol *symbol, QColor thecolor)
{
  QPen pen;
  QBrush brush;
  if(mVectorLayer->vectorType() == QGis::Line)
  {
    pen.setColor(thecolor);
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(0.4);
  }
  else
  {
    brush.setColor(thecolor);
    brush.setStyle(Qt::SolidPattern);
    pen.setColor(Qt::black);
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(0.4);
  }
  symbol->setPen(pen);
  symbol->setBrush(brush);
}

void QgsUniqueValueDialog::addClass(QString value)
{
  QgsDebugMsg("called.");
  if( value.isNull() || mValues.contains(value) )
  {
    int i;
    for(i=0; mValues.contains(value+QString::number(i)); i++)
      ;
    value += QString::number(i);
  }

  QgsSymbol *symbol=new QgsSymbol(mVectorLayer->vectorType(), value);
  mValues.insert(value, symbol);

  QListWidgetItem *item = new QListWidgetItem(value);
  item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
  item->setData( Qt::UserRole, value );
  item->setToolTip(symbol->label());
  mClassListWidget->addItem(item);

  setSymbolColor(symbol, randomColor() );
}

void QgsUniqueValueDialog::randomizeColors()
{
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  if(selection.size()>0) {
    for(int i=0; i<selection.size(); i++)
    {
      QListWidgetItem *item=selection[i];
      if(!item)
        continue;

      if( !mValues.contains( item->text() ) )
        continue;

      setSymbolColor( mValues[ item->text() ], randomColor() );
    }
  }
  else
  {
    for(QMap<QString, QgsSymbol *>::iterator it = mValues.begin(); it!=mValues.end(); it++)
    {
      setSymbolColor( it.value(), randomColor() );
    }
  }

  selectionChanged();
}

void QgsUniqueValueDialog::resetColors()
{
  QColor white;
  white.setRgb(255.0, 255.0, 255.0);

  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  if(selection.size()>0) {
    for(int i=0; i<selection.size(); i++)
    {
      QListWidgetItem *item=selection[i];
      if(!item)
        continue;

      if( !mValues.contains( item->text() ) )
        continue;

      setSymbolColor( mValues[ item->text() ], white);
    }
  }
  else
  {
    for(QMap<QString, QgsSymbol *>::iterator it = mValues.begin(); it!=mValues.end(); it++)
    {
      setSymbolColor( it.value(), white);
    }
  }

  selectionChanged();
}


void QgsUniqueValueDialog::changeClassificationAttribute()
{
  QgsDebugMsg("called.");
  QString attributeName = mClassificationComboBox->currentText();

  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
  if (provider)
  {
    QString value;
    QgsAttributeList attlist;

    int nr = provider->indexFromFieldName(attributeName);
    if(nr == -1)
    {
      return;
    }
    attlist.append(nr);

    provider->select(attlist, QgsRect(), false);
    QgsFeature feat;

    //go through all the features and insert their value into the map and into mClassListWidget
    while(provider->getNextFeature(feat)) 
    {
      const QgsAttributeMap& attrs = feat.attributeMap();
      value = attrs[nr].toString();

      if( mValues.contains(value) )
        continue;

      addClass(value);
    }
  }
  mClassListWidget->setCurrentRow(0);
}

void QgsUniqueValueDialog::itemChanged( QListWidgetItem *item )
{
  QString oldValue = item->data( Qt::UserRole ).toString();
  QString newValue = item->text();

  if(oldValue==newValue)
    return;
  
  if( !mValues.contains(newValue) )
  {
    QgsSymbol *sy = mValues[oldValue];
    mValues.erase(oldValue);
    mValues.insert(newValue, sy);
    sy->setLowerValue(newValue);
    item->setData( Qt::UserRole, newValue );
  }
  else
    item->setText(oldValue);
}


void QgsUniqueValueDialog::selectionChanged()
{
  QgsDebugMsg("called.");
  sydialog.blockSignals(true);//block signal to prevent sydialog from changing the current QgsRenderItem
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();

  if(selection.size()==0)
  {
    mDeletePushButton->setEnabled(false);
    sydialog.unset();
  }
  else
  {
    mDeletePushButton->setEnabled(true);

    if(selection.size()==1)
    {
      QListWidgetItem *item=selection[0];
      if(!item)
        return;

      if( !mValues.contains( item->text() ) )
        return;

      QgsSymbol *symbol = mValues[ item->text() ];
      sydialog.set( symbol );
      sydialog.setLabel( symbol->label() );
    }
    else if(selection.size()>1)
    {
      if( !mValues.contains( selection[0]->text() ) )
        return;

      sydialog.set( mValues[ selection[0]->text() ] );

      for(int i=1; i<selection.size(); i++)
      {
        if( !mValues.contains( selection[i]->text() ) )
          continue;

        sydialog.updateSet( mValues[ selection[i]->text() ] );
      }
    }
  }
  sydialog.blockSignals(false);
}

void QgsUniqueValueDialog::deleteSelectedClasses()
{
  QgsDebugMsg("called.");
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  for(int i=0; i<selection.size(); i++) 
  {
    QListWidgetItem* currentItem = selection[i];
    if(!currentItem)
      continue;
 
    mValues.erase( currentItem->text() );

    mClassListWidget->removeItemWidget(currentItem);
    delete currentItem;
  }

  QgsDebugMsg( QString("numRows: %1").arg( mClassListWidget->count()) );
}

void QgsUniqueValueDialog::applySymbologyChanges()
{
  QgsDebugMsg("called.");
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  for(int i=0; i<selection.size(); i++)
  {
    QListWidgetItem* item=selection[i];
    if(!item)
    {
      QgsDebugMsg( QString("selected item %1 not found").arg(i) );
      continue;
    }

    QString value=item->text();
    if( !mValues.contains( value ) ) {
      QgsDebugMsg( QString("value %1 not found").arg(value) );
      continue;
    }

    QgsSymbol *symbol = mValues[ value ];
    symbol->setLabel(sydialog.label());
    symbol->setLowerValue(value);
    sydialog.apply(symbol);

    item->setToolTip(sydialog.label());
    item->setData( Qt::UserRole, value);
  }
}
