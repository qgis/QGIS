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

void QgsGraduatedSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, double* scalefactor)
{
    //first find out the value for the classification attribute
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    double value = vec[mClassificationField].fieldValue().toDouble();
    //double value = vec[0].fieldValue().toDouble();

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
	QgsSymbol* sy = new QgsSymbol();
	sy->brush().setStyle(Qt::SolidPattern);
	sy->pen().setStyle(Qt::SolidLine);
	sy->pen().setWidth(1);//set width 1 as default instead of width 0
	
	//random fill colors for points and polygons and pen colors for lines
	int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	
	//font tor the legend text

	QFont f("times", 12, QFont::Normal);
	QFontMetrics fm(f);

	QPixmap *pixmap;
	if (toproperties)
        {
	    pixmap = pr->getBufferPixmap();
	} 
	else
        {
	    pixmap = layer->legendPixmap();
        }
	QString name = layer->name();
	int width = 40 + fm.width(layer->name());
	int height = (fm.height() + 10 > 35) ? fm.height() + 10 : 35;
	pixmap->resize(width, height);
	pixmap->fill();
	QPainter p(pixmap);

	if (layer->vectorType() == QGis::Line)
        {
	    sy->pen().setColor(QColor(red, green, blue));
	    //paint the pixmap for the legend
	    p.setPen(sy->pen());
	    p.drawLine(10, pixmap->height() - 25, 25, pixmap->height() - 10);
	} 
	else
        {
	    sy->brush().setColor(QColor(red, green, blue));
	    sy->pen().setColor(QColor(0, 0, 0));
	    //paint the pixmap for the legend
	    p.setPen(sy->pen());
	    p.setBrush(sy->brush());
	    if (layer->vectorType() == QGis::Point)
            {
		p.drawRect(20, pixmap->height() - 17, 5, 5);
	    } 
	    else                //polygon
            {
		p.drawRect(10, pixmap->height() - 25, 20, 15);
            }

        }

	p.setPen(Qt::black);
	p.setFont(f);
	p.drawText(35, pixmap->height() - 10, name);

	QgsRangeRenderItem *ri = new QgsRangeRenderItem(sy, QString::number(-DBL_MAX, 'f', 2), QString::number(DBL_MAX, 'f', 2), "");
	addItem(ri);

	QgsGraSyDialog *dialog = new QgsGraSyDialog(layer);

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
		item->setPixmap(0, (*pixmap));
            }
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

void QgsGraduatedSymRenderer::writeXML(std::ofstream& xml)
{
    xml << "\t\t<graduatedsymbol>\n";
    xml << "\t\t\t<classificationfield>" + QString::number(this->classificationField()) +
	"</classificationfield>\n";
    for (std::list < QgsRangeRenderItem * >::iterator it = this->items().begin(); it != this->items().end();
	 ++it)
    {
	xml << "\t\t\t<rangerenderitem>\n";
	xml << "\t\t\t\t<lowervalue>" + (*it)->value() + "</lowervalue>\n";
	xml << "\t\t\t\t<uppervalue>" + (*it)->upper_value() + "</uppervalue>\n";
	xml << "\t\t\t\t<symbol>\n";
	QgsSymbol *symbol = (*it)->getSymbol();
	xml << "\t\t\t\t\t<outlinecolor red=\"" + QString::number(symbol->pen().color().red()) + "\" green=\"" +
	    QString::number(symbol->pen().color().green()) + "\" blue=\"" + QString::number(symbol->pen().color().blue()) +
	    "\" />\n";
	xml << "\t\t\t\t\t<outlinestyle>" + QgsSymbologyUtils::penStyle2QString(symbol->pen().style()) +
	    "</outlinestyle>\n";
	xml << "\t\t\t\t\t<outlinewidth>" + QString::number(symbol->pen().width()) + "</outlinewidth>\n";
	xml << "\t\t\t\t\t<fillcolor red=\"" + QString::number(symbol->brush().color().red()) + "\" green=\"" +
	    QString::number(symbol->brush().color().green()) + "\" blue=\"" +
	    QString::number(symbol->brush().color().blue()) + "\" />\n";
	xml << "\t\t\t\t\t<fillpattern>" + QgsSymbologyUtils::brushStyle2QString(symbol->brush().style()) +
	    "</fillpattern>\n";
	xml << "\t\t\t\t</symbol>\n";
	xml << "\t\t\t\t<label>" + (*it)->label() + "</label>\n";
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
