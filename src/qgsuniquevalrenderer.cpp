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

}

void QgsUniqueValRenderer::writeXML(std::ofstream& xml)
{

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
