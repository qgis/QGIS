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
#include <cfloat>
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgsgrasydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpixmap.h>
#include <qpicture.h>
//XXX Inlining this destructor kills build on WIN32 - sorry
QgsGraduatedSymRenderer::~QgsGraduatedSymRenderer()
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
	
	QgsRenderItem *item = *it;

	// Point 
	if ( pic && mVectorType == QGis::Point ) {
	    QPainter painter;
	    painter.begin(pic);

	    QPicture pic = item->getSymbol()->getPointSymbolAsPicture();
	    painter.drawPicture(0,0,pic);
	    if(selected) {
		painter.setBrush(QColor(255,255,0));
		QRect br = pic.boundingRect();
		painter.drawRect(-br.x(), -br.y(), br.width(), br.height());
	    }
	    
	    if ( scalefactor ) *scalefactor = 1;

	    painter.end();
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		p->setPen(item->getSymbol()->pen());
		p->setBrush(item->getSymbol()->brush());
	    }
	    else
	    {
		QPen pen=item->getSymbol()->pen();
		pen.setColor(mSelectionColor);
		QBrush brush=item->getSymbol()->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
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
        mVectorType = layer->vectorType();
	
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
    mVectorType = vl.vectorType();
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

	sy->readXML ( synode );

	QDomElement labelelement = rangerendernode.namedItem("label").toElement();
	QString label = labelelement.text();

	//create a renderitem and add it to the renderer

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

bool QgsGraduatedSymRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    bool returnval=true;
    QDomElement graduatedsymbol=document.createElement("graduatedsymbol");
    layer_node.appendChild(graduatedsymbol);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    graduatedsymbol.appendChild(classificationfield);
    for(std::list<QgsRangeRenderItem*>::iterator it=mItems.begin();it!=mItems.end();++it)
    {
	if(!(*it)->writeXML(graduatedsymbol,document))
	{
	    returnval=false;
	}
    }
    return returnval;
}

const std::list<QgsRenderItem*> QgsGraduatedSymRenderer::items() const
{
    std::list<QgsRenderItem*> list;
    for(std::list<QgsRangeRenderItem*>::const_iterator iter=mItems.begin();iter!=mItems.end();++iter)
    {
	list.push_back(*iter);
    }
    return list;
}
