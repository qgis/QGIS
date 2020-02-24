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
#include "qgscolorrampshader.h"
#include "qgsmeshdataprovider.h"

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

    /**
     * How to render the color of the mesh
     *
     * \since QGIS 3.12
     */
    enum RenderingStyle
    {
      //! Render the mesh with a single color
      SingleColor = 0,
      //! Render the mesh with a color ramp
      ColorRamp,
      //! Render the mesh with the color ramp shader of the 2D rendering
      ColorRamp2DRendering
    };

    /**
     * Which value to render the Z value of the mesh
     *
     * \since QGIS 3.14
     */
    enum ZvalueType
    {
      //! Use the Z value of the vertices
      VerticesZValue = 0,
      //! Render the mesh with a color ramp
      ScalarDatasetZvalue
    };

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

    /**
     * Returns if mesh triangle are smoothed
     *
     * \since QGIS 3.12
     */
    bool smoothedTriangles() const;

    /**
     * Sets if the mesh triangles have to been smoothed
     *
     * \since QGIS 3.12
     */
    void setSmoothedTriangles( bool smoothTriangles );

    /**
     * Returns if the mesh wireframe
     *
     * \since QGIS 3.12
     */
    bool wireframeEnabled() const;

    /**
     * Sets if the mesh wireframe
     *
     * \since QGIS 3.12
     */
    void setWireframeEnabled( bool wireframeEnabled );

    /**
     * Returns wireframe line width
     *
     * \since QGIS 3.12
     */
    double wireframeLineWidth() const;

    /**
     * Sets wireframe line width
     *
     * \since QGIS 3.12
     */
    void setWireframeLineWidth( double wireframeLineWidth );

    /**
     * Returns wireframe line color
     *
     * \since QGIS 3.12
     */
    QColor wireframeLineColor() const;

    /**
     * Sets wireframe line color
     *
     * \since QGIS 3.12
     */
    void setWireframeLineColor( const QColor &wireframeLineColor );

    /**
     * Returns mesh vertical scale
     *
     * \since QGIS 3.12
     */
    double verticalScale() const;

    /**
     * Sets mesh vertical scale
     *
     * \since QGIS 3.12
     */
    void setVerticalScale( double verticalScale );

    /**
     * Returns the color ramp shader used to render the color
     *
     * \since QGIS 3.12
     */
    QgsColorRampShader colorRampShader() const;

    /**
     * Sets the color ramp shader used to render the color
     *
     * \since QGIS 3.12
     */
    void setColorRampShader( const QgsColorRampShader &colorRampShader );

    /**
     * Returns the single color
     *
     * \since QGIS 3.12
     */
    QColor singleMeshColor() const;

    /**
     * Sets the single color
     *
     * \since QGIS 3.12
     */
    void setSingleMeshColor( const QColor &singleMeshColor );

    /**
     * Returns the rendering style
     *
     * \since QGIS 3.12
     */
    QgsMesh3DSymbol::RenderingStyle renderingStyle() const;

    /**
     * Sets the rendering style
     *
     * \since QGIS 3.12
     */
    void setRenderingStyle( const QgsMesh3DSymbol::RenderingStyle &textureType );

    /**
     * Returns the index og the dataset group that will be used to render the vertical of the 3D mesh
     *
     * \since QGIS 3.14
     */
    int verticalDatasetGroupIndex() const;

    /**
     * Sets the index og the dataset group that will be used to render the vertical of the 3D mesh
     *
     * \since QGIS 3.14
     */
    void setVerticalDatasetGroupIndex( int verticalDatasetGroupIndex );

    /**
     * Returns if the verticale magnitude is relative to the mesh vertices Z value
     *
     * \since QGIS 3.14
     */
    bool isVerticalMagnitudeRelative() const;

    /**
     * Sets if the verticale magnitude is relative to the mesh vertices Z value
     *
     * \since QGIS 3.14
     */
    void setIsVerticalMagnitudeRelative( bool isVerticalMagnitudeRelative );

  private:

    //old settings
    //! how to handle altitude of vector features
    Qgs3DTypes::AltitudeClamping mAltClamping = Qgs3DTypes::AltClampRelative;
    float mHeight = 0.0f;           //!< Base height of triangles
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    bool mAddBackFaces = false;

    // Triangles settings
    bool mSmoothedTriangles = false;
    bool mWireframeEnabled = false;
    double mWireframeLineWidth = 1.0;
    QColor mWireframeLineColor = Qt::darkGray;

    // Verticals settings
    double mVerticalScale = 1.0;
    int mVerticalDatasetGroupIndex = -1;
    bool mIsVerticalMagnitudeRelative = false;

    // Color rendering settings
    QgsMesh3DSymbol::RenderingStyle mRenderingStyle = QgsMesh3DSymbol::SingleColor;
    QgsColorRampShader mColorRampShader;
    QColor mSingleColor = Qt::darkGreen;
};

#endif // QGSMESH3DSYMBOL_H
