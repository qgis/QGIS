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
    virtual ~QgsSiMaRenderer();
    /**Replaces the current mItem by ri*/
    void addItem(QgsRenderItem* ri);
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor, bool selected, int oversampling = 1, double widthScale = 1.);
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    virtual void writeXML(std::ostream& xml);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document );
    bool needsAttributes();
    /**Returns an empty list, since no classification attributes are used*/
    virtual std::list<int> classificationAttributes();
    QgsRenderItem* item();
    /**Returns the renderers name*/
    QString name();
    /**Return symbology items*/
    const std::list<QgsRenderItem*> items() const;
    /**Return symbology items*/
    const std::list<QgsSymbol*> symbols() const {}
 protected:
    QgsRenderItem* mItem;
};

inline QgsSiMaRenderer::QgsSiMaRenderer()
{
  //call superclass method to set up selection colour
  initialiseSelectionColor();

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
