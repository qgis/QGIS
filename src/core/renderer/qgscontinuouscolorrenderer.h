/***************************************************************************
                         qgscontinuouscolorrenderer.h  -  description
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
/* $Id: qgscontinuouscolorrenderer.h 5371 2006-04-25 01:52:13Z wonder $ */
#ifndef QGSCONTINUOUSCOLORRENDERER_H
#define QGSCONTINUOUSCOLORRENDERER_H

#include "qgsrenderer.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsfeature.h"

class QgsSymbol;
class QPainter;
class QImage;

/**Renderer class which interpolates rgb values linear between the minimum and maximum value of the classification field*/
class CORE_EXPORT QgsContinuousColorRenderer: public QgsRenderer
{
  public:
    QgsContinuousColorRenderer( QGis::GeometryType type );
    QgsContinuousColorRenderer( const QgsContinuousColorRenderer& other );
    QgsContinuousColorRenderer& operator=( const QgsContinuousColorRenderer& other );
    virtual ~QgsContinuousColorRenderer();

    /**Renders the feature using the minimum and maximum value of the classification field
     * added in 1.2 */
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected, double opacity = 1.0 );

    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Sets the id of the classification field*/
    void setClassificationField( int id );
    /**Sets the symbol for the minimum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMinimumSymbol( QgsSymbol* sy );
    /**Sets the symbol for the maximum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMaximumSymbol( QgsSymbol* sy );
    /** Sets whether to draw the polygon outline*/
    void setDrawPolygonOutline( bool draw ) { mDrawPolygonOutline = draw;}
    /**Returns the symbol for the minimum value*/
    const QgsSymbol* minimumSymbol() const;
    /**Returns the symbol for the maximum value*/
    const QgsSymbol* maximumSymbol() const;
    /** whether to draw a polygon outline*/
    bool drawPolygonOutline() const { return mDrawPolygonOutline; }
    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read
     @param vl the vector layer which will be associated with the renderer
     @return 0 in case of success, 1 if vector layer has no renderer, 2 if classification field not found
    */
    virtual int readXML( const QDomNode& rnode, QgsVectorLayer& vl );
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const;
    /** Returns true*/
    bool needsAttributes() const;
    /**Returns a list with the index of the classification attribute*/
    QgsAttributeList classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Return symbology items*/
    const QList<QgsSymbol*> symbols() const;
    QgsRenderer* clone() const;
  protected:
    /**Number of the classification field (it must be a numerical field)*/
    int mClassificationField;
    /**Item for the minimum value*/
    QgsSymbol* mMinimumSymbol;
    /**Item for the maximum value*/
    QgsSymbol* mMaximumSymbol;
    /** Whether to draw the polygon outline or not */
    bool mDrawPolygonOutline;
};

inline int QgsContinuousColorRenderer::classificationField() const
{
  return mClassificationField;
}

inline void QgsContinuousColorRenderer::setClassificationField( int id )
{
  mClassificationField = id;
}

inline const QgsSymbol* QgsContinuousColorRenderer::minimumSymbol() const
{
  return mMinimumSymbol;
}

inline const QgsSymbol* QgsContinuousColorRenderer::maximumSymbol() const
{
  return mMaximumSymbol;
}

inline bool QgsContinuousColorRenderer::needsAttributes() const
{
  return true;
}


#endif
