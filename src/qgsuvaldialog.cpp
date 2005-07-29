/***************************************************************************
                         qgsuvaldialog.cpp  -  description
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
#include "qgsuvaldialog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgsuniquevalrenderer.h"
#include "qgssisydialog.h"
#include "qgssymbol.h"
#include "qgsrenderitem.h"
#include "qgsuniquevalrenderer.h"
#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpainter.h>
#include <list>

QgsUValDialog::QgsUValDialog(QgsVectorLayer* vl): QgsUValDialogBase(), mVectorLayer(vl), sydialog(vl)
{
    setSizeGripEnabled(true); 

    //find out the fields of mVectorLayer
    QgsVectorDataProvider *provider;
    if (provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider()))
    {
	std::vector < QgsField > const & fields = provider->fields();
	QString str;
	
	for (std::vector < QgsField >::const_iterator it = fields.begin(); 
             it != fields.end(); 
             ++it)
        {
	    str = (*it).name();
	    str = str.left(1).upper() + str.right(str.length() - 1);  //make the first letter uppercase
	    mClassificationComboBox->insertItem(str);
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsUValDialog::QgsUValDialog");
	return;
    }

    QObject::connect(mClassificationComboBox, SIGNAL(activated(int)), this, SLOT(changeClassificationAttribute(int)));
    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
    QObject::connect(&sydialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
    mSymbolWidgetStack->addWidget(&sydialog);
    mSymbolWidgetStack->raiseWidget(&sydialog);

    const QgsUniqueValRenderer* renderer = dynamic_cast < const QgsUniqueValRenderer * >(mVectorLayer->renderer());
    
    if (renderer)
    {
	mClassBreakBox->clear();
	std::list<int>::iterator iter=renderer->classificationAttributes().begin();
	int classattr=*iter;
	mClassificationComboBox->setCurrentItem(classattr);
	
	if(renderer->symbols().size()>0)
	{
	    changeClassificationAttribute(classattr);
	}

	const std::list<QgsSymbol*> list = renderer->symbols();
	//fill the items of the renderer into mValues
	for(std::list<QgsSymbol*>::const_iterator iter=list.begin();iter!=list.end();++iter)
	{
	    QgsSymbol* symbol=(*iter);
	    QString symbolvalue=symbol->lowerValue();
	    QgsSymbol* sym=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
	    sym->setPen(symbol->pen());
	    sym->setBrush(symbol->brush());
	    sym->setNamedPointSymbol(symbol->pointSymbolName());
	    sym->setPointSize(symbol->pointSize());
	    
	    mValues.insert(std::make_pair(symbolvalue,sym));
	    mClassBreakBox->insertItem(symbolvalue);
	}
    }
    else
    {
	changeClassificationAttribute(0);	
    }
    mClassBreakBox->setCurrentItem(0);
}

QgsUValDialog::~QgsUValDialog()
{
    std::map<QString, QgsSymbol *>::iterator myValueIterator = mValues.begin();
    while ( myValueIterator != mValues.end() )
    {
        delete myValueIterator->second;
        
        mValues.erase( myValueIterator );
        
        myValueIterator = mValues.begin(); // since iterator invalidated due to
                                        // erase(), reset to new first element
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUValDialog::apply()
{
    QgsUniqueValRenderer *renderer = new QgsUniqueValRenderer(mVectorLayer->vectorType());

    //go through mValues and add the entries to the renderer
    for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	QgsSymbol* symbol=it->second;
#ifdef QGISDEBUG
	qWarning("apply: lower value is "+symbol->lowerValue());
#endif
	QgsSymbol* newsymbol=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
	newsymbol->setPen(symbol->pen());
	newsymbol->setBrush(symbol->brush());
	newsymbol->setNamedPointSymbol(symbol->pointSymbolName());
	newsymbol->setPointSize(symbol->pointSize());
	renderer->insertValue(it->first,newsymbol);
    }

    renderer->setClassificationField(mClassificationComboBox->currentItem());
    mVectorLayer->setRenderer(renderer);
    mVectorLayer->refreshLegend();
}

void QgsUValDialog::changeClassificationAttribute(int nr)
{
    //delete old entries
    for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }
    mValues.clear();
    
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
    if (provider)
    {
	QString value;
	std::list<int> attlist;
	attlist.push_back(nr);
	std::vector < QgsFeatureAttribute > vec;
	QgsSymbol* symbol;
	QgsRenderItem* ritemptr;

	provider->reset();
	QgsFeature* f;

	//go through all the features and insert their value into the map and into mClassBreakBox
	mClassBreakBox->clear();
	while((f=provider->getNextFeature(attlist)))
	{
	    vec = f->attributeMap();
	    value=vec[0].fieldValue();
	   
	    if(mValues.find(value)==mValues.end())
	    {
		symbol=new QgsSymbol(mVectorLayer->vectorType(), value);
		mValues.insert(std::make_pair(value,symbol));
	    }
	    delete f;
	}

	//set symbology for all QgsSiSyDialogs
	QColor thecolor;
	double number=0;
	double frac;

	for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    ++number;
	    //color range from blue to red
	    frac=number/mValues.size();
	    thecolor.setRgb(int(255*frac),0,int(255-(255*frac)));
	    mClassBreakBox->insertItem(it->first);
	    QgsSymbol* sym=it->second;
	    QPen pen;
	    QBrush brush;
	    if(mVectorLayer->vectorType() == QGis::Line)
	    {
		pen.setColor(thecolor);
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(1);
	    }
	    else
	    {
		brush.setColor(thecolor);
		brush.setStyle(Qt::SolidPattern);
		pen.setColor(Qt::black);
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(1);
	    }
	    sym->setPen(pen);
	    sym->setBrush(brush);
	}
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUValDialog::changeCurrentValue()
{
    sydialog.blockSignals(true);//block signal to prevent sydialog from changing the current QgsRenderItem
    QListBoxItem* item=mClassBreakBox->selectedItem();
    QString value=item->text();
    std::map<QString,QgsSymbol*>::iterator it=mValues.find(value);
    if(it!=mValues.end())
    {
	sydialog.set( it->second);
	sydialog.setLabel(it->second->label());
    }
    else
    {
	//no entry found
    }
    sydialog.blockSignals(false);
}

void QgsUValDialog::applySymbologyChanges()
{
  QListBoxItem* item=mClassBreakBox->selectedItem();
  QString value=item->text();
  std::map<QString,QgsSymbol*>::iterator it=mValues.find(value);
  if(it!=mValues.end())
  {
      it->second->setLabel(sydialog.label());
      it->second->setLowerValue(value);
      sydialog.apply( it->second );
  }
}
