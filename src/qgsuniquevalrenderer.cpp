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
#include <qdom.h>
#include <qpainter.h>
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
    
void QgsUniqueValRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected)
{
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    QString value = vec[0].fieldValue();
    std::map<QString,QgsRenderItem*>::iterator it=mEntries.find(value);
    if(it!=mEntries.end())
    {
	QgsRenderItem* ritem=it->second;
	p->setPen(ritem->getSymbol()->pen());
	p->setBrush(ritem->getSymbol()->brush());

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
#ifdef QGISDEBUG
	qWarning("readXML, value is "+value);
#endif
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

	QDomElement labelelement = renderitemnode.namedItem("label").toElement();
	QString label = labelelement.text();

	//create a renderitem and add it to the renderer
	msy->setBrush(brush);
	msy->setPen(pen);

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

void QgsUniqueValRenderer::writeXML(std::ostream& xml)
{
#ifdef QGISDEBUG
    qWarning("in QgsUniqueValRenderer::writeXML");
#endif
    xml << "\t\t<uniquevalue>\n";
    xml << "\t\t\t<classificationfield>" << QString::number(this->classificationField()) << "</classificationfield>\n";
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	xml << "\t\t\t<renderitem>\n";
	xml << "\t\t\t\t<value>" << QString(it->first) << "</value>\n";
	xml << "\t\t\t\t<symbol>\n";
	QgsSymbol *symbol = (it->second)->getSymbol();
	xml << "\t\t\t\t\t<outlinecolor red=\"" << QString::number(symbol->pen().color().red()) << "\" green=\"" <<
	    QString::number(symbol->pen().color().green()) << "\" blue=\"" << QString::number(symbol->pen().color().blue())  << 
	    "\" />\n";
	xml << "\t\t\t\t\t<outlinestyle>" << QgsSymbologyUtils::penStyle2QString(symbol->pen().style())  << 
	    "</outlinestyle>\n";
	xml << "\t\t\t\t\t<outlinewidth>" << QString::number(symbol->pen().width()) << "</outlinewidth>\n";
	xml << "\t\t\t\t\t<fillcolor red=\"" << QString::number(symbol->brush().color().red()) << "\" green=\""  << 
	    QString::number(symbol->brush().color().green()) << "\" blue=\""  << 
	    QString::number(symbol->brush().color().blue()) << "\" />\n";
	xml << "\t\t\t\t\t<fillpattern>" << QgsSymbologyUtils::brushStyle2QString(symbol->brush().style())  << 
	    "</fillpattern>\n";
	xml << "\t\t\t\t</symbol>\n";
	xml << "\t\t\t\t<label>" << (it->second)->label() << "</label>\n";
#ifdef QGISDEBUG
	qWarning((it->second)->label());
#endif
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
