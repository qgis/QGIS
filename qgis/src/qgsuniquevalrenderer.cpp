/***************************************************************************
                         qgsuniquevalrenderer.cpp  -  description
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
#include "qgsdlgvectorlayerproperties.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvaldialog.h"
#include "qgsrenderitem.h"
#include "qgsfeatureattribute.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbologyutils.h"
#include "qgsuvaldialog.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <vector>

QgsUniqueValRenderer::QgsUniqueValRenderer(): mClassificationField(0)
{
  //call superclass method to set up selection colour
  initialiseSelectionColor();

}

QgsUniqueValRenderer::~QgsUniqueValRenderer()
{
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	delete it->second;
    }
}

void QgsUniqueValRenderer::initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr)
{
    mVectorType = layer->vectorType();
    QgsUValDialog *dialog = new QgsUValDialog(layer);

	if (pr)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
        }
}
    
void QgsUniqueValRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, 
	double* scalefactor, bool selected, int oversampling, double widthScale)
{
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    QString value = vec[0].fieldValue();
    std::map<QString,QgsRenderItem*>::iterator it=mEntries.find(value);
    if(it!=mEntries.end())
    {
	QgsRenderItem *item = it->second;

	// Point 
	if ( pic && mVectorType == QGis::Point ) {
	    *pic = item->getSymbol()->getPointSymbolAsPicture( oversampling, widthScale,
		                                             selected, mSelectionColor );
	    
	    if ( scalefactor ) *scalefactor = 1;
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		QPen pen=item->getSymbol()->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		p->setPen(item->getSymbol()->pen());
		p->setBrush(item->getSymbol()->brush());
	    }
	    else
	    {
		QPen pen=item->getSymbol()->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		pen.setColor(mSelectionColor);
		QBrush brush=item->getSymbol()->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("Warning, no render item found in QgsUniqueValRenderer::renderFeature");
#endif
    }
    
}

void QgsUniqueValRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode renderitemnode = rnode.namedItem("renderitem");
    while (!renderitemnode.isNull())
    {
	QDomNode valuenode = renderitemnode.namedItem("value");
	QString value = valuenode.toElement().text();
#ifdef QGISDEBUG
	qWarning("readXML, value is "+value);
#endif
	QgsSymbol* msy = new QgsSymbol();
	QPen pen;
	QBrush brush;
	
	QDomNode synode = renderitemnode.namedItem("symbol");

	msy->readXML ( synode );

	QDomElement labelelement = renderitemnode.namedItem("label").toElement();
	QString label = labelelement.text();

	//create a renderitem and add it to the renderer

	QgsRenderItem *ri = new QgsRenderItem(msy, value, label);
	this->insertValue(value,ri);

	renderitemnode = renderitemnode.nextSibling();
    }

    vl.setRenderer(this);
    QgsUValDialog *uvaldialog = new QgsUValDialog(&vl);
    vl.setRendererDialog(uvaldialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Unique Value");

    uvaldialog->apply();
}

void QgsUniqueValRenderer::clearValues()
{
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	delete it->second;
    }
    mEntries.clear();
}

QString QgsUniqueValRenderer::name()
{
    return "Unique Value";
}

std::list<int> QgsUniqueValRenderer::classificationAttributes()
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

std::map<QString,QgsRenderItem*>& QgsUniqueValRenderer::items()
{
    return mEntries;
}

bool QgsUniqueValRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    bool returnval=true;
    QDomElement uniquevalue=document.createElement("uniquevalue");
    layer_node.appendChild(uniquevalue);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    uniquevalue.appendChild(classificationfield);
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	if(!(it->second)->writeXML(uniquevalue,document))
	{
	    returnval=false;  
	}
    }
    return returnval;
}

const std::list<QgsRenderItem*> QgsUniqueValRenderer::items() const
{
    std::list<QgsRenderItem*> list;
    for(std::map<QString,QgsRenderItem*>::const_iterator iter=mEntries.begin();iter!=mEntries.end();++iter)
    {
	list.push_back(iter->second);
    }
    return list;
}
