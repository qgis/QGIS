/***************************************************************************
                         qgsgraduatedmarenderer.h  -  description
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

#ifndef QGSGRADUATEDMARENDERER_H
#define QGSGRADUATEDMARENDERER_H

#include "qgsrenderer.h"
#include "qgsrangerenderitem.h"
#include <list>

class QgsGraduatedMaRenderer: public QgsRenderer
{
 public:
    QgsGraduatedMaRenderer();
    ~QgsGraduatedMaRenderer();
    /**Adds a new item
    \param ri a pointer to the QgsRangeRenderItem to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when the instance is destroyed*/
    void addItem(QgsRangeRenderItem* ri);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all items*/
    void removeItems();
    virtual void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    virtual void renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected);
    /**Sets the number of the classicifation field
    \param field the number of the field to classify*/
    void setClassificationField(int field);
    /**Returns the list with the render items*/
    std::list<QgsRangeRenderItem*>& items();
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    virtual void writeXML(std::ofstream& xml);
    virtual bool needsAttributes();
    /**Returns a list with the index of the classification attribute*/
    virtual std::list<int> classificationAttributes();
    /**Returns the renderers name*/
    QString name();
 protected:
    /**Name of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**List holding the render items for the individual classes*/
    std::list<QgsRangeRenderItem*> mItems;
};

inline QgsGraduatedMaRenderer::QgsGraduatedMaRenderer()
{

}

inline void QgsGraduatedMaRenderer::addItem(QgsRangeRenderItem* ri)
{
    mItems.push_back(ri); 
}

inline int QgsGraduatedMaRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsGraduatedMaRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

inline std::list<QgsRangeRenderItem*>& QgsGraduatedMaRenderer::items()
{
    return mItems;
}

inline bool QgsGraduatedMaRenderer::needsAttributes()
{
    return true;
}

#endif
