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

class QgsSiMaRenderer: public QgsRenderer
{
 public:
    QgsSiMaRenderer();
    ~QgsSiMaRenderer();
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    void renderFeature(QPainter* p, QgsFeature* f);
    bool needsAttributes();
};

inline QgsSiMaRenderer::QgsSiMaRenderer()
{

}

inline QgsSiMaRenderer::~QgsSiMaRenderer()
{

}

inline bool QgsSiMaRenderer::needsAttributes()
{
    return false;
}

#endif
