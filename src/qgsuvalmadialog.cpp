/***************************************************************************
                         qgsuvalmadialog.cpp  -  unique value marker dialog
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net
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
#include "qgsuvalmadialog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgssimadialog.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"
#include "qgssvgcache.h"
#include "qgsrenderitem.h"
#include "qgsuvalmarenderer.h"
#include <qapplication.h>
#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpainter.h>
#include <cassert>
#include <list>
#ifdef WIN32
extern QString PKGDATAPATH;
#endif

QgsUValMaDialog::QgsUValMaDialog(QgsVectorLayer* vl): QgsUValMaDialogBase(), mVectorLayer(vl), madialog(vl)
{
    setSizeGripEnabled(true); 

    //find out the fields of mVectorLayer
    QgsVectorDataProvider *provider;
    if (provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider()))
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
    QObject::connect(&madialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
    mSymbolWidgetStack->addWidget(&madialog);
    mSymbolWidgetStack->raiseWidget(&madialog);

    //restore settings if unique value renderer was read from a project file
    QgsUValMaRenderer *renderer;
    //initial settings, use the buffer of the propertiesDialog if possible. If this is not possible, use the renderer of the vectorlayer directly
    if (mVectorLayer->propertiesDialog())
    {
	renderer = dynamic_cast < QgsUValMaRenderer * >(mVectorLayer->propertiesDialog()->getBufferRenderer());
    } 
    else
    {
	renderer = dynamic_cast < QgsUValMaRenderer * >(mVectorLayer->renderer());
    }


    if (renderer)
    {
	mClassBreakBox->clear();
	std::list<int>::iterator iter=renderer->classificationAttributes().begin();
	int classattr=*iter;
	mClassificationComboBox->setCurrentItem(classattr);
	if(renderer->items().size()==0)
	{
	    changeClassificationAttribute(classattr);
	}

	for(std::map<QString,QgsRenderItem*>::iterator it=renderer->items().begin();it!=renderer->items().end();++it)
	{
	    QgsRenderItem* item=(*it).second;
	    QString itemvalue=item->value();
	    QgsMarkerSymbol* s=dynamic_cast<QgsMarkerSymbol*>(item->getSymbol());
	    if(s)
	    {
		QgsMarkerSymbol* sym=new QgsMarkerSymbol();
		QgsRenderItem* ritem=new QgsRenderItem(sym,item->value(),item->label());
		sym->setPen(s->pen());
		sym->setBrush(s->brush());
		sym->setPicture(s->picture());
		sym->setScaleFactor(s->scaleFactor());
		mValues.insert(std::make_pair(itemvalue,ritem));
		mClassBreakBox->insertItem(itemvalue);
	    }
	}

	mClassBreakBox->setCurrentItem(0);
    }
    
}

QgsUValMaDialog::~QgsUValMaDialog()
{
  for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
	it->second = NULL;
    }
}

void QgsUValMaDialog::apply()
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
	    maxvaluewidth=currentvwidth;
	}
    }

    QgsUValMaRenderer *renderer = dynamic_cast < QgsUValMaRenderer * >(mVectorLayer->renderer());

    //go through mValues and add the entries to the renderer
    int pixheight = topspace+2*fm.height()+rowspace;
    if(renderer)
    {
	renderer->clearValues();
	for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	  QgsMarkerSymbol* symbol = 
	    dynamic_cast<QgsMarkerSymbol*>(it->second->getSymbol());
	  QgsMarkerSymbol* newsymbol=new QgsMarkerSymbol();
	  newsymbol->setPicture(symbol->picture());
	  newsymbol->setScaleFactor(symbol->scaleFactor());
	  QgsRenderItem* ritem=new QgsRenderItem(newsymbol,it->first,"");
	  renderer->insertValue(it->first,ritem);

	  //find out the width of the string
	  valuewidth = fm.width(it->first);
	  if(valuewidth>widestvalue) {
	    widestvalue=valuewidth;
	  }
	  
	  // add to the total height
	  QPixmap pm = QgsSVGCache::instance().
	    getPixmap(symbol->picture(), symbol->scaleFactor());
	  pixheight += ((rowheight > pm.height() ? rowheight : pm.height()) +
			rowspace);
	  
	  // find out the width of the symbols
	  if (pm.width() > symbolwidth)
	    symbolwidth = pm.width();
	  
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
   
    pixheight += bottomspace;
    
    pix->resize(pixwidth,pixheight);
    pix->fill();
    QPainter p(pix);
    p.setFont(f);
    
    //draw the layer name and the name of the classification field into the pixmap
    p.drawText(leftspace, topspace + fm.height(), name);
    p.drawText(leftspace, topspace + 2 * fm.height(), field);

    // draw symbols and values for the different classes
    int intermheight=topspace+2*fm.height()+rowspace;
    int currentheight = intermheight;
    for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
        QgsMarkerSymbol* sym = 
	  dynamic_cast<QgsMarkerSymbol*>(it->second->getSymbol());
	QPixmap pm = QgsSVGCache::instance().getPixmap(sym->picture(),
						       sym->scaleFactor());
	p.drawPixmap(leftspace + (symbolwidth - pm.width()) / 2, 
		     currentheight, pm);
	p.setPen(Qt::black);
	p.drawText(leftspace+symbolwidth+wordspace, currentheight + rowheight,
		   it->first);
	p.drawText(leftspace+symbolwidth+2*wordspace+widestvalue, 
		   currentheight + rowheight, it->second->label());
	currentheight += (rowheight > pm.height() ? rowheight : pm.height()) + 
	  rowspace;
    }
    
    mVectorLayer->updateItemPixmap();
    mVectorLayer->triggerRepaint();
}

void QgsUValMaDialog::changeClassificationAttribute(int nr)
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
    
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
    if (provider)
    {
	QString value;
	std::list<int> attlist;
	attlist.push_back(nr);
	std::vector < QgsFeatureAttribute > vec;
	QgsMarkerSymbol* symbol;
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
		symbol=new QgsMarkerSymbol();
		ritemptr=new QgsRenderItem(symbol,"","");
		mValues.insert(std::make_pair(value,ritemptr));
	    }
	    delete f;
	}

	//set symbology for all QgsSiMaDialogs
	QColor thecolor;
	double number=0;
	double frac;

	for(std::map<QString,QgsRenderItem*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    // set all markers to svg/symbol/Cross4.svg with scale factor from
	    // 0.4 to 1.0
	    const double minSize = 0.4, maxSize = 1.0;
	    frac=number/mValues.size();
	    mClassBreakBox->insertItem(it->first);
	    QgsMarkerSymbol* sym = 
	      dynamic_cast<QgsMarkerSymbol*>(it->second->getSymbol());
	    assert(sym != NULL);
#if defined(WIN32) || defined(Q_OS_MACX)
        QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
	    sym->setPicture(QString(PKGDATAPATH) + "/svg/symbol/Cross4.svg");
	    sym->setScaleFactor(minSize + (maxSize - minSize) * frac);
	    ++number;
	}
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUValMaDialog::changeCurrentValue()
{
    QListBoxItem* item=mClassBreakBox->selectedItem();
    QString value=item->text();
    std::map<QString,QgsRenderItem*>::iterator it=mValues.find(value);
    if(it!=mValues.end())
    {
        QgsMarkerSymbol* ms = 
	  dynamic_cast<QgsMarkerSymbol*>(it->second->getSymbol());
	assert(ms != NULL);
	madialog.setMarker(ms->picture(), ms->scaleFactor());
    }
    else
    {
	//no entry found
    }
}

void QgsUValMaDialog::applySymbologyChanges()
{
  QListBoxItem* item=mClassBreakBox->selectedItem();
  QString value=item->text();
  std::map<QString,QgsRenderItem*>::iterator it=mValues.find(value);
  if(it!=mValues.end())
  {
    QgsMarkerSymbol* ms = dynamic_cast<QgsMarkerSymbol*>(it->second->
							 getSymbol());
    assert(ms != NULL);
    std::cerr<<"madialog.getPicture() = "<<madialog.getPicture()<<std::endl;
    ms->setPicture(madialog.getPicture());
    ms->setScaleFactor(madialog.getScaleFactor());
  }
}
