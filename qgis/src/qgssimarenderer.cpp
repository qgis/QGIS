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
#include "qgsdlgvectorlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsmarkersymbol.h"
#include <qpainter.h>

void QgsSiMaRenderer::initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr)
{
    bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
    if (pr)
    {
	toproperties = true;
    }
    
    if (layer)
    {
	QgsMarkerSymbol sy;
	sy.brush().setStyle(Qt::NoBrush);
	sy.pen().setStyle(Qt::NoPen);
	sy.pen().setWidth(1);//set width 1 as default instead of width 0

	QgsRenderItem ri(sy, "", "");
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

void QgsSiMaRenderer::renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor)
{
    p->setPen(mItem.getSymbol()->pen());
    p->setBrush(mItem.getSymbol()->brush());

    QgsSymbol* testsymbol=mItem.getSymbol();
    QgsMarkerSymbol* ms=dynamic_cast<QgsMarkerSymbol*>(mItem.getSymbol());
    if(ms)
    {
	pic=ms->picture();
	(*scalefactor)=ms->scaleFactor();//does not work, but why?
    }
}
