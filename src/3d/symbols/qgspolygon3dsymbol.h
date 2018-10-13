/***************************************************************************
  qgspolygon3dsymbol.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOLYGON3DSYMBOL_H
#define QGSPOLYGON3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"

#include <Qt3DRender/QCullFace>

/**
 * \ingroup 3d
 * 3D symbol that draws polygon geometries as planar polygons, optionally extruded (with added walls).
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPolygon3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsPolygon3DSymbol
    QgsPolygon3DSymbol() = default;

    QString type() const override { return "polygon"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns method that determines how altitude is bound to individual vertices
    AltitudeBinding altitudeBinding() const { return mAltBinding; }
    //! Sets method that determines how altitude is bound to individual vertices
    void setAltitudeBinding( AltitudeBinding altBinding ) { mAltBinding = altBinding; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns extrusion height (in map units)
    float extrusionHeight() const { return mExtrusionHeight; }
    //! Sets extrusion height (in map units)
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    //! Returns material used for shading of the symbol
    QgsPhongMaterialSettings material() const { return mMaterial; }
    //! Sets material used for shading of the symbol
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

    //! Returns front/back culling mode
    Qt3DRender::QCullFace::CullingMode cullingMode() const { return mCullingMode; } SIP_SKIP
    //! Sets front/back culling mode
    void setCullingMode( Qt3DRender::QCullFace::CullingMode mode ) { mCullingMode = mode; } SIP_SKIP

    //! Returns whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    bool invertNormals() const { return mInvertNormals; }
    //! Sets whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    void setInvertNormals( bool invert ) { mInvertNormals = invert; }

    /**
     * Returns whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    bool addBackFaces() const { return mAddBackFaces; }

    /**
     * Sets whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    void setAddBackFaces( bool add ) { mAddBackFaces = add; }

  private:
    //! how to handle altitude of vector features
    AltitudeClamping mAltClamping = AltClampRelative;
    //! how to handle clamping of vertices of individual features
    AltitudeBinding mAltBinding = AltBindCentroid;

    float mHeight = 0.0f;           //!< Base height of polygons
    float mExtrusionHeight = 0.0f;  //!< How much to extrude (0 means no walls)
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    Qt3DRender::QCullFace::CullingMode mCullingMode = Qt3DRender::QCullFace::NoCulling;  //!< Front/back culling mode
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
};


#endif // QGSPOLYGON3DSYMBOL_H
