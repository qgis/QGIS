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
    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
    QObject::connect(&sydialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
    mSymbolWidgetStack->addWidget(&sydialog);
    mSymbolWidgetStack->raiseWidget(&sydialog);

    //restore settings if unique value renderer was read from a project file
    QgsUniqueValRenderer *renderer;
    //initial settings, use the buffer of the propertiesDialog if possible. If this is not possible, use the renderer of the vectorlayer directly
    if (mVectorLayer->propertiesDialog())
    {
	renderer = dynamic_cast < QgsUniqueValRenderer * >(mVectorLayer->propertiesDialog()->getBufferRenderer());
    } 
    else
    {
	renderer = dynamic_cast < QgsUniqueValRenderer * >(mVectorLayer->renderer());
    }


    if (renderer)
    {
	mClassBreakBox->clear();
	std::list<int>::iterator iter=renderer->classificationAttributes().begin();
	int classattr=*iter;
	mClassificationComboBox->setCurrentItem(classattr);
	mClassBreakBox->setCurrentItem(0);
	changeClassificationAttribute(classattr);
    }
    
}

QgsUValDialog::~QgsUValDialog()
{
    for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUValDialog::apply()
{
    //font tor the legend text
    QFont f("arial", 10, QFont::Normal);
    QFontMetrics fm(f);

    int symbolheight = 15;    //height of an area where a symbol is painted
    int symbolwidth = 15;     //width of an area where a symbol is painted
    int rowheight = (fm.height() > symbolheight) ? fm.height() : symbolheight;  //height of a row in the symbology part
    int topspace = 5;
    int bottomspace = 5;
    int leftspace = 5;
    int rightspace = 5;
    int rowspace = 5;
    int wordspace = 5;        //space between graphics/word
    int widestvalue = 0;
    int valuewidth;

    //find out the width of the widest label and of the broadest value string
    int maxlabelwidth=0;
    int maxvaluewidth=0;

    for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	int currentlabelwidth=fm.width(it->second->label());
	if(currentlabelwidth>maxlabelwidth)
	{
	    maxlabelwidth=currentlabelwidth;
	}
	int currentvwidth=fm.width(it->second->value());
	if(currentvwidth>maxvaluewidth)
	{
	    //widestlu = string2;
	    maxvaluewidth=currentvwidth;
	}
    }

    QgsUniqueValRenderer *renderer = dynamic_cast < QgsUniqueValRenderer * >(mVectorLayer->renderer());

    //go through mValues and add the entries to the renderer
    if(renderer)
    {
	renderer->clearValues();
	for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    QgsSymbol* symbol=it->second->getSymbol();
	    QgsSymbol* newsymbol=new QgsSymbol();
	    newsymbol->setPen(symbol->pen());
	    newsymbol->setBrush(symbol->brush());
	    QgsRenderItem* ritem=new QgsRenderItem(newsymbol,it->first,"");
	    renderer->insertValue(it->first,ritem);
	    //find out the width of the string
	    valuewidth=fm.width(it->first);
	    if(valuewidth>widestvalue)
	    {
		widestvalue=valuewidth;
	    }
	}
	renderer->setClassificationField(mClassificationComboBox->currentItem());
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("Warning, typecast failed in qgsuvaldialog.cpp, l. 61"); 
#endif
    }

    //render the legend item
    QPixmap *pix = mVectorLayer->legendPixmap();
    QString name;
    QString field=mClassificationComboBox->currentText();
    int fieldwidth=fm.width(field);
    if(fieldwidth>widestvalue)
    {
	widestvalue=fieldwidth;
    }
    if (mVectorLayer->propertiesDialog())
    {
	name = mVectorLayer->propertiesDialog()->displayName();
    } 
    else
    {
	name = "";
    }
    int namewidth=fm.width(name);
    int pixwidth;
    if(namewidth>widestvalue)
    {
	if(namewidth>(symbolwidth+wordspace+widestvalue+maxlabelwidth))
	{
	    pixwidth = leftspace+wordspace+namewidth+rightspace;
	}
	else
	{
	    pixwidth = leftspace+2*wordspace+symbolwidth+maxlabelwidth+widestvalue+rightspace; 
	}
    }
    else
    {
	pixwidth = leftspace+2*wordspace+symbolwidth+widestvalue+maxlabelwidth+rightspace;
    }
   
    int pixheight = topspace+2*fm.height()+rowspace+(rowheight+rowspace)*mValues.size()+bottomspace;
    
    pix->resize(pixwidth,pixheight);
    pix->fill();
    QPainter p(pix);
    p.setFont(f);
    
    //draw the layer name and the name of the classification field into the pixmap
    p.drawText(leftspace, topspace + fm.height(), name);
    p.drawText(leftspace, topspace + 2 * fm.height(), field);
    int intermheight=topspace+2*fm.height()+rowspace;
    int row=0;

    for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	QgsSymbol* sym=it->second->getSymbol();
	p.setPen(sym->pen());
	p.setBrush(sym->brush());

	if (mVectorLayer->vectorType() == QGis::Polygon)
	{
	    p.drawRect(leftspace,intermheight+row*(rowheight+rowspace)+rowheight-symbolheight,symbolwidth,symbolheight); 
	}
	else if (mVectorLayer->vectorType() == QGis::Line)
	{
	    p.drawLine(leftspace,intermheight+row*(rowheight+rowspace)+rowheight-symbolheight,leftspace+symbolwidth,intermheight+row*(rowheight+rowspace)+rowheight);
	}
	else if (mVectorLayer->vectorType() == QGis::Point)
	{
	    p.drawRect(leftspace + symbolwidth / 2, intermheight + (int) ((rowheight+rowspace) * (row + 0.5)),5,5);
	}
	p.setPen(Qt::black);
	p.drawText(leftspace+symbolwidth+wordspace, intermheight+row*(rowheight+rowspace)+rowheight, it->first);
	p.drawText(leftspace+symbolwidth+2*wordspace+widestvalue, intermheight+row*(rowheight+rowspace)+rowheight, it->second->label());
	++row;
    }

    
    
    mVectorLayer->updateItemPixmap();
    mVectorLayer->triggerRepaint();
}

void QgsUValDialog::changeClassificationAttribute(int nr)
{
#ifdef QGISDEBUG
    qWarning("in changeClassificationAttribute, nr is: "+QString::number(nr));
#endif

    //delete old entries
    for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }
    mValues.clear();
    
    QgsVectorDataProvider *provider = mVectorLayer->getDataProvider();
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
		symbol=new QgsSymbol();
		ritemptr=new QgsRenderItem(symbol,"","");
		mValues.insert(std::make_pair(value,ritemptr));
	    }
	    delete f;
	}

	//set symbology for all QgsSiSyDialogs
	QColor thecolor;
	double number=0;
	double frac;

	for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    ++number;
	    //color range from blue to red
	    frac=number/mValues.size();
	    thecolor.setRgb(int(255*frac),0,int(255-(255*frac)));
	    mClassBreakBox->insertItem(it->first);
	    QgsSymbol* sym=it->second->getSymbol();
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
    QListBoxItem* item=mClassBreakBox->selectedItem();
    QString value=item->text();
    std::map<QString,QgsRenderItem*>::iterator it=mValues.find(value);
    if(it!=mValues.end())
    {
	QPen& pen=it->second->getSymbol()->pen();
	QBrush& brush=it->second->getSymbol()->brush();
	QColor fcolor(brush.color().red(),brush.color().green(),brush.color().blue());
	QColor ocolor(pen.color().red(),pen.color().green(),pen.color().blue());
	sydialog.setFillColor(fcolor);
	sydialog.setFillStyle(brush.style());
	sydialog.setOutlineColor(ocolor);
	sydialog.setOutlineStyle(pen.style());
	sydialog.setOutlineWidth(pen.width());
	sydialog.setLabel(it->second->label());
    }
    else
    {
	//no entry found
    }
}

void QgsUValDialog::applySymbologyChanges()
{
  QListBoxItem* item=mClassBreakBox->selectedItem();
  QString value=item->text();
  std::map<QString,QgsRenderItem*>::iterator it=mValues.find(value);
  if(it!=mValues.end())
  {
      QPen& pen=it->second->getSymbol()->pen();
      QBrush& brush=it->second->getSymbol()->brush(); 
      pen.setWidth(sydialog.getOutlineWidth());
      pen.setColor(sydialog.getOutlineColor());
      pen.setStyle(sydialog.getOutlineStyle());
      brush.setColor(sydialog.getFillColor());
      brush.setStyle(sydialog.getFillStyle());
      it->second->setLabel(sydialog.label());
  }
}
