/***************************************************************************
                         qgsuvalmarenderer.cpp  - unique value marker renderer
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

#include "qgsdlgvectorlayerproperties.h"
#include "qgsuvalmarenderer.h"
#include "qgsuvalmadialog.h"
#include "qgsrenderitem.h"
#include "qgsfeatureattribute.h"
#include "qgsmarkersymbol.h"
#include "qgssvgcache.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbologyutils.h"
#include "qgsuvalmadialog.h"
#include <qdom.h>
#include <qpainter.h>
#include <vector>

QgsUValMaRenderer::QgsUValMaRenderer(): mClassificationField(0),mSelectionColor(QColor(255,255,0))
{

}

QgsUValMaRenderer::~QgsUValMaRenderer()
{
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	delete it->second;
    }
}

void QgsUValMaRenderer::initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr)
{
    QgsUValMaDialog *dialog = new QgsUValMaDialog(layer);

	if (pr)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
        }
}
    
void QgsUValMaRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected)
{
#ifdef QGISDEBUG
    qWarning("in QgsUValMaRenderer::renderFeature");
#endif

    p->setPen(Qt::NoPen);
    p->setBrush(Qt::NoBrush);

    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    QString value = vec[0].fieldValue();
#ifdef QGISDEBUG
    qWarning("Wert: "+value);
#endif
    std::map<QString,QgsRenderItem*>::iterator it=mEntries.find(value);
    if(it!=mEntries.end())
    {
      QgsRenderItem* ritem=it->second;
      QgsMarkerSymbol* ms =static_cast<QgsMarkerSymbol*>(ritem->getSymbol());
      
      QPainter painter(pic);
      QPixmap pm = QgsSVGCache::instance().getPixmap(ms->picture(), 
						     ms->scaleFactor());
      painter.drawPixmap(0, 0, pm);
      (*scalefactor) = 1;
      
      if(selected) {
	painter.setBrush(QColor(255,255,0));
	painter.drawRect(0,0,pm.width(),pm.height());
      }
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("Warning, no render item found in QgsUValMaRenderer::renderFeature");
#endif
    }
    
}

void QgsUValMaRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
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
	QgsMarkerSymbol* msy = new QgsMarkerSymbol();
	QDomNode synode = renderitemnode.namedItem("markersymbol");
	QString svgpath = "", scalefactor = "";
	QDomNode svgnode = synode.namedItem("svgpath");
	svgpath = svgnode.toElement().text();
	QDomNode scalenode = synode.namedItem("scalefactor");
	scalefactor = scalenode.toElement().text();

	//create a renderitem and add it to the renderer
	msy->setPicture(svgpath);
	msy->setScaleFactor(scalefactor.toDouble());

	QgsRenderItem *ri = new QgsRenderItem(msy, value, " ");
	this->insertValue(value,ri);

	renderitemnode = renderitemnode.nextSibling();
    }

    vl.setRenderer(this);
    QgsUValMaDialog *uvalmadialog = new QgsUValMaDialog(&vl);
    vl.setRendererDialog(uvalmadialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Unique Value Marker");

    uvalmadialog->apply();
}

void QgsUValMaRenderer::writeXML(std::ofstream& xml)
{
    xml << "\t\t<uniquevaluemarker>\n";
    xml << "\t\t\t<classificationfield>" << QString::number(this->classificationField()) +
	"</classificationfield>\n";
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	xml << "\t\t\t<renderitem>\n";
	xml << "\t\t\t\t<value>" << it->first << "</value>\n";
	xml << "\t\t\t\t<markersymbol>\n";
	QgsMarkerSymbol *symbol = 
	  dynamic_cast<QgsMarkerSymbol*>((it->second)->getSymbol());
	xml << "\t\t\t\t\t<svgpath>" << symbol->picture() << "</svgpath>\n";
	xml << "\t\t\t\t\t<scalefactor>" << symbol->scaleFactor() 
	    << "</scalefactor>\n";
	xml << "\t\t\t\t</markersymbol>\n";
	xml << "\t\t\t</renderitem>\n";
    }
    xml << "\t\t</uniquevaluemarker>\n";
}

void QgsUValMaRenderer::clearValues()
{
    for(std::map<QString,QgsRenderItem*>::iterator it=mEntries.begin();it!=mEntries.end();++it)
    {
	delete it->second;
    }
    mEntries.clear();
}

QString QgsUValMaRenderer::name()
{
    return "Unique Value";
}

std::list<int> QgsUValMaRenderer::classificationAttributes()
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

std::map<QString,QgsRenderItem*>& QgsUValMaRenderer::items()
{
    return mEntries;
}
