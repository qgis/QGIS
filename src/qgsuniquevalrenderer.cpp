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

#include "qgsdlgvectorlayerproperties.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvaldialog.h"
#include "qgsrenderitem.h"
#include "qgsfeatureattribute.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbologyutils.h"
#include "qgsuvaldialog.h"
#include <qdom.h>
#include <qpainter.h>
#include <vector>

QgsUniqueValRenderer::QgsUniqueValRenderer(): mClassificationField(-1),mSelectionColor(QColor(255,255,0))
{

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
    QgsUValDialog *dialog = new QgsUValDialog(layer);

	if (pr)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
	    //layer->updateItemPixmap();
        }
}
    
void QgsUniqueValRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected)
{
#ifdef QGISDEBUG
    qWarning("in QgsUniqueValRenderer::renderFeature");
#endif

    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    QString value = vec[0].fieldValue();
#ifdef QGISDEBUG
    qWarning("Wert: "+value);
#endif
    std::map<QString,QgsRenderItem*>::iterator it=mEntries.find(value);
    if(it!=mEntries.end())
    {
	QgsRenderItem* ritem=it->second;
	p->setPen(ritem->getSymbol()->pen());
	p->setBrush(ritem->getSymbol()->brush());
#ifdef QGISDEBUG
	qWarning("outline color: "+QString::number(ritem->getSymbol()->pen().color().red())+"//"+QString::number(ritem->getSymbol()->pen().color().green())+"//"+QString::number(ritem->getSymbol()->pen().color().blue()));
	qWarning("fill color: "+QString::number(ritem->getSymbol()->brush().color().red())+"//"+QString::number(ritem->getSymbol()->brush().color().green())+"//"+QString::number(ritem->getSymbol()->brush().color().blue()));
	qWarning("outline style: "+QString::number((int)(ritem->getSymbol()->pen().style())));
#endif
	if(selected)
	{
	    QPen pen=ritem->getSymbol()->pen();
	    pen.setColor(mSelectionColor);
	    QBrush brush=ritem->getSymbol()->brush();
	    brush.setColor(mSelectionColor);
	    p->setPen(pen);
	    p->setBrush(brush);
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
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode renderitemnode = rnode.namedItem("renderitem");
    while (!renderitemnode.isNull())
    {
	QDomNode valuenode = renderitemnode.namedItem("value");
	QString value = valuenode.toElement().text();
	QgsSymbol* msy = new QgsSymbol();
	QPen pen;
	QBrush brush;
	QDomNode synode = renderitemnode.namedItem("symbol");
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

	//create a renderitem and add it to the renderer
	msy->setBrush(brush);
	msy->setPen(pen);

	QgsRenderItem *ri = new QgsRenderItem(msy, value, " ");
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

void QgsUniqueValRenderer::writeXML(std::ofstream& xml)
{
    xml << "\t\t<uniquevalue>\n";
    xml << "\t\t\t<classificationfield>" << QString::number(this->classificationField()) +
	"</classificationfield>\n";
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	xml << "\t\t\t<renderitem>\n";
	xml << "\t\t\t\t<value>" << it->first << "</value>\n";
	xml << "\t\t\t\t<symbol>\n";
	QgsSymbol *symbol = (it->second)->getSymbol();
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
	xml << "\t\t\t</renderitem>\n";
    }
    xml << "\t\t</uniquevalue>\n";
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
