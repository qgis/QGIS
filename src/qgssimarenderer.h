/***************************************************************************
                          qgssimarenderer.h 
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

#ifndef QGSSIMARENDERER_H
#define QGSSIMARENDERER_H

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include "qgsmarkersymbol.h"

class QgsSiMaRenderer: public QgsRenderer
{
 public:
    QgsSiMaRenderer();
    ~QgsSiMaRenderer();
    /**Replaces the current mItem by ri*/
    void addItem(QgsRenderItem* ri);
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor);
    bool needsAttributes();
    QgsRenderItem* item();
 protected:
    QgsRenderItem* mItem;
};

inline QgsSiMaRenderer::QgsSiMaRenderer()//: mItem(new QgsRenderItem())
{
    mItem=new QgsRenderItem(new QgsMarkerSymbol(),"","");
}

inline QgsSiMaRenderer::~QgsSiMaRenderer()
{
    delete mItem;
}

inline bool QgsSiMaRenderer::needsAttributes()
{
    return false;
}

inline void QgsSiMaRenderer::addItem(QgsRenderItem* ri)
{
    delete mItem;
    mItem=ri;
}

inline QgsRenderItem* QgsSiMaRenderer::item()
{
    return mItem;
}

#endif
