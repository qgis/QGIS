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

#include "qgsuvaldialog.h"
#include "qgsdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgsuniquevalrenderer.h"
#include "qgssisydialog.h"
#include "qgsrenderitem.h"
#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qcombobox.h>

QgsUValDialog::QgsUValDialog(QgsVectorLayer* vl): QgsUValDialogBase(), mVectorLayer(vl)
{

    //find out the fields of mVectorLayer
    QgsDataProvider *provider;
    if (provider = mVectorLayer->getDataProvider())
    {
	std::vector < QgsField > &fields = provider->fields();
	QString str;
	
	for (std::vector < QgsField >::iterator it = fields.begin(); it != fields.end(); ++it)
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
    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeSymbologyDialog()));
}

QgsUValDialog::~QgsUValDialog()
{
    for(std::map<QString,QgsSiSyDialog*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }
}

void QgsUValDialog::apply()
{
    QgsUniqueValRenderer *renderer = dynamic_cast < QgsUniqueValRenderer * >(mVectorLayer->renderer());
    //go through mValues and add the entries to the renderer
    if(renderer)
    {
	renderer->clearValues();
	for(std::map<QString,QgsSiSyDialog*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    QgsSiSyDialog* sdialog=it->second;
	    QgsRenderItem* ritem=new QgsRenderItem();
	    QgsSymbol* symbol=new QgsSymbol();
	    QPen pen(sdialog->getOutlineColor(),sdialog->getOutlineWidth(),sdialog->getOutlineStyle());
	    QBrush brush(sdialog->getFillColor(), sdialog->getFillStyle());
	    symbol->setPen(pen);
	    symbol->setBrush(brush);
	    ritem->setSymbol(symbol);
	    renderer->insertValue(it->first,ritem);
	}
	renderer->setClassificationField(mClassificationComboBox->currentItem());
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("Warning, typecast failed in qgsuvaldialog.cpp, l. 61"); 
#endif
    }
    mVectorLayer->triggerRepaint();
}

void QgsUValDialog::changeClassificationAttribute(int nr)
{
#ifdef QGISDEBUG
    qWarning("in changeClassificationAttribute, nr is: "+QString::number(nr));
#endif
    mValues.clear();

    //delete the old dialogs
    for(std::map<QString,QgsSiSyDialog*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }

    QgsDataProvider *provider = mVectorLayer->getDataProvider();
    if (provider)
    {
	QString value;
	std::list<int> attlist;
	attlist.push_back(nr);
	std::vector < QgsFeatureAttribute > vec;
	QgsSiSyDialog* dialog;

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
		dialog=new QgsSiSyDialog(mVectorLayer);
		mValues.insert(std::make_pair(value,dialog));
		//mClassBreakBox->insertItem(value);
		if(mSymbolWidgetStack->addWidget(dialog)==-1)
		{
		    //failed
#ifdef QGISDEBUG
		    qWarning("adding widget to the stack failed");
#endif
		}
	    }
	}
	
	//set symbology for all QgsSiSyDialogs
	QColor thecolor;
	double number=0;
	double frac;

	for(std::map<QString,QgsSiSyDialog*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    ++number;
	    //color range from blue to red
	    frac=number/mValues.size();
	    thecolor.setRgb(int(255*frac),0,int(255-(255*frac)));
	    mClassBreakBox->insertItem(it->first);
	    QgsSiSyDialog* dia=it->second;
	    if(mVectorLayer->vectorType() == QGis::Line)
	    {
		dia->setOutlineColor(thecolor);
		dia->setOutlineStyle(Qt::SolidLine);
		QColor black(0,0,0);
		dia->setFillColor(black);
	    }
	    else
	    {
		dia->setFillColor(thecolor);
		dia->setFillStyle(Qt::SolidPattern);
		QColor black(0,0,0);
		dia->setOutlineColor(black);
		dia->setOutlineStyle(Qt::SolidLine);		  
	    }
	}
	
    }
    
}

void QgsUValDialog::changeSymbologyDialog()
{
    QListBoxItem* item=mClassBreakBox->selectedItem();
    QString value=item->text();
    std::map<QString,QgsSiSyDialog*>::iterator it=mValues.find(value);
    if(it!=mValues.end())
    {
	mSymbolWidgetStack->raiseWidget(it->second);
	QgsSiSyDialog* dialog=it->second;
	dialog->show();
    }
    else
    {
	//no entry found
    }
}
