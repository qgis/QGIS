/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRENDERER_H
#define QGSRENDERER_H

class QgsFeature;
class QgsVectorLayer;
class QPainter;
class QImage;
class QDomDocument;
class QColor;

#include "qgis.h"
#include "qgsrendercontext.h"
#include <QList>
#include <QDomNode>

class QgsSymbol;
class QBrush;

typedef QList<int> QgsAttributeList;


/**Abstract base class for renderers. A renderer holds all the information necessary to draw the contents of a vector layer to a map canvas. The vector layer then passes each feature to paint to the renderer*/
class CORE_EXPORT QgsRenderer
{
  public:
    /** Default ctor sets up selection color from project properties */
    QgsRenderer();
    /** Virtual destructor because we have virtual methods... */
    virtual ~QgsRenderer();
    /** Determines if a feature will be rendered or not
    @param f a pointer to the feature to determine if rendering will happen*/
    virtual bool willRenderFeature( QgsFeature *f )
    {
      //prevent unused var warnings
      if ( !f )
      {
        return true;
      }
      return true;
    }
    /**A vector layer passes features to a renderer object to change the brush and pen of the qpainter
     @param p the painter storing brush and pen
     @param f a pointer to the feature to be rendered
     @param img a pointer to picture
     @param selected feature is selected
     @param widthScale scale
     @param rasterScaleFactor raster scale
     @note deprecated */
    void renderFeature( QPainter* p, QgsFeature& f, QImage* img, bool selected, double widthScale = 1.0, double rasterScaleFactor = 1.0 )
    {
      QgsRenderContext r;
      r.setPainter( p );
      r.setScaleFactor( widthScale );
      r.setRasterScaleFactor( rasterScaleFactor );
      renderFeature( r, f, img, selected );
    }

    /**A vector layer passes features to a renderer object to change the brush and pen of the qpainter
      @note added in 1.2 */
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected )
    {
      renderFeature( renderContext, f, pic, selected, 1.0 );
    }

    virtual void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity ) = 0;

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read
     @param vl the vector layer which will be associated with the renderer
     @return 0 in case of success, 1 if vector layer has no renderer, 2 if classification field not found*/
    virtual int readXML( const QDomNode& rnode, QgsVectorLayer& vl ) = 0;

    /**Writes the contents of the renderer to a configuration file*/
    // virtual void writeXML(std::ostream& xml)=0;
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const = 0;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    virtual bool needsAttributes() const = 0;
    /**Returns a list with indexes of classification attributes*/
    virtual QgsAttributeList classificationAttributes() const = 0;
    /**Returns the renderers name*/
    virtual QString name() const = 0;
    /**Return symbology items*/
    virtual const QList<QgsSymbol*> symbols() const = 0;
    /**Returns a copy of the renderer (a deep copy on the heap)*/
    virtual QgsRenderer* clone() const = 0;
    /** Change selection color */
    static void setSelectionColor( QColor color );
    /** Get selection color. Added in QGIS v1.4 */
    static QColor selectionColor();
    /**Returns true if this renderer returns a pixmap in the render method (e.g. for point data or diagrams)*/
    virtual bool containsPixmap() const;
    /**Returns true if this renderer uses its own transparency settings, e.g. differentiated by classification.
     This is a hint for QgsVectorLayer to not use the transparency setting on layer level in this cases*/
    virtual bool usesTransparency() const {return false;}

    /**Returns renderer symbol for a feature.
        @note: this method was added in version 1.6*/
    virtual QgsSymbol* symbolForFeature( const QgsFeature* f ) { Q_UNUSED( f ); return 0;}

    /**Scales a brush to a given raster scale factor (e.g. for printing)*/
    static void scaleBrush( QBrush& b, double rasterScaleFactor );

  protected:
    /**Color to draw selected features - static so we can change it in proj props and automatically
     all renderers are updated*/
    static QColor mSelectionColor;

    /**Layer type*/
    QGis::GeometryType mGeometryType;
};

#endif // QGSRENDERER_H
