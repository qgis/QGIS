/***************************************************************************
                         qgssinglesymrenderer.cpp  -  description
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
#include "qgsgraduatedsymrenderer.h"
#include <cfloat>
#include "qgsgrasydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include <qdom.h>

inline QgsGraduatedSymRenderer::~QgsGraduatedSymRenderer()
{
    //free the memory first
    /*for (std::list < QgsRangeRenderItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
	delete *it;
    }

    //and remove the pointers then
    mItems.clear();*/
    removeItems();
}

void QgsGraduatedSymRenderer::removeItems()
{
    //free the memory first
    for (std::list < QgsRangeRenderItem * >::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
	delete *it;
    }

    //and remove the pointers then
    mItems.clear();
}

void QgsGraduatedSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, double* scalefactor, bool selected)
{
    //first find out the value for the classification attribute
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    //double value = vec[mClassificationField].fieldValue().toDouble();
    double value = vec[0].fieldValue().toDouble();

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
	std::cout << "Warning, value is contained in no class" << std::endl << std::flush;
	return;
    } 
    else
    {
	//set the qpen and qpainter to the right values
	p->setPen((*it)->getSymbol()->pen());
	p->setBrush((*it)->getSymbol()->brush());
    }
    
    if(selected)
    {
	 QPen pen=(*it)->getSymbol()->pen();
	 pen.setColor(mSelectionColor);
	 QBrush brush=(*it)->getSymbol()->brush();
	 brush.setColor(mSelectionColor);
	 p->setPen(pen);
	 p->setBrush(brush);
    }
}

void QgsGraduatedSymRenderer::initializeSymbology(QgsVectorLayer * layer, QgsDlgVectorLayerProperties * pr)
{
    bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if (pr)
    {
	toproperties = true;
    }

    setClassificationField(0);    //the classification field does not matter
    
    if (layer)
    {
	
	QgsGraSyDialog *dialog = new QgsGraSyDialog(layer);

	if (toproperties)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
	    QgsLegendItem *item;
	    layer->updateItemPixmap();
        }
    } 
    else
    {
	qWarning("Warning, layer is null in QgsGraduatedSymRenderer::initializeSymbology(..)");
    }
}

void QgsGraduatedSymRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode rangerendernode = rnode.namedItem("rangerenderitem");
    while (!rangerendernode.isNull())
    {
	QgsSymbol* sy = new QgsSymbol();
	QPen pen;
	QBrush brush;

	QDomNode lvnode = rangerendernode.namedItem("lowervalue");
	QString lowervalue = lvnode.toElement().text();

	QDomNode uvnode = rangerendernode.namedItem("uppervalue");
	QString uppervalue = uvnode.toElement().text();

	QDomNode synode = rangerendernode.namedItem("symbol");

	QDomElement oulcelement = synode.namedItem("outlinecolor").toElement();
	int red = oulcelement.attribute("red").toInt();
	int green = oulcelement.attribute("green").toInt();
	int blue = oulcelement.attribute("blue").toInt();
	pen.setColor(QColor(red, green, blue));

	QDomElement oustelement = synode.namedItem("outlinestyle").toElement();
	pen.setStyle(QgsSymbologyUtils::qString2PenStyle(oustelement.text()));

	QDomElement oulwelement = synode.namedItem("outlinewidth").toElement();
	pen.setWidth(oulwelement.text().toInt());

	QDomElement fillcelement = synode.namedItem("fillcolor").toElement();
	red = fillcelement.attribute("red").toInt();
	green = fillcelement.attribute("green").toInt();
	blue = fillcelement.attribute("blue").toInt();
	brush.setColor(QColor(red, green, blue));

	QDomElement fillpelement = synode.namedItem("fillpattern").toElement();
	brush.setStyle(QgsSymbologyUtils::qString2BrushStyle(fillpelement.text()));

	QDomElement labelelement = rangerendernode.namedItem("label").toElement();
	QString label = labelelement.text();

	//create a renderitem and add it to the renderer
	sy->setBrush(brush);
	sy->setPen(pen);

	QgsRangeRenderItem *ri = new QgsRangeRenderItem(sy, lowervalue, uppervalue, label);
	this->addItem(ri);

	rangerendernode = rangerendernode.nextSibling();
    }

    vl.setRenderer(this);
    QgsGraSyDialog *gdialog = new QgsGraSyDialog(&vl);
    vl.setRendererDialog(gdialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Graduated Symbol");

    gdialog->apply();
}

void QgsGraduatedSymRenderer::writeXML(std::ostream& xml)
{
    xml << "\t\t<graduatedsymbol>\n";
    xml << "\t\t\t<classificationfield>" << QString::number(this->classificationField()).ascii() <<
	"</classificationfield>\n";
    for (std::list < QgsRangeRenderItem * >::iterator it = this->items().begin(); it != this->items().end();
	 ++it)
    {
	xml << "\t\t\t<rangerenderitem>\n";
	xml << "\t\t\t\t<lowervalue>" << (*it)->value().ascii() << "</lowervalue>\n";
	xml << "\t\t\t\t<uppervalue>" << (*it)->upper_value().ascii() << "</uppervalue>\n";
	xml << "\t\t\t\t<symbol>\n";
	QgsSymbol *symbol = (*it)->getSymbol();
	xml << "\t\t\t\t\t<outlinecolor red=\"" << QString::number(symbol->pen().color().red()).ascii() << "\" green=\"" <<
	    QString::number(symbol->pen().color().green()).ascii() << "\" blue=\"" << QString::number(symbol->pen().color().blue()).ascii()  << 
	    "\" />\n";
	xml << "\t\t\t\t\t<outlinestyle>" << QgsSymbologyUtils::penStyle2QString(symbol->pen().style()).ascii()  << 
	    "</outlinestyle>\n";
	xml << "\t\t\t\t\t<outlinewidth>" << QString::number(symbol->pen().width()).ascii() << "</outlinewidth>\n";
	xml << "\t\t\t\t\t<fillcolor red=\"" << QString::number(symbol->brush().color().red()).ascii() << "\" green=\""  << 
	    QString::number(symbol->brush().color().green()).ascii() << "\" blue=\""  << 
	    QString::number(symbol->brush().color().blue()).ascii() << "\" />\n";
	xml << "\t\t\t\t\t<fillpattern>" << QgsSymbologyUtils::brushStyle2QString(symbol->brush().style()).ascii()  << 
	    "</fillpattern>\n";
	xml << "\t\t\t\t</symbol>\n";
	xml << "\t\t\t\t<label>" << (*it)->label().ascii() << "</label>\n";
	xml << "\t\t\t</rangerenderitem>\n";
    }
    xml << "\t\t</graduatedsymbol>\n";
}

std::list<int> QgsGraduatedSymRenderer::classificationAttributes()
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

QString QgsGraduatedSymRenderer::name()
{
    return "Graduated Symbol";
}
