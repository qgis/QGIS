/***************************************************************************
                         qgsgraduatedsymrenderer.h  -  description
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
/* $Id */

#ifndef QGSGRADUATEDSYMRENDERER_H
#define QGSGRADUATEDSYMRENDERER_H

#include "qgsrenderer.h"
#include "qgsrangerenderitem.h"
#include <list>
#include <iostream>
#include "qgspoint.h"
#include "qpainter.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"

/**This class contains the information for graduate symbol rendering*/
class QgsGraduatedSymRenderer: public QgsRenderer
{
 public:
    QgsGraduatedSymRenderer();
    ~QgsGraduatedSymRenderer();
    /**Adds a new item
    \param ri a pointer to the QgsRangeRenderItem to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when the instance is destroyed*/
    void addItem(QgsRangeRenderItem* ri);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all items*/
    void removeItems();
    /**Renders an OGRFeature
     \param p a painter (usually the one from the current map canvas)
     \param f a pointer to a feature to render
     \param t the transform object containing the information how to transform the map coordinates to screen coordinates*/
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor);
    /**Sets the number of the classicifation field
    \param field the number of the field to classify*/
    void setClassificationField(int field);
    /**Sets the initial symbology configuration for a layer. Besides of applying default symbology settings, an instance of the corresponding renderer dialog is created and associated with the layer (or with the property dialog, if pr is not 0). Finally, a pixmap for the legend is drawn (or, if pr is not 0, it is stored in the property dialog, until the settings are applied).
       @param layer the vector layer associated with the renderer
       @param pr the property dialog. This is only needed if the renderer is created from the property dialog and not yet associated with the vector layer, otherwise 0*/
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    /**Returns the list with the render items*/
    std::list<QgsRangeRenderItem*>& items();
    /**Writes the contents of the renderer to a configuration file*/
    virtual void writeXML(std::ofstream& xml);
    /** Returns true*/
    bool needsAttributes();
 protected:
    /**Name of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**List holding the render items for the individual classes*/
    std::list<QgsRangeRenderItem*> mItems;
    
};

inline QgsGraduatedSymRenderer::QgsGraduatedSymRenderer()
{

}

inline void QgsGraduatedSymRenderer::addItem(QgsRangeRenderItem* ri)
{
    mItems.push_back(ri); 
}

inline int QgsGraduatedSymRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsGraduatedSymRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

inline std::list<QgsRangeRenderItem*>& QgsGraduatedSymRenderer::items()
{
    return mItems;
}

inline bool QgsGraduatedSymRenderer::needsAttributes()
{
    return true;
}


#endif
