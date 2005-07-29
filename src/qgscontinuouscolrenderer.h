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
#include <qpainter.h>
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include <iostream>
#include "qgsdlgvectorlayerproperties.h"

class QgsSymbol;

/**Renderer class which interpolates rgb values linear between the minimum and maximum value of the classification field*/
class QgsContinuousColRenderer: public QgsRenderer
{
 public:
    QgsContinuousColRenderer(QGis::VectorType type);
    QgsContinuousColRenderer(const QgsContinuousColRenderer& other);
    QgsContinuousColRenderer& operator=(const QgsContinuousColRenderer& other);
    virtual ~QgsContinuousColRenderer();
    /**Renders the feature using the minimum and maximum value of the classification field*/
    void renderFeature(QPainter* p, QgsFeature* f, QPicture* pic, double* scalefactor, bool selected, int oversampling = 1, double widthScale = 1.);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Sets the id of the classification field*/
    void setClassificationField(int id);
    /**Sets the symbol for the minimum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMinimumSymbol(QgsSymbol* sy);
    /**Sets the symbol for the maximum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMaximumSymbol(QgsSymbol* sy);
    /**Returns the symbol for the minimum value*/
    const QgsSymbol* minimumSymbol() const;
    /**Returns the symbol for the maximum value*/
    const QgsSymbol* maximumSymbol() const;
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document ) const;
    /** Returns true*/
    bool needsAttributes() const;
    /**Returns a list with the index of the classification attribute*/
    virtual std::list<int> classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Return symbology items*/
    const std::list<QgsSymbol*> symbols() const;
    QgsRenderer* clone() const;
 protected:
    /**Number of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**Item for the minimum value*/
    QgsSymbol* mMinimumSymbol;
    /**Item for the maximum value*/
    QgsSymbol* mMaximumSymbol;
};

inline int QgsContinuousColRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsContinuousColRenderer::setClassificationField(int id)
{
    mClassificationField=id;
}

inline const QgsSymbol* QgsContinuousColRenderer::minimumSymbol() const
{
    return mMinimumSymbol;
}

inline const QgsSymbol* QgsContinuousColRenderer::maximumSymbol() const
{
    return mMaximumSymbol;
}

inline bool QgsContinuousColRenderer::needsAttributes() const
{
  return true;
}


#endif
