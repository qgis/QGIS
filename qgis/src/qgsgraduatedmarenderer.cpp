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
#include "qgsvectorlayer.h"
#include <iostream.h>
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
    
void QgsGraduatedMaRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor)
{
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::NoBrush);

    //first find out the value for the classification attribute
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    double value = vec[mClassificationField].fieldValue().toDouble();

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
	QgsMarkerSymbol* ms=dynamic_cast<QgsMarkerSymbol*>((*it)->getSymbol());
	if(ms&&pic)
	{
	    pic->load(ms->picture(),"svg");
	    (*scalefactor)=ms->scaleFactor();
	}
    }
}

void QgsGraduatedMaRenderer::writeXML(std::ofstream& xml)
{
    //soon
}
