/***************************************************************************
  qgsline3dsymbol.h
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

#ifndef QGSLINE3DSYMBOL_H
#define QGSLINE3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"


/**
 * \ingroup 3d
 * 3D symbol that draws linestring geometries as planar polygons (created from lines using a buffer with given thickness).
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsLine3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsLine3DSymbol
    QgsLine3DSymbol() = default;

    QString type() const override { return "line"; }
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

    //! Returns width of the line symbol (in map units)
    float width() const { return mWidth; }
    //! Sets width of the line symbol (in map units)
    void setWidth( float width ) { mWidth = width; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns extrusion height (in map units)
    float extrusionHeight() const { return mExtrusionHeight; }
    //! Sets extrusion height (in map units)
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    //! Returns whether the renderer will render data with simple lines (otherwise it uses buffer)
    bool renderAsSimpleLines() const { return mRenderAsSimpleLines; }
    //! Sets whether the renderer will render data with simple lines (otherwise it uses buffer)
    void setRenderAsSimpleLines( bool enabled ) { mRenderAsSimpleLines = enabled; }

    //! Returns material used for shading of the symbol
    QgsPhongMaterialSettings material() const { return mMaterial; }
    //! Sets material used for shading of the symbol
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

  private:
    //! how to handle altitude of vector features
    AltitudeClamping mAltClamping = AltClampRelative;
    //! how to handle clamping of vertices of individual features
    AltitudeBinding mAltBinding = AltBindCentroid;

    float mWidth = 2.0f;            //!< Line width (horizontally)
    float mHeight = 0.0f;           //!< Base height of polygons
    float mExtrusionHeight = 0.0f;  //!< How much to extrude (0 means no walls)
    bool mRenderAsSimpleLines = false;   //!< Whether to render data with simple lines (otherwise it uses buffer)
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
};


#endif // QGSLINE3DSYMBOL_H
