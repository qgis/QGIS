/***************************************************************************
                         qgsgraduatedsymbolrenderer.h  -  description
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
/* $Id: qgsgraduatedsymbolrenderer.h 5371 2006-04-25 01:52:13Z wonder $ */

#ifndef QGSGRADUATEDSYMBOLRENDERER_H
#define QGSGRADUATEDSYMBOLRENDERER_H

#include "qgsrenderer.h"

class QgsVectorLayer;

/**This class contains the information for graduate symbol rendering*/
class CORE_EXPORT QgsGraduatedSymbolRenderer: public QgsRenderer
{
 public:
    QgsGraduatedSymbolRenderer(QGis::VectorType type);
    QgsGraduatedSymbolRenderer(const QgsGraduatedSymbolRenderer& other);
    QgsGraduatedSymbolRenderer& operator=(const QgsGraduatedSymbolRenderer& other);
    virtual ~QgsGraduatedSymbolRenderer();
    /**Adds a new item
    \param sy a pointer to the QgsSymbol to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when this object is destroyed*/
    void addSymbol(QgsSymbol* sy);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all symbols*/
    void removeSymbols();
    /** Determines if a feature will be rendered or not 
	@param f a pointer to the feature to determine if rendering will happen*/
    bool willRenderFeature(QgsFeature *f);
    /**Renders an OGRFeature
     \param p a painter (usually the one from the current map canvas)
     \param f a pointer to a feature to render
     \param t the transform object containing the information how to transform the map coordinates to screen coordinates*/
    void renderFeature(QPainter* p, QgsFeature& f, QImage* img, double* scalefactor, bool selected, double widthScale = 1.);
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
    /**Returns a list of all needed attributes*/
    QgsAttributeList classificationAttributes() const;
    void updateSymbolAttributes();
    /**Returns the renderers name*/
    QString name() const;
    /**Returns the symbols of the items*/
    const QList<QgsSymbol*> symbols() const;
    /**Returns a copy of the renderer (a deep copy on the heap)*/
    QgsRenderer* clone() const;
 protected:
    /**Index of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**List holding the symbols for the individual classes*/
    QList<QgsSymbol*> mSymbols;
    QgsSymbol* symbolForFeature(const QgsFeature* f);
    /**Cached copy of all underlying symbols required attribute fields*/
    QgsAttributeList mSymbolAttributes;
    
};

inline void QgsGraduatedSymbolRenderer::addSymbol(QgsSymbol* sy)
{
    mSymbols.push_back(sy);
}

inline int QgsGraduatedSymbolRenderer::classificationField() const
{
    return mClassificationField;
}

inline void QgsGraduatedSymbolRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

inline bool QgsGraduatedSymbolRenderer::needsAttributes() const
{
    return true;
}


#endif
