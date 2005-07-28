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
/* $Id$ */

#ifndef QGSGRADUATEDSYMRENDERER_H
#define QGSGRADUATEDSYMRENDERER_H

#include "qgsrenderer.h"
#include "qgsrangerenderitem.h"
#include <list>
#include <iostream>
#include "qgspoint.h"
#include "qpainter.h"
#include "qgsmaptopixel.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"

/**This class contains the information for graduate symbol rendering*/
class QgsGraduatedSymRenderer: public QgsRenderer
{
 public:
    QgsGraduatedSymRenderer(QGis::VectorType type);
    QgsGraduatedSymRenderer(const QgsGraduatedSymRenderer& other);
    QgsGraduatedSymRenderer& operator=(const QgsGraduatedSymRenderer& other);
    virtual ~QgsGraduatedSymRenderer();
    /**Adds a new item
    \param sy a pointer to the QgsSymbol to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when this object is destroyed*/
    void addSymbol(QgsSymbol* sy);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all symbols*/
    void removeSymbols();
    /**Renders an OGRFeature
     \param p a painter (usually the one from the current map canvas)
     \param f a pointer to a feature to render
     \param t the transform object containing the information how to transform the map coordinates to screen coordinates*/
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor, bool selected, int oversampling = 1, double widthScale = 1.);
    /**Sets the number of the classicifation field
    \param field the number of the field to classify*/
    void setClassificationField(int field);
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document ) const;
    /** Returns true*/
    bool needsAttributes() const;
    /**Returns a list with the index to the classification field*/
    virtual std::list<int> classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Returns the symbols of the items*/
    const std::list<QgsSymbol*> symbols() const;
    QgsRenderer* clone() const;
 protected:
    /**Name of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**List holding the symbols for the individual classes*/
    std::list<QgsSymbol*> mSymbols;
    
};

inline void QgsGraduatedSymRenderer::addSymbol(QgsSymbol* sy)
{
    mSymbols.push_back(sy);
}

inline int QgsGraduatedSymRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsGraduatedSymRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

inline bool QgsGraduatedSymRenderer::needsAttributes() const
{
    return true;
}


#endif
