/***************************************************************************
                         qgsgrasydialog.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#include "qgsgrasydialog.h"
#include "qspinbox.h"
#include "qpushbutton.h"
#include <qcombobox.h>
#include <qlistbox.h>
#include "qgssymbologyutils.h"
#include "qgsrangerenderitem.h"
#include "qlineedit.h"
#include "qgsludialog.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgsvectorlayer.h"
#include "qgslegenditem.h"
#include "qgsvectordataprovider.h"
#include "qgsfield.h"
#include "qscrollview.h"
#include <qlayout.h>
#include <qwidgetstack.h>

QgsGraSyDialog::QgsGraSyDialog(QgsVectorLayer * layer):QgsGraSyDialogBase(), mVectorLayer(layer), sydialog(layer)
{


#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyDialog");
#endif

    setOrientation(Qt::Vertical);
    setSizeGripEnabled(true);

    //find out the numerical fields of mVectorLayer
    QgsVectorDataProvider *provider;
    if (provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider()))
    {
	std::vector < QgsField > const & fields = provider->fields();
	int fieldnumber = 0;
	QString str;
	
	for (std::vector < QgsField >::const_iterator it = fields.begin(); 
             it != fields.end(); 
             ++it)
        {
	    QString type = (*it).type();
	    if (type != "String" && type != "varchar" && type != "geometry")
            {
		str = (*it).name();
		str = str.left(1).upper() + str.right(str.length() - 1);  //make the first letter uppercase
		classificationComboBox->insertItem(str);
		mFieldMap.insert(std::make_pair(str, fieldnumber));
            }
	    fieldnumber++;
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsGraSyDialog::QgsGraSyDialog(...)");
	return;
    }

    modeComboBox->insertItem("Empty");
    modeComboBox->insertItem("Equal Interval");
    
    //restore the correct settings
    const QgsGraduatedSymRenderer* renderer = dynamic_cast < const QgsGraduatedSymRenderer * >(layer->renderer());
    
    if (renderer)
    {
	std::list < QgsSymbol * >list = renderer->symbols();
	
	//display the classification field
	QString classfield="";
	for(std::map<QString,int>::iterator it=mFieldMap.begin();it!=mFieldMap.end();++it)
	{
	    if(it->second==renderer->classificationField())
	    {
		classfield=it->first;
		break;
	    }
	}
	classificationComboBox->setCurrentText(classfield);

	QGis::VectorType m_type = mVectorLayer->vectorType();
	numberofclassesspinbox->setValue(list.size());
	//fill the items of the renderer into mValues
	for(std::list<QgsSymbol*>::iterator it=list.begin();it!=list.end();++it)
	{
	    //todo: make an assignment operator and a copy constructor for QgsSymbol
		QString classbreak=(*it)->lowerValue()+" - "+(*it)->upperValue();
		QgsSymbol* sym=new QgsSymbol(mVectorLayer->vectorType(), (*it)->lowerValue(), (*it)->upperValue(), (*it)->label());
		sym->setPen((*it)->pen());
		sym->setBrush((*it)->brush());
		sym->setNamedPointSymbol((*it)->pointSymbolName());
		sym->setPointSize((*it)->pointSize());
		mEntries.insert(std::make_pair(classbreak,sym));
		mClassBreakBox->insertItem(classbreak);
	}
	
    }
    
    //do the necessary signal/slot connections
    QObject::connect(numberofclassesspinbox, SIGNAL(valueChanged(int)), this, SLOT(adjustClassification()));
    QObject::connect(classificationComboBox, SIGNAL(activated(int)), this, SLOT(adjustClassification()));
    QObject::connect(modeComboBox, SIGNAL(activated(int)), this, SLOT(adjustClassification()));
    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
    QObject::connect(&sydialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
    QObject::connect(mClassBreakBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(changeClass(QListBoxItem*)));

    mSymbolWidgetStack->addWidget(&sydialog);
    mSymbolWidgetStack->raiseWidget(&sydialog); 

    mClassBreakBox->setCurrentItem(0);
}

QgsGraSyDialog::QgsGraSyDialog(): QgsGraSyDialogBase(), mVectorLayer(0), sydialog(0)
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyDialog");
#endif
}

QgsGraSyDialog::~QgsGraSyDialog()
{
#ifdef QGISDEBUG
    qWarning("destructor QgsGraSyDialog");
#endif
}

void QgsGraSyDialog::adjustNumberOfClasses()
{
    //find out the number of the classification field
    QString fieldstring = classificationComboBox->currentText();
    
    if (fieldstring.isEmpty())    //don't do anything, it there is no classification field
    {
	show();
	return;
    }
    
    std::map < QString, int >::iterator iter = mFieldMap.find(fieldstring);
    int field = iter->second;
}

void QgsGraSyDialog::apply()
{
	if (classificationComboBox->currentText().isEmpty())  //don't do anything, it there is no classification field
        {
	    return;
        }
	
	QgsGraduatedSymRenderer* renderer = new QgsGraduatedSymRenderer(mVectorLayer->vectorType());

	for (int item=0;item<mClassBreakBox->count();++item)
        {
	    QString classbreak=mClassBreakBox->text(item);
	    std::map<QString,QgsSymbol*>::iterator it=mEntries.find(classbreak);
	    if(it==mEntries.end())
	    {
		continue;
	    }
	
	    QString lower_bound=it->second->lowerValue();
	    QString upper_bound=it->second->upperValue();
	    QString label=it->second->label();

	    QgsSymbol* sy = new QgsSymbol(mVectorLayer->vectorType(), lower_bound, upper_bound, label);
	    
	    sy->setColor(it->second->pen().color());
	    sy->setLineStyle(it->second->pen().style());
	    sy->setLineWidth(it->second->pen().width());
	    
	    if (mVectorLayer->vectorType() == QGis::Point)
	    {
		sy->setNamedPointSymbol(it->second->pointSymbolName());
		sy->setPointSize(it->second->pointSize());
	     
	    }
	    
	    if (mVectorLayer->vectorType() != QGis::Line)
            {
		sy->setFillColor(it->second->brush().color());
		sy->setFillStyle(it->second->brush().style());
            }
	    
	    //test, if lower_bound is numeric or not (making a subclass of QString would be the proper solution)
	    bool lbcontainsletter = false;
	    for (uint j = 0; j < lower_bound.length(); j++)
            {
		if (lower_bound.ref(j).isLetter())
                {
		    lbcontainsletter = true;
                }
            }
	    
	    //test, if upper_bound is numeric or not (making a subclass of QString would be the proper solution)
	    bool ubcontainsletter = false;
	    for (uint j = 0; j < upper_bound.length(); j++)
            {
		if (upper_bound.ref(j).isLetter())
                {
		    ubcontainsletter = true;
                }
            }
	    if (lbcontainsletter == false && ubcontainsletter == false && lower_bound.length() > 0 && upper_bound.length() > 0) //only add the item if the value bounds do not contain letters and are not null strings
            {
		renderer->addSymbol(sy);
	    }
	    else
	    {
		delete sy;
	    }
	    //++i;
        }
	
	std::map<QString,int>::iterator iter=mFieldMap.find(classificationComboBox->currentText());
	if(iter!=mFieldMap.end())
	{
	   renderer->setClassificationField(iter->second); 
	}
	mVectorLayer->setRenderer(renderer);
	mVectorLayer->refreshLegend();
}

void QgsGraSyDialog::adjustClassification()
{
    mClassBreakBox->clear();
    QGis::VectorType m_type = mVectorLayer->vectorType();
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
    double minimum, maximum;
    
    //delete all previous entries
    for(std::map<QString, QgsSymbol*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	delete it->second;
    }
    mEntries.clear();

    //find out the number of the classification field
    QString fieldstring = classificationComboBox->currentText();

    if (fieldstring.isEmpty())    //don't do anything, it there is no classification field
    {
	show();
	return;
    }
    
    std::map < QString, int >::iterator iter = mFieldMap.find(fieldstring);
    int field = iter->second;

    if (provider)
    {
	if (modeComboBox->currentText() == "Equal Interval")
	{
	    minimum = provider->minValue(field).toDouble();
	    maximum = provider->maxValue(field).toDouble();
	} 
	else                    //don't waste performance if mMode is QgsGraSyDialog::EMPTY
	{
	    minimum = 0;
	    maximum = 0;
	}
    }
 
    
    QString listboxtext;
    for(int i=0;i<numberofclassesspinbox->value();++i)
    {
	QgsSymbol* symbol = new QgsSymbol(m_type);
	symbol->setLabel("");
	QPen pen;
	QBrush brush;

	if (modeComboBox->currentText() == "Empty")
	{
	    listboxtext="Empty"+QString::number(i+1);
	    mClassBreakBox->insertItem(listboxtext);
	}
	else if(modeComboBox->currentText() == "Equal Interval")
	{
	    double lower=minimum + (maximum - minimum) / numberofclassesspinbox->value() * i;
	    double upper=minimum + (maximum - minimum) / numberofclassesspinbox->value() * (i+1);
	    if(i==0)//make sure all feature attributes are between minimum and maximum value (round off problem)
	      {
		lower-=0.001;
	      }
	    if(i==numberofclassesspinbox->value()-1)
	      {
		upper+=0.001;
	      }
	    symbol->setLowerValue(QString::number(lower,'f',3));
	    symbol->setUpperValue(QString::number(upper,'f',3));
	    listboxtext=QString::number(lower,'f',3)+" - " +QString::number(upper,'f',3);
	    mClassBreakBox->insertItem(listboxtext);
	}
	    //set default symbology

	    //apply a nice color range from blue to red as default
	    if (i == 0)
	    {
		if (m_type == QGis::Line)
		{
		    pen.setColor(QColor(0, 0, 255));
		} 
		else                //point or polygon
		{
		    brush.setColor(QColor(0, 0, 255));
		    pen.setColor(Qt::black);
		}
	    } 
	    else
	    {
		if (m_type == QGis::Line)
		{
		    pen.setColor(QColor(255 / numberofclassesspinbox->value() * (i+1), 0, 255 - (255 / numberofclassesspinbox->value() * (i+1))));
		} 
		else                //point or polygon
		{
		    brush.setColor(QColor(255 / numberofclassesspinbox->value() * (i+1), 0, 255 - (255 / numberofclassesspinbox->value() * (i+1))));
		    pen.setColor(Qt::black);
		}
	    }
	    pen.setWidth(1);
	    brush.setStyle(Qt::SolidPattern);
	    symbol->setPen(pen);
	    symbol->setBrush(brush);
       
	mEntries.insert(std::make_pair(listboxtext,symbol));
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsGraSyDialog::changeCurrentValue()
{
    sydialog.blockSignals(true);//block signals to prevent sydialog from changing the current QgsRenderItem
    QListBoxItem* item=mClassBreakBox->selectedItem();
    if(item)
    {
	QString value=item->text();
	std::map<QString,QgsSymbol*>::iterator it=mEntries.find(value);
	if(it!=mEntries.end())
	{
	    sydialog.set((*it).second);
	    sydialog.setLabel((*it).second->label());
	}
    }
    sydialog.blockSignals(false);
}

void QgsGraSyDialog::applySymbologyChanges()
{
    QListBoxItem* item=mClassBreakBox->selectedItem();
    if(item)
    {
	QString value=item->text();
	std::map<QString,QgsSymbol*>::iterator it=mEntries.find(value);
	if(it!=mEntries.end())
	{
	    sydialog.apply((*it).second);
	    it->second->setLabel((*it).second->label());
	}
    }
}

void QgsGraSyDialog::changeClass(QListBoxItem* item)
{
    QString currenttext=item->text();
    QgsSymbol* symbol=0;
    std::map<QString,QgsSymbol*>::iterator iter=mEntries.find(currenttext);
    if(iter!=mEntries.end())
    {
	symbol=iter->second;
    }
    QgsLUDialog dialog(this);
    
    if(symbol)
    {
	dialog.setLowerValue(symbol->lowerValue());
	dialog.setUpperValue(symbol->upperValue());
    }

    if(dialog.exec()==QDialog::Accepted)
    {
	if(symbol)
	{
	    mEntries.erase(currenttext);
	    symbol->setLowerValue(dialog.lowerValue());
	    symbol->setUpperValue(dialog.upperValue());
	    QString newclass=dialog.lowerValue()+"-"+dialog.upperValue();
	    mEntries.insert(std::make_pair(newclass,symbol));
	    int index=mClassBreakBox->index(item);
	    QObject::disconnect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
	    mClassBreakBox->removeItem(index);
	    mClassBreakBox->insertItem(newclass,index);
	    mClassBreakBox->setSelected(index,true);
	    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
	}	
    }
}
