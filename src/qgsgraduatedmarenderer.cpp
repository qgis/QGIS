/***************************************************************************
                         qgsgraduatedmarenderer.cpp  -  description
                             -------------------
    begin                : April 2004
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
/* $Id */

#include "qgsdlgvectorlayerproperties.h"
#include "qgsfeature.h"
//#include "qgsfeatureattribute.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsgramadialog.h"
#include "qgslegenditem.h"
#include "qgsmarkersymbol.h"
#include "qgssvgcache.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"
#include <iostream>
#include <qdom.h>
#include <qpainter.h>
#include <vector>

QgsGraduatedMaRenderer::~QgsGraduatedMaRenderer()
{
    removeItems();
}

void QgsGraduatedMaRenderer::removeItems()
{
    //free the memory first
    for (std::list < QgsRangeRenderItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
	delete *it;
    }

    //and remove the pointers then
    mItems.clear();
}

void QgsGraduatedMaRenderer::initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr)
{
    bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if (pr)
    {
	toproperties = true;
    }

    setClassificationField(0);    //the classification field does not matter
    
    if (layer)
    {
	QgsGraMaDialog *dialog = new QgsGraMaDialog(layer);

	if (toproperties)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
	    QgsLegendItem *item;
	    if (item = layer->legendItem())
            {
		//item->setPixmap(0, (*pixmap));
            }
        }
    }
    else
    {
	qWarning("Warning, layer is null in QgsGraduatedMaRenderer::initializeSymbology(..)");
    }
}
    
void QgsGraduatedMaRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, 
	     double* scalefactor, bool selected, int oversampling, double widthScale)
{
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::NoBrush);

    //first find out the value for the classification attribute
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    double value = vec[0].fieldValue().toDouble();
    //double value = vec[mClassificationField].fieldValue().toDouble();

    std::list < QgsRangeRenderItem * >::iterator it;
    //first find the first render item which contains the feature
    for (it = mItems.begin(); it != mItems.end(); ++it)
    {
	if (value >= (*it)->value().toDouble() && value <= (*it)->upper_value().toDouble())
        {
	    break;
        }
    }
    
    if (it == mItems.end())      //value is contained in no item
    {
	QPicture p;
	*pic=p;
	return;
    } 
    else
    {
	QgsMarkerSymbol* ms=dynamic_cast<QgsMarkerSymbol*>((*it)->getSymbol());
	if(ms&&pic)
	{
	    QString picture=ms->picture();
	    if(picture=="unnamed")
	    {
		QPicture p;
		*pic=p;
	    }
	    else
	    {
	        QPainter p(pic);
		QPixmap pix = QgsSVGCache::instance().
		  getPixmap(ms->picture(), ms->scaleFactor());
		p.drawPixmap(0, 0, pix);
	    }
	    (*scalefactor) = 1;
	    
	    if(selected)
	    {
		QRect bound=pic->boundingRect();
		QPainter painter(pic);
		painter.setBrush(QColor(255,255,0));
		painter.drawRect(0,0,bound.width(),bound.height());
	    }
	}
    }
}

void QgsGraduatedMaRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode rangerendernode = rnode.namedItem("rangerenderitem");
    while (!rangerendernode.isNull())
    {
	QDomNode lvnode = rangerendernode.namedItem("lowervalue");
	QString lowervalue = lvnode.toElement().text();

	QDomNode uvnode = rangerendernode.namedItem("uppervalue");
	QString uppervalue = uvnode.toElement().text();

	QgsMarkerSymbol* msy = new QgsMarkerSymbol();
	QPen pen;
	QBrush brush;
	QString svgpath;
	double scalefactor;
	QString value, label;

	QDomNode synode = rangerendernode.namedItem("markersymbol");
	
	QDomNode svgnode = synode.namedItem("svgpath");
	svgpath = svgnode.toElement().text();
	
	QDomNode scalenode = synode.namedItem("scalefactor");
	scalefactor = scalenode.toElement().text().toDouble();

	QDomNode outlcnode = synode.namedItem("outlinecolor");
	QDomElement oulcelement = outlcnode.toElement();
	int red = oulcelement.attribute("red").toInt();
	int green = oulcelement.attribute("green").toInt();
	int blue = oulcelement.attribute("blue").toInt();
	pen.setColor(QColor(red, green, blue));

	QDomNode outlstnode = synode.namedItem("outlinestyle");
	QDomElement outlstelement = outlstnode.toElement();
	pen.setStyle(QgsSymbologyUtils::qString2PenStyle(outlstelement.text()));

	QDomNode outlwnode = synode.namedItem("outlinewidth");
	QDomElement outlwelement = outlwnode.toElement();
	pen.setWidth(outlwelement.text().toInt());

	QDomNode fillcnode = synode.namedItem("fillcolor");
	QDomElement fillcelement = fillcnode.toElement();
	red = fillcelement.attribute("red").toInt();
	green = fillcelement.attribute("green").toInt();
	blue = fillcelement.attribute("blue").toInt();
	brush.setColor(QColor(red, green, blue));

	QDomNode fillpnode = synode.namedItem("fillpattern");
	QDomElement fillpelement = fillpnode.toElement();
	brush.setStyle(QgsSymbologyUtils::qString2BrushStyle(fillpelement.text()));

	QDomNode lnode = rangerendernode.namedItem("label");
	QDomElement lnodee = lnode.toElement();
	label = lnodee.text();

#ifdef QGISDEBUG
	qWarning("label is: ");
	qWarning(label);
#endif

	//create a renderer and add it to the vector layer
	msy->setBrush(brush);
	msy->setPen(pen);
	msy->setPicture(svgpath);

#ifdef QGISDEBUG
	qWarning("the svgpath: "+svgpath);
#endif
	msy->setScaleFactor(scalefactor);
#ifdef QGISDEBUG
	qWarning("the scalefactor: "+QString::number(scalefactor,'f',2));
#endif
	QgsRangeRenderItem* ri = new QgsRangeRenderItem(msy,lowervalue,uppervalue,label);
#ifdef QGISDEBUG
	qWarning("lowervalue "+lowervalue);
	qWarning("uppervalue "+uppervalue);
#endif
	this->addItem(ri);

	rangerendernode = rangerendernode.nextSibling();
    }

    vl.setRenderer(this);
    QgsGraMaDialog *gdialog = new QgsGraMaDialog(&vl);
    vl.setRendererDialog(gdialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Graduated Marker");

    gdialog->apply();
}

void QgsGraduatedMaRenderer::writeXML(std::ostream& xml)
{
    xml << "\t\t<graduatedmarker>\n";
    xml << "\t\t\t<classificationfield>" + QString::number(this->classificationField()) +
	"</classificationfield>\n";
    for (std::list < QgsRangeRenderItem * >::iterator it = this->items().begin(); it != this->items().end();
	 ++it)
    {
	xml << "\t\t\t<rangerenderitem>\n";
	xml << "\t\t\t\t<lowervalue>" + (*it)->value() + "</lowervalue>\n";
	xml << "\t\t\t\t<uppervalue>" + (*it)->upper_value() + "</uppervalue>\n";
	QgsMarkerSymbol *markersymbol = dynamic_cast<QgsMarkerSymbol*>((*it)->getSymbol());
	xml << "\t\t\t\t<markersymbol>\n";
	xml << "\t\t\t\t\t<svgpath>" + markersymbol->picture() + "</svgpath>\n";
	xml << "\t\t\t\t\t<scalefactor>" + QString::number(markersymbol->scaleFactor()) + "</scalefactor>\n";
	xml << "\t\t\t\t\t<outlinecolor red=\"" + QString::number(markersymbol->pen().color().red()) + "\" green=\"" +
	    QString::number(markersymbol->pen().color().green()) + "\" blue=\"" + QString::number(markersymbol->pen().color().blue()) +
	    "\" />\n";
	xml << "\t\t\t\t\t<outlinestyle>" + QgsSymbologyUtils::penStyle2QString(markersymbol->pen().style()) + "</outlinestyle>\n";
	xml << "\t\t\t\t\t<outlinewidth>" + QString::number(markersymbol->pen().width()) + "</outlinewidth>\n";
	xml << "\t\t\t\t\t<fillcolor red=\"" + QString::number(markersymbol->brush().color().red()) + "\" green=\"" +
	    QString::number(markersymbol->brush().color().green()) + "\" blue=\"" + QString::number(markersymbol->brush().color().blue()) +
	    "\" />\n";
	xml << "\t\t\t\t\t<fillpattern>" + QgsSymbologyUtils::brushStyle2QString(markersymbol->brush().style()) +
	    "</fillpattern>\n";
	xml << "\t\t\t\t</markersymbol>\n";
	xml << "\t\t\t\t<label>" + (*it)->label() + "</label>\n";
	xml << "\t\t\t</rangerenderitem>\n";
    }
    xml << "\t\t</graduatedmarker>\n";
}

std::list<int> QgsGraduatedMaRenderer::classificationAttributes()
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

QString QgsGraduatedMaRenderer::name()
{
    return "Graduated Marker";
}

bool QgsGraduatedMaRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    bool returnvalue=true;
    QDomNode graduatedmarker=document.createElement("graduatedmarker");
    layer_node.appendChild(graduatedmarker);
    QDomNode classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    graduatedmarker.appendChild(classificationfield);
    for(std::list<QgsRangeRenderItem*>::iterator it=mItems.begin();it!=mItems.end();++it)
    {
	if(!(*it)->writeXML(graduatedmarker,document))
	{
	    returnvalue=false;
	}
    }
    return returnvalue;
}

const std::list<QgsRenderItem*> QgsGraduatedMaRenderer::items() const
{
    std::list<QgsRenderItem*> list;
    for(std::list<QgsRangeRenderItem*>::const_iterator iter=mItems.begin();iter!=mItems.end();++iter)
    {
	list.push_back(*iter);
    }
    return list;
}
