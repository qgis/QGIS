/***************************************************************************
  qgsmesh3dsymbol.h
  -----------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DSYMBOL_H
#define QGSMESH3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dtypes.h"

#include <Qt3DRender/QCullFace>

/**
 * \ingroup 3d
 * 3D symbol that draws mesh geometry as planar triangles.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.6
 */
class _3D_EXPORT QgsMesh3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsMesh3DSymbol
    QgsMesh3DSymbol() = default;

    QString type() const override { return "mesh"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgs3DTypes::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgs3DTypes::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns material used for shading of the symbol
    QgsPhongMaterialSettings material() const { return mMaterial; }
    //! Sets material used for shading of the symbol
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

    /**
     * Returns whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     */
    bool addBackFaces() const { return mAddBackFaces; }

    /**
     * Sets whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     */
    void setAddBackFaces( bool add ) { mAddBackFaces = add; }

  private:
    //! how to handle altitude of vector features
    Qgs3DTypes::AltitudeClamping mAltClamping = Qgs3DTypes::AltClampRelative;
    float mHeight = 0.0f;           //!< Base height of triangles
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    bool mAddBackFaces = false;
};


#endif // QGSMESH3DSYMBOL_H
