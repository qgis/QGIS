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
#include "qgs3dtypes.h"
#include "qgscolorrampshader.h"
#include "qgsmeshdataprovider.h"

#include <Qt3DRender/QCullFace>

class QgsAbstractMaterialSettings;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief 3D symbol that draws mesh geometry as planar triangles.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \note Not available in Python bindings
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
     * How to render the Z value of the mesh
     *
     * \since QGIS 3.14
     */
    enum ZValueType
    {
      //! Use the Z value of the vertices
      VerticesZValue = 0,
      //! Use the value from a dataset (for example, water surface value)
      ScalarDatasetZvalue
    };

    //! Constructor for QgsMesh3DSymbol
    QgsMesh3DSymbol();
    ~QgsMesh3DSymbol() override;

    QString type() const override { return "mesh"; }
    QgsMesh3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
     * Returns if the 3d rendering is enabled
     *
     * \since QGIS 3.14
     */
    bool isEnabled() const;

    /**
     * Sets if the 3d rendering is enabled
     *
     * \since QGIS 3.14
     */
    void setEnabled( bool enabled );

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgis::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns material used for shading of the symbol
    QgsAbstractMaterialSettings *material() const;

    /**
     * Sets the \a material settings used for shading of the symbol.
     *
     * Ownership of \a material is transferred to the symbol.
     */
    void setMaterial( QgsAbstractMaterialSettings *material SIP_TRANSFER );

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
     * Returns the index of the dataset group that will be used to render the vertical component of the 3D mesh geometry
     *
     * \since QGIS 3.14
     */
    int verticalDatasetGroupIndex() const;

    /**
     * Sets the index of the dataset group that will be used to render the vertical component of the 3D mesh geometry
     *
     * \since QGIS 3.14
     */
    void setVerticalDatasetGroupIndex( int verticalDatasetGroupIndex );

    /**
     * Returns if the vertical component of the mesh is relative to the mesh vertices Z value
     *
     * \since QGIS 3.14
     */
    bool isVerticalMagnitudeRelative() const;

    /**
     * Sets if the vertical component of the mesh is relative to the mesh vertices Z value
     *
     * \since QGIS 3.14
     */
    void setIsVerticalMagnitudeRelative( bool isVerticalMagnitudeRelative );

    /**
     * Returns if arrows are enabled for 3D rendering
     *
     * \since QGIS 3.14
     */
    bool arrowsEnabled() const;

    /**
     * Sets if arrows are enabled for 3D rendering
     *
     * \since QGIS 3.14
     */
    void setArrowsEnabled( bool arrowsEnabled );

    /**
     * Returns the arrow spacing
     *
     * \since QGIS 3.14
     */
    double arrowsSpacing() const;

    /**
     * Sets the arrow spacing
     *
     * \since QGIS 3.14
     */
    void setArrowsSpacing( double arrowsSpacing );

    /**
     * Returns the maximum texture size supported by the hardware
     * Used to store the GL_MAX_TEXTURE_SIZE value that comes from the 3D engine
     * before creating the entity
     *
     * \since QGIS 3.14
     */
    int maximumTextureSize() const;

    /**
     * Sets the maximum texture size supported by the hardware
     * Used to store the GL_MAX_TEXTURE_SIZE value that comes from the 3D engine
     * before creating the entity
     *
     * \since QGIS 3.14
     */
    void setMaximumTextureSize( int maximumTextureSize );

    /**
     * Returns if the arrow size is fixed
     *
     * \since QGIS 3.14
     */
    bool arrowsFixedSize() const;

    /**
     * Sets if the arrow size is fixed
     *
     * \since QGIS 3.14
     */
    void setArrowsFixedSize( bool arrowsFixedSize );

    /**
     * Returns the index of the level of detail of the mesh that is the position of the simplified mesh that will be rendered (0 is the original mesh)
     * \see QgsMeshSimplificationSettings
     *
     * \since QGIS 3.18
     */
    int levelOfDetailIndex() const;

    /**
     * Returns the index of the level of detail of the mesh that is the position of the simplified mesh that will be rendered (0 is the original mesh)
     * \see QgsMeshSimplificationSettings
     *
     * \since QGIS 3.18
     */
    void setLevelOfDetailIndex( int lod );

  private:

    //! how to handle altitude of vector features
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;
    float mHeight = 0.0f;           //!< Base height of triangles
    std::unique_ptr< QgsAbstractMaterialSettings > mMaterial;  //!< Defines appearance of objects
    bool mAddBackFaces = false;

    bool mEnabled = true;

    //! Triangles settings
    bool mSmoothedTriangles = false;
    bool mWireframeEnabled = false;
    double mWireframeLineWidth = 1.0;
    QColor mWireframeLineColor = Qt::darkGray;
    int mLevelOfDetailIndex = 0;

    //! Verticals settings
    double mVerticalScale = 1.0;
    int mVerticalDatasetGroupIndex = -1;
    bool mIsVerticalMagnitudeRelative = false;

    //! Color rendering settings
    QgsMesh3DSymbol::RenderingStyle mRenderingStyle = QgsMesh3DSymbol::SingleColor;
    QgsColorRampShader mColorRampShader;
    QColor mSingleColor = Qt::darkGreen;

    //! Arrows rendering
    bool mArrowsEnabled = false;
    double mArrowsSpacing = 25;
    bool mArrowsFixedSize = false;
    QColor mArrowsColor = Qt::yellow;
    int mMaximumTextureSize = 1024;
};

#endif // QGSMESH3DSYMBOL_H
