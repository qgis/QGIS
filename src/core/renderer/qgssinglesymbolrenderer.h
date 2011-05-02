/***************************************************************************
                         qgssinglesymbolrenderer.h  -  description
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
/* $Id: qgssinglesymbolrenderer.h 5371 2006-04-25 01:52:13Z wonder $ */

#ifndef QGSSINGLESYMBOLRENDERER_H
#define QGSSINGLESYMBOLRENDERER_H

#include <QMap>
#include "qgsrenderer.h"
#include "qgsrendercontext.h"


/**Render class to display all the features with a single QgsSymbol*/
class CORE_EXPORT QgsSingleSymbolRenderer: public QgsRenderer
{
  public:
    QgsSingleSymbolRenderer( QGis::GeometryType type );
    QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer& other );
    QgsSingleSymbolRenderer& operator=( const QgsSingleSymbolRenderer& other );
    virtual ~QgsSingleSymbolRenderer();

    /**Replaces the current mSymbol by sy*/
    void addSymbol( QgsSymbol* sy );
    /*Returns a pointer to mSymbol*/
    const QgsSymbol* symbol() const;

    /**Renders a feature
     * added in 1.2 */
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected, double opacity = 1.0 );

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read
     @param vl the vector layer which will be associated with the renderer
     @return 0 in case of success, 1 if vector layer has no renderer, 2 if classification field not found
    */
    virtual int readXML( const QDomNode& rnode, QgsVectorLayer& vl );
    /**Writes the contents of the renderer to a configuration file*/
    /*virtual void writeXML(std::ostream& xml);*/
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const;
    /**Returns true, attributes needed for single symbol*/
    bool needsAttributes() const;
    /**Returns a list of all needed attributes*/
    QgsAttributeList classificationAttributes() const;
    void updateSymbolAttributes();
    /**Returns the renderers name*/
    virtual QString name() const;
    /**Returns a list containing mSymbol*/
    const QList<QgsSymbol*> symbols() const;
    /**Returns a deep copy of this renderer*/
    QgsRenderer* clone() const;

    /**Returns renderer symbol for a feature
        @note: this method was added in version 1.6*/
    QgsSymbol* symbolForFeature( const QgsFeature* f ) { return mSymbol0; }

  protected:
    /**Object containing symbology information*/
    QgsSymbol *mSymbol0;
    QMap<QString, QgsSymbol*> mSymbols;
    /**Cached copy of all underlying symbols required attribute fields*/
    QgsAttributeList mSymbolAttributes;
};

inline const QgsSymbol* QgsSingleSymbolRenderer::symbol() const
{
  return mSymbol0;
}

inline bool QgsSingleSymbolRenderer::needsAttributes() const
{
  return true;
}

#endif
