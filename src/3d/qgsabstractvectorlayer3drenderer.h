/***************************************************************************
  qgsabstractvectorlayer3drenderer.h
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTVECTORLAYER3DRENDERER_H
#define QGSABSTRACTVECTORLAYER3DRENDERER_H

#include "qgis_3d.h"

#include "qgsabstract3drenderer.h"
#include "qgsmaplayerref.h"

class QgsVectorLayer;


/**
 * \ingroup 3d
 * Base class for 3D renderers that are based on vector layers.
 *
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsAbstractVectorLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    QgsAbstractVectorLayer3DRenderer();

    //! Sets vector layer associated with the renderer
    void setLayer( QgsVectorLayer *layer );
    //! Returns vector layer associated with the renderer
    QgsVectorLayer *layer() const;

    void resolveReferences( const QgsProject &project ) override;

  protected:
    //! copy common properties of this object to another object
    void copyBaseProperties( QgsAbstractVectorLayer3DRenderer *r ) const;
    //! write common properties of this object to DOM element
    void writeXmlBaseProperties( QDomElement &elem, const QgsReadWriteContext &context ) const;
    //! read common properties of this object from DOM element
    void readXmlBaseProperties( const QDomElement &elem, const QgsReadWriteContext &context );

  private:
    QgsMapLayerRef mLayerRef; //!< Layer used to extract polygons from
};

#endif // QGSABSTRACTVECTORLAYER3DRENDERER_H
