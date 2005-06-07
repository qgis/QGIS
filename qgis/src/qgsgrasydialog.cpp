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
    QgsGraduatedSymRenderer *renderer;
    
    renderer = dynamic_cast < QgsGraduatedSymRenderer * >(layer->renderer());
    
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

	//font tor the legend text
	//QFont f("arial", 10, QFont::Normal);
	//QFontMetrics fm(f);
	
	//spaces
	//int topspace = 5;
	//int bottomspace = 5;
	//int leftspace = 10;       //space between left side of the pixmap and the text/graphics
	//int rightspace = 5;       //space between text/graphics and right side of the pixmap
	//int wordspace = 5;        //space between graphics/word
	//int symbolheight = 15;    //height of an area where a symbol is painted
	//int symbolwidth = 15;     //width of an area where a symbol is painted
	//int lowerupperwidth; //widht of the broadest lower-upper pair
	//int rowspace = 5;         //spaces between rows of symbols
	//int rowheight = (fm.height() > symbolheight) ? fm.height() : symbolheight;  //height of a row in the symbology part

	//find out the width of the widest label and of the broadest lower-upper pair
	//int labelwidth=0;
	//lowerupperwidth=0;

	//for(std::map<QString,QgsRangeRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
	//{
	//  int currentlabelwidth=fm.width(it->second->label());
	//  if(currentlabelwidth>labelwidth)
	//  {
	//	labelwidth=currentlabelwidth;
	//  }
	//  int currentluwidth=fm.width(it->second->value())+fm.width(" - ")+fm.width(it->second->upper_value());
	//  if(currentluwidth>lowerupperwidth)
	//  {
		//widestlu = string2;
	//	lowerupperwidth=currentluwidth;
	//  }
	//}
	
	//create the pixmap for the render item
	//QPixmap *pix = mVectorLayer->legendPixmap();
	//QString name;
	//if (mVectorLayer->propertiesDialog())
        //{
	//  name = mVectorLayer->propertiesDialog()->displayName();
	//} 
	//else
        //{
	//    name = "";
        //}
	//query the name and the maximum upper value to estimate the necessary width of the pixmap
	//int pixwidth = leftspace + rightspace + symbolwidth + 2 * wordspace + labelwidth + lowerupperwidth; //width of the pixmap with symbol and values
	//consider 240 pixel for labels
	//int namewidth = leftspace + fm.width(name) + rightspace;
	//int width = (pixwidth > namewidth) ? pixwidth : namewidth;
	//pix->resize(width, topspace + 2 * fm.height() + bottomspace + (rowheight + rowspace) * numberofclassesspinbox->value());
	//pix->fill();
	//QPainter p(pix);
	//p.setFont(f);
	//draw the layer name and the name of the classification field into the pixmap
	//p.drawText(leftspace, topspace + fm.height(), name);
	//p.drawText(leftspace, topspace + 2 * fm.height(), classificationComboBox->currentText());
	
	QgsGraduatedSymRenderer *renderer = dynamic_cast < QgsGraduatedSymRenderer * >(mVectorLayer->renderer());
	
	if (!renderer)
        {
	    renderer=new QgsGraduatedSymRenderer(mVectorLayer->vectorType());
	    mVectorLayer->setRenderer(renderer);
        }
	
	renderer->removeSymbols();
	
	//int offset = topspace + 2 * fm.height();
	//int rowincrement = rowheight + rowspace;
	//int i=0;

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
		//add the symbol to the picture

		//QString legendstring = lower_bound + " - " + upper_bound;
		//p.setPen(sy->pen());
		//p.setBrush(sy->brush());
		//if (mVectorLayer->vectorType() == QGis::Polygon)
                //{
		//p.drawRect(leftspace, offset + rowincrement * i + (rowheight - symbolheight), symbolwidth, symbolheight); //implement different painting for lines and points here
		//} 
		//else if (mVectorLayer->vectorType() == QGis::Line)
                //{
		//    p.drawLine(leftspace, offset + rowincrement * i + (rowheight - symbolheight), leftspace + symbolwidth,
                //             offset + rowincrement * i + rowheight);
		//} 
		//else if (mVectorLayer->vectorType() == QGis::Point)
                //{
		    //p.drawRect(leftspace + symbolwidth / 2, offset + (int) (rowincrement * (i + 0.5)), 5, 5);
		//  QPixmap pm = sy->getPointSymbolAsPixmap();
		//  p.drawPixmap ( (int) (leftspace+symbolwidth/2-pm.width()/2), (int) (offset+rowincrement*(i+0.5)-pm.height()/2), pm );
                //}
		//p.setPen(Qt::black);
		//p.drawText(leftspace + symbolwidth + wordspace, offset + rowincrement * i + rowheight, legendstring);
		//p.drawText(pixwidth - labelwidth - rightspace, offset + rowincrement * i + rowheight, label);
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
	
	//mVectorLayer->updateItemPixmap();

	//if (mVectorLayer->propertiesDialog())
        //{
	//    mVectorLayer->propertiesDialog()->setRendererDirty(false);
        //}
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
    QgsLUDialog dialog;
    
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
