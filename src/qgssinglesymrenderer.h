/***************************************************************************
                         qgssinglesymrenderer.h  -  description
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

#ifndef QGSSINGLESYMRENDERER_H
#define QGSSINGLESYMRENDERER_H

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include "qgspoint.h"
#include "qpainter.h"
#include "qgscoordinatetransform.h"
class QgsFeature;

/**Render class to display all the features with a single QgsSymbol*/
class QgsSingleSymRenderer: public QgsRenderer
{
 public:
    QgsSingleSymRenderer();
    ~QgsSingleSymRenderer();
    /**Replaces the current m_item by ri*/
    void addItem(QgsRenderItem ri);
    /**Returns a pointer to m_item*/
    QgsRenderItem* item();
    /**Renders an OGRFeature*/
    void renderFeature(QPainter* p, QgsFeature* f, QgsCoordinateTransform* t);
    /**Sets the initial symbology configuration for a layer. An instance of the corresponding renderer dialog is created and associated with the layer. Finally, a pixmap for the legend is drawn
     @param layer the vector layer associated with the renderer*/
    virtual void initializeSymbology(QgsVectorLayer* layer);
    /**Returns false, no attributes neede for single symbol*/
    bool needsAttributes();
 protected:
    QgsRenderItem m_item;
};

inline QgsRenderItem* QgsSingleSymRenderer::item()
{
    return &m_item;
}
inline bool QgsSingleSymRenderer::needsAttributes(){
  return false;
}
#endif
