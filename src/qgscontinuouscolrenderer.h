/***************************************************************************
                         qgscontinuouscolrenderer.h  -  description
                             -------------------
    begin                : Nov 2003
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
#ifndef QGSCONTINUOUSCOLRENDERER_H
#define QGSCONTINUOUSCOLRENDERER_H

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include <qpainter.h>
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include <iostream>
#include "qgsdlgvectorlayerproperties.h"


/**Renderer class which interpolates rgb values linear between the minimum and maximum value of the classification field*/
class QgsContinuousColRenderer: public QgsRenderer
{
 public:
    QgsContinuousColRenderer();
    virtual ~QgsContinuousColRenderer();
    /**Sets the initial symbology configuration for a layer. Besides of applying default symbology settings, an instance of the corresponding renderer dialog is created and associated with the layer (or with the property dialog, if pr is not 0). Finally, a pixmap for the legend is drawn (or, if pr is not 0, it is stored in the property dialog, until the settings are applied).
       @param layer the vector layer associated with the renderer
       @param pr the property dialog. This is only needed if the renderer is created from the property dialog and not yet associated with the vector layer, otherwise 0*/
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    /**Renders the feature using the minimum and maximum value of the classification field*/
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor, bool selected);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Sets the id of the classification field*/
    void setClassificationField(int id);
    /**Sets the item for the minimum value. The item has to be created using the new operator and is automatically deleted when inserting a new item or when the instance is destroyed*/
    void setMinimumItem(QgsRenderItem* it);
    /**Sets the item for the maximum value. The item has to be created using the new operator and is automatically deleted when inserting a new item or when the instance is destroyed*/
    void setMaximumItem(QgsRenderItem* it);
    /**Returns the item for the minimum value*/
    QgsRenderItem* minimumItem();
    /**Returns the item for the maximum value*/
    QgsRenderItem* maximumItem();
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    virtual void writeXML(std::ostream& xml);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document );
    /** Returns true*/
    bool needsAttributes();
    /**Returns a list with the index of the classification attribute*/
    virtual std::list<int> classificationAttributes();
    /**Returns the renderers name*/
    QString name();
    /**Return symbology items*/
    const std::list<QgsRenderItem*> items() const;
 protected:
    /**Number of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**Item for the minimum value*/
    QgsRenderItem* mMinimumItem;
    /**Item for the maximum value*/
    QgsRenderItem* mMaximumItem;
};

inline QgsContinuousColRenderer::QgsContinuousColRenderer(): mMinimumItem(0), mMaximumItem(0)
{
  //call superclass method to set up selection colour
  initialiseSelectionColor();

}

inline int QgsContinuousColRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsContinuousColRenderer::setClassificationField(int id)
{
    mClassificationField=id;
}

inline QgsRenderItem* QgsContinuousColRenderer::minimumItem()
{
    return mMinimumItem;
}

inline QgsRenderItem* QgsContinuousColRenderer::maximumItem()
{
    return mMaximumItem;
}

inline bool QgsContinuousColRenderer::needsAttributes()
{
  return true;
}


#endif
