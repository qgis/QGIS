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
#include "qgssinglesymrenderer.h"
#include "qgsfeature.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qstring.h"
#include "qgssisydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include <qdom.h>

QgsSingleSymRenderer::QgsSingleSymRenderer(): mItem(new QgsRenderItem())
{
  //call superclass method to set up selection colour
  initialiseSelectionColor();

}

QgsSingleSymRenderer::~QgsSingleSymRenderer()
{
    delete mItem;
}

void QgsSingleSymRenderer::addItem(QgsRenderItem* ri)
{
    delete mItem;
    mItem = ri;
}

void QgsSingleSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, double* scalefactor, bool selected)
{
	p->setPen(mItem->getSymbol()->pen());
	p->setBrush(mItem->getSymbol()->brush());
	if(selected)
	{
	    QPen pen=mItem->getSymbol()->pen();
	    pen.setColor(mSelectionColor);
	    QBrush brush=mItem->getSymbol()->brush();
	    brush.setColor(mSelectionColor);
	    p->setPen(pen);
	    p->setBrush(brush);
	}
}

void QgsSingleSymRenderer::initializeSymbology(QgsVectorLayer * layer, QgsDlgVectorLayerProperties * pr)
{
    bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if (pr)
    {
	toproperties = true;
    }

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
	QFont f("arial", 10, QFont::Normal);
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
	p.setPen(sy->pen());
	
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
	QgsRenderItem* ri = new QgsRenderItem(sy, "", "");
	addItem(ri);

	QgsSiSyDialog *dialog = new QgsSiSyDialog(layer);
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
	qWarning("Warning, null pointer in QgsSingleSymRenderer::initializeSymbology()");
    }
}

void QgsSingleSymRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    QgsSymbol* sy = new QgsSymbol();
    QPen pen;
    QBrush brush;

    QDomNode rinode = rnode.namedItem("renderitem");

    QDomNode vnode = rinode.namedItem("value");
    QDomElement velement = vnode.toElement();
    QString value = velement.text();

    QDomNode synode = rinode.namedItem("symbol");

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
    QString label = lnodee.text();

    //create a renderer and add it to the vector layer
    sy->setBrush(brush);
    sy->setPen(pen);
    QgsRenderItem* ri = new QgsRenderItem(sy, value, label);
    this->addItem(ri);
    vl.setRenderer(this);
    QgsSiSyDialog *sdialog = new QgsSiSyDialog(&vl);
    vl.setRendererDialog(sdialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Single Symbol");

    sdialog->apply();
}

void QgsSingleSymRenderer::writeXML(std::ostream& xml)
{
    xml << "\t\t<singlesymbol>\n";
    xml << "\t\t\t<renderitem>\n";
    xml << "\t\t\t\t<value>" << this->item()->value() << "</value>\n";
    QgsSymbol *symbol = this->item()->getSymbol();

    xml << "\t\t\t\t<symbol>\n";
    xml << "\t\t\t\t\t<outlinecolor red=\"" 
      << symbol->pen().color().red()
      << "\" green=\"" 
      << QString::number(symbol->pen().color().green()) 
      << "\" blue=\"" 
      << QString::number(symbol->pen().color().blue()) 
      << "\" />\n";
    xml << "\t\t\t\t\t<outlinestyle>" << (const char *)QgsSymbologyUtils::penStyle2QString(symbol->pen().style()) << "</outlinestyle>\n";
    xml << "\t\t\t\t\t<outlinewidth>" << symbol->pen().width() << "</outlinewidth>\n";
    xml << "\t\t\t\t\t<fillcolor red=\"" <<  symbol->brush().color().red() 
      << "\" green=\"" 
      << symbol->brush().color().green() 
      << "\" blue=\"" 
      << symbol->brush().color().blue() 
      << "\" />\n";
    xml << "\t\t\t\t\t<fillpattern>" 
      << (const char *)QgsSymbologyUtils::brushStyle2QString(symbol->brush().style()) 
      << "</fillpattern>\n";
    xml << "\t\t\t\t</symbol>\n";
    //xml << "\t\t\t\t<label>" << this->item()->label().latin1() << "</label>\n";
    xml << "\t\t\t</renderitem>\n";
    xml << "\t\t</singlesymbol>\n";
}

bool QgsSingleSymRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    bool returnval=false;
    QDomElement singlesymbol=document.createElement("singlesymbol");
    layer_node.appendChild(singlesymbol);
    if(mItem)
    {
	returnval=mItem->writeXML(singlesymbol,document);
    }
    return returnval;
}


std::list<int> QgsSingleSymRenderer::classificationAttributes()
{
    std::list<int> list;
    return list;//return an empty list
}

QString QgsSingleSymRenderer::name()
{
    return "Single Symbol";
}

const std::list<QgsRenderItem*> QgsSingleSymRenderer::items() const
{
    std::list<QgsRenderItem*> list;
    list.push_back(mItem);
    return list;
}
