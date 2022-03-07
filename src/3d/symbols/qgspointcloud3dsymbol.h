/***************************************************************************
  qgspointcloud3dsymbol.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUD3DSYMBOL_H
#define QGSPOINTCLOUD3DSYMBOL_H

#include "qgis_3d.h"

#include <Qt3DRender/QMaterial>

#include "qgsabstract3dsymbol.h"
#include "qgscolorrampshader.h"
#include "qgspointcloudlayer.h"
#include "qgscontrastenhancement.h"
#include "qgspointcloudclassifiedrenderer.h"

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point cloud geometries as 3D objects.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloud3DSymbol : public QgsAbstract3DSymbol SIP_ABSTRACT
{
  public:

    /**
     * How to render the point cloud
     */
    enum RenderingStyle
    {
      // Do not render anything
      NoRendering = 0,
      //! Render the point cloud with a single color
      SingleColor,
      //! Render the point cloud with a color ramp
      ColorRamp,
      //! Render the RGB colors of the point cloud
      RgbRendering,
      //! Render the point cloud with classified colors
      Classification
    };

    //! Constructor for QgsPointCloud3DSymbol
    QgsPointCloud3DSymbol();
    //! Destructor for QgsPointCloud3DSymbol
    ~QgsPointCloud3DSymbol() override;

    QString type() const override { return "pointcloud"; }

    /**
     * Returns a unique string identifier of the symbol type.
     */
    virtual QString symbolType() const = 0;

    /**
     * Returns the point size of the point cloud
     * \see setPointSize( float size )
     */
    float pointSize() const { return mPointSize; }

    /**
     * Sets the point size
     * \see pointSize()
     */
    void setPointSize( float size );

    //! Returns the byte stride for the geometries used to for the vertex buffer
    virtual unsigned int byteStride() = 0;
    //! Used to fill material object with necessary QParameters (and consequently opengl uniforms)
    virtual void fillMaterial( Qt3DRender::QMaterial *material ) = 0 SIP_SKIP;

    /**
     * Returns whether points are triangulated to render solid surface
     *
     * \since QGIS 3.26
     */
    bool renderAsTriangles() const;

    /**
     * Sets whether points are triangulated to render solid surface
     *
     * \since QGIS 3.26
     */
    void setRenderAsTriangles( bool asTriangles );

    /**
     * Returns whether triangles are filtered by horizontal size for rendering. If the triangles are horizontally filtered by size,
     * triangles with a horizontal side size greater than a threshold value will not be rendered, see horizontalFilterThreshold().
     *
     * \since QGIS 3.26
     */
    bool horizontalTriangleFilter() const;

    /**
     * Sets whether whether triangles are filtered by horizontal size for rendering. If the triangles are horizontally filtered by size,
     * triangles with a horizontal side size greater than a threshold value will not be rendered, see setHorizontalFilterThreshold().
     *
     * \since QGIS 3.26
     */
    void setHorizontalTriangleFilter( bool horizontalTriangleFilter );

    /**
     * Returns the threshold horizontal size value for filtering triangles. If the triangles are horizontally filtered by size,
     * triangles with a horizontal side size greater than a threshold value will not be rendered, see horizontalTriangleFilter().
     *
     * \since QGIS 3.26
     */
    float horizontalFilterThreshold() const;

    /**
     * Sets the threshold horizontal size value for filtering triangles. If the triangles are horizontally filtered by size,
     * triangles with a horizontal side size greater than a threshold value will not be rendered, see setHorizontalTriangleFilter().
     *
     * \since QGIS 3.26
     */
    void setHorizontalFilterThreshold( float horizontalFilterThreshold );

    /**
     * Returns whether triangles are filtered by vertical height for rendering. If the triangles are vertically filtered, triangles with a vertical height greater
     * than a threshold value will not be rendered, see verticalFilterThreshold().
     *
     * \since QGIS 3.26
     */
    bool verticalTriangleFilter() const;

    /**
     * Sets whether triangles are filtered by vertical height for rendering. If the triangles are vertically filtered, triangles with a vertical height greater
     * than a threshold value will not be rendered, see setVerticalFilterThreshold().
     *
     * \since QGIS 3.26
     */
    void setVerticalTriangleFilter( bool verticalTriangleFilter );

    /**
     * Returns the threshold vertical height value for filtering triangles. If the triangles are filtered vertically, triangles with a vertical height greater
     * than this threshold value will not be rendered, see verticalTriangleFilter().
     *
     * \since QGIS 3.26
     */
    float verticalFilterThreshold() const;

    /**
     * Sets the threshold vertical height value for filtering triangles. If the triangles are filtered vertically, triangles with a vertical height greater
     * than this threshold value will not be rendered, see setVerticalTriangleFilter().
     *
     * \since QGIS 3.26
     */
    void setVerticalFilterThreshold( float verticalFilterThreshold );

  protected:
    float mPointSize = 2.0;
    bool mRenderAsTriangles = false;
    bool mHorizontalTriangleFilter = false;
    float mHorizontalFilterThreshold = 10.0;
    bool mVerticalTriangleFilter = false;
    float mVerticalFilterThreshold = 10.0;

    /**
     * Writes symbol configuration of this class to the given DOM element
     *
     * \since QGIS 3.26
     */
    void writeBaseXml( QDomElement &elem, const QgsReadWriteContext &context ) const;

    /**
     * Reads symbol configuration of this class from the given DOM element
     *
     * \since QGIS 3.26
     */
    void readBaseXml( const QDomElement &elem, const QgsReadWriteContext &context );

    void copyBaseSettings( QgsAbstract3DSymbol *destination ) const override;
};

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point cloud geometries as 3D objects.using one color
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsSingleColorPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsSingleColorPointCloud3DSymbol
    QgsSingleColorPointCloud3DSymbol();

    QString symbolType() const override;
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
    * Returns the color used by the renderer when using SingleColor rendering mode
    * \see setSingleColor( QColor color )
    */
    QColor singleColor() const { return mSingleColor; }

    /**
    * Sets the color used by the renderer when using SingleColor rendering mode
    * \see singleColor()
    */
    void setSingleColor( QColor color );

    unsigned int byteStride() override { return 3 * sizeof( float ); }
    void fillMaterial( Qt3DRender::QMaterial *material ) override SIP_SKIP;


  private:
    QColor mSingleColor = QColor( 0, 0, 255 );
};

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point cloud geometries as 3D objects.using color ramp shader
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsColorRampPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsColorRampPointCloud3DSymbol
    QgsColorRampPointCloud3DSymbol();

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;
    QString symbolType() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
    * Returns the attribute used to select the color of the point cloud.
    * \see setAttribute()
    */
    QString attribute() const;

    /**
    * Sets the \a attribute used to select the color of the point cloud.
    * \see attribute()
    */
    void setAttribute( const QString &attribute );

    /**
    * Returns the color ramp shader used to render the color
    * \see setColorRampShader( const QgsColorRampShader &colorRampShader )
    */
    QgsColorRampShader colorRampShader() const;

    /**
     * Sets the color ramp shader used to render the point cloud
     * \see colorRampShader()
     */
    void setColorRampShader( const QgsColorRampShader &colorRampShader );

    /**
     * Returns the minimum value used when classifying colors in the color ramp shader
     * \see setColorRampShaderMinMax( double min, double max )
     */
    double colorRampShaderMin() const { return mColorRampShaderMin; }

    /**
     * Returns the maximum value used when classifying colors in the color ramp shader
     * \see setColorRampShaderMinMax( double min, double max )
     */
    double colorRampShaderMax() const { return mColorRampShaderMax; }

    /**
     * Sets the minimum and maximum values used when classifying colors in the color ramp shader
     * \see colorRampShaderMin() colorRampShaderMax()
     */
    void setColorRampShaderMinMax( double min, double max );

    unsigned int byteStride() override { return 4 * sizeof( float ); }
    void fillMaterial( Qt3DRender::QMaterial *material ) override SIP_SKIP;

  private:
    QString mRenderingParameter;
    QgsColorRampShader mColorRampShader;
    double mColorRampShaderMin = 0.0;
    double mColorRampShaderMax = 1.0;
};

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point cloud geometries as 3D objects using RGB colors in the dataset
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsRgbPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsRGBPointCloud3DSymbol
    QgsRgbPointCloud3DSymbol();

    //! QgsRgbPointCloud3DSymbol cannot be copied - use clone() instead
    QgsRgbPointCloud3DSymbol( const QgsRgbPointCloud3DSymbol &other ) = delete;

    //! QgsRgbPointCloud3DSymbol cannot be copied - use clone() instead
    QgsRgbPointCloud3DSymbol &operator=( const QgsRgbPointCloud3DSymbol &other ) = delete;

    QString symbolType() const override;
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    unsigned int byteStride() override { return 6 * sizeof( float ); }
    void fillMaterial( Qt3DRender::QMaterial *material ) override SIP_SKIP;

    /**
     * Returns the attribute to use for the red channel.
     *
     * \see greenAttribute()
     * \see blueAttribute()
     * \see setRedAttribute()
     */
    QString redAttribute() const;

    /**
     * Sets the \a attribute to use for the red channel.
     *
     * \see setGreenAttribute()
     * \see setBlueAttribute()
     * \see redAttribute()
     */
    void setRedAttribute( const QString &attribute );

    /**
     * Returns the attribute to use for the green channel.
     *
     * \see redAttribute()
     * \see blueAttribute()
     * \see setGreenAttribute()
     */
    QString greenAttribute() const;

    /**
     * Sets the \a attribute to use for the green channel.
     *
     * \see setRedAttribute()
     * \see setBlueAttribute()
     * \see greenAttribute()
     */
    void setGreenAttribute( const QString &attribute );

    /**
     * Returns the attribute to use for the blue channel.
     *
     * \see greenAttribute()
     * \see redAttribute()
     * \see setBlueAttribute()
     */
    QString blueAttribute() const;

    /**
     * Sets the \a attribute to use for the blue channel.
     *
     * \see setRedAttribute()
     * \see setGreenAttribute()
     * \see blueAttribute()
     */
    void setBlueAttribute( const QString &attribute );

    /**
     * Returns the contrast enhancement to use for the red channel.
     *
     * \see setRedContrastEnhancement()
     * \see greenContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    QgsContrastEnhancement *redContrastEnhancement();

    /**
     * Sets the contrast \a enhancement to use for the red channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see redContrastEnhancement()
     * \see setGreenContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setRedContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the green channel.
     *
     * \see setGreenContrastEnhancement()
     * \see redContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    QgsContrastEnhancement *greenContrastEnhancement();

    /**
     * Sets the contrast \a enhancement to use for the green channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see greenContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setGreenContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the blue channel.
     *
     * \see setBlueContrastEnhancement()
     * \see redContrastEnhancement()
     * \see greenContrastEnhancement()
     */
    QgsContrastEnhancement *blueContrastEnhancement();

    /**
     * Sets the contrast \a enhancement to use for the blue channel.
     *
     * Ownership of \a enhancement is transferred.
     *
     * \see blueContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setGreenContrastEnhancement()
     */
    void setBlueContrastEnhancement( QgsContrastEnhancement *enhancement SIP_TRANSFER );

  private:

#ifdef SIP_RUN
    QgsRgbPointCloud3DSymbol( const QgsRgbPointCloud3DSymbol &other );
#endif

    QString mRedAttribute = QStringLiteral( "Red" );
    QString mGreenAttribute = QStringLiteral( "Green" );
    QString mBlueAttribute = QStringLiteral( "Blue" );

    std::unique_ptr< QgsContrastEnhancement > mRedContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mGreenContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mBlueContrastEnhancement;

};

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point cloud geometries as 3D objects using classification of the dataset
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsClassificationPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsClassificationPointCloud3DSymbol
    QgsClassificationPointCloud3DSymbol();

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;
    QString symbolType() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
    * Returns the attribute used to select the color of the point cloud.
    * \see setAttribute()
    */
    QString attribute() const;

    /**
    * Sets the \a attribute used to select the color of the point cloud.
    * \see attribute()
    */
    void setAttribute( const QString &attribute );

    /**
     * Returns the list of categories of the classification
     * \see setCategoriesList()
     */
    QgsPointCloudCategoryList categoriesList() const { return mCategoriesList; }

    /**
     * Sets the list of categories of the classification
     * \see categoriesList()
     */
    void setCategoriesList( const QgsPointCloudCategoryList &categories );

    /**
     * Gets the list of categories of the classification that should not be rendered
     * \see categoriesList() setCategoriesList()
     */
    QgsPointCloudCategoryList getFilteredOutCategories() const;

    unsigned int byteStride() override { return 4 * sizeof( float ); }
    void fillMaterial( Qt3DRender::QMaterial *material ) override SIP_SKIP;

  private:
    QString mRenderingParameter;
    QgsPointCloudCategoryList mCategoriesList;

    QgsColorRampShader colorRampShader() const;
};

#endif // QGSPOINTCLOUD3DSYMBOL_H
