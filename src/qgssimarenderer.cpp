/***************************************************************************
                            qgssimarenderer.cpp 
                            Single marker renderer
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
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

#include "qgssimarenderer.h"
#include "qgssimadialog.h"
#include "qgssvgcache.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsmarkersymbol.h"
#include "qgssymbologyutils.h"
#include <qpainter.h>
#include <qdom.h>

void QgsSiMaRenderer::initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr)
{
    bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if (pr)
    {
	toproperties = true;
    }
    
    if (layer)
    {
	QgsMarkerSymbol* sy=new QgsMarkerSymbol();
	sy->brush().setStyle(Qt::NoBrush);
	sy->pen().setStyle(Qt::NoPen);
	sy->pen().setWidth(1);//set width 1 as default instead of width 0

	QgsRenderItem* ri = new QgsRenderItem();
	ri->setSymbol(sy);
	addItem(ri);
	
	//todo: add a pixmap for the legend
	

	QgsSiMaDialog *dialog = new QgsSiMaDialog(layer);
	if (toproperties)
	{
	    pr->setBufferDialog(dialog);
	} 
	else
	{
	    layer->setRendererDialog(dialog);
	}
    }
}

void QgsSiMaRenderer::renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor, bool selected)
{
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::NoBrush);

    QgsMarkerSymbol* ms=dynamic_cast<QgsMarkerSymbol*>(mItem->getSymbol());
    if(ms&&pic)
    {
        QPainter painter(pic);
	QPixmap pm = QgsSVGCache::instance().getPixmap(ms->picture(), 
						       ms->scaleFactor());
	painter.drawPixmap(0, 0, pm);
	(*scalefactor) = 1;

	if(selected)
	{
	  painter.setBrush(QColor(255,255,0));
	  painter.drawRect(0,0,pm.width(),pm.height());
	}
    }
}

void QgsSiMaRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    QgsMarkerSymbol* msy = new QgsMarkerSymbol();
    QPen pen;
    QBrush brush;
    QString svgpath;
    double scalefactor;
    QString value, label;

    QDomNode rinode = rnode.namedItem("renderitem");
    
    QDomNode vnode = rinode.namedItem("value");
    QDomElement velement = vnode.toElement();
    value = velement.text();

    QDomNode synode = rinode.namedItem("markersymbol");
		    

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

    QDomNode lnode = rinode.namedItem("label");
    QDomElement lnodee = lnode.toElement();
    label = lnodee.text();

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
    QgsRenderItem* ri = new QgsRenderItem();
    ri->setSymbol(msy);
    ri->setLabel(label);
    ri->setValue(value);

    this->addItem(ri);
    vl.setRenderer(this);
    QgsSiMaDialog *smdialog = new QgsSiMaDialog(&vl);
    vl.setRendererDialog(smdialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Single Marker");

    smdialog->apply();
}

void QgsSiMaRenderer::writeXML(std::ostream& xml)
{
    xml << "\t\t<singlemarker>\n";
    xml << "\t\t\t<renderitem>\n";
    xml << "\t\t\t\t<value>" + this->item()->value() + "</value>\n";

    QgsMarkerSymbol *markersymbol = dynamic_cast<QgsMarkerSymbol*>(this->item()->getSymbol());
    if(markersymbol)
    {
	xml << "\t\t\t\t<markersymbol>\n";
	xml << "\t\t\t\t\t<svgpath>" << (const char *) markersymbol->picture() << "</svgpath>\n";
	xml << "\t\t\t\t\t<scalefactor>"  << markersymbol->scaleFactor() << "</scalefactor>\n";
	xml << "\t\t\t\t\t<outlinecolor red=\"" 
    << markersymbol->pen().color().red() 
    << "\" green=\"" 
	  << markersymbol->pen().color().green() 
    << "\" blue=\"" 
    << markersymbol->pen().color().blue()
	  << "\" />\n";
	xml << "\t\t\t\t\t<outlinestyle>" << (const char *) QgsSymbologyUtils::penStyle2QString(markersymbol->pen().style()) << "</outlinestyle>\n";
	xml << "\t\t\t\t\t<outlinewidth>" <<  markersymbol->pen().width() << "</outlinewidth>\n";
	xml << "\t\t\t\t\t<fillcolor red=\"" << markersymbol->brush().color().red() 
    << "\" green=\"" 
    << markersymbol->brush().color().green()
    << "\" blue=\"" 
    << markersymbol->brush().color().blue()
	  << "\" />\n";
	xml << "\t\t\t\t\t<fillpattern>" 
    << (const char *)QgsSymbologyUtils::brushStyle2QString(markersymbol->brush().style())
	  << "</fillpattern>\n";
	xml << "\t\t\t\t</markersymbol>\n";
	xml << "\t\t\t\t<label>" << this->item()->label() << "</label>\n";
	xml << "\t\t\t</renderitem>\n";
	xml << "\t\t</singlemarker>\n";
    }else
    {
	qWarning("warning, type cast failed in qgsprojectio.cpp line 715"); 
    }
}

std::list<int> QgsSiMaRenderer::classificationAttributes()
{
    std::list<int> list;
    return list;//return an empty list
}

QString QgsSiMaRenderer::name()
{
    return "Single Marker";
}
