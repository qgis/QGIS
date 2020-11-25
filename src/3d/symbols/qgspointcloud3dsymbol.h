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

#include "qgsabstract3dsymbol.h"
#include "qgscolorrampshader.h"
#include "qgspointcloudlayer.h"

/**
 * \ingroup 3d
 * 3D symbol that draws point cloud geometries as 3D objects.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloud3DSymbol : public QgsAbstract3DSymbol
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
      RGBRendering
    };

    //! Constructor for QgsPointCloud3DSymbol
    QgsPointCloud3DSymbol( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol::RenderingStyle style );
    //! Destructor for QgsPointCloud3DSymbol
    ~QgsPointCloud3DSymbol() override;

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY { return nullptr; }

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override { Q_UNUSED( elem ); Q_UNUSED( context ); }
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override { Q_UNUSED( elem ); Q_UNUSED( context ); }

    QString type() const override { return "pointcloud"; }

    /**
     * Returns the point cloud layer object used by the symbol
     * see setLayer( QgsPointCloudLayer *layer )
     */
    QgsPointCloudLayer *layer() { return mLayer; }

    /**
     * Sets the point cloud layer object used by the symbol
     * see layer()
     */
    void setLayer( QgsPointCloudLayer *layer );

    //! Returns the rendering style used to render the point cloud
    QgsPointCloud3DSymbol::RenderingStyle renderingStyle() const { return mRenderingStyle; }

  protected:
    QgsPointCloud3DSymbol::RenderingStyle mRenderingStyle = QgsPointCloud3DSymbol::NoRendering;
    QgsPointCloudLayer *mLayer = nullptr;
};

/**
 * \ingroup 3d
 * 3D symbol that is used to indicate that the point cloud won't be rendered
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsNoRenderingPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsNoRenderingPointCloud3DSymbol
    QgsNoRenderingPointCloud3DSymbol( QgsPointCloudLayer *layer );

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
};

/**
 * \ingroup 3d
 * 3D symbol that draws point cloud geometries as 3D objects.using one color
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
    QgsSingleColorPointCloud3DSymbol( QgsPointCloudLayer *layer );

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

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

  private:
    float mPointSize = 2.0f;
    QColor mSingleColor = Qt::blue;
};

/**
 * \ingroup 3d
 * 3D symbol that draws point cloud geometries as 3D objects.using color ramp shader
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
    QgsColorRampPointCloud3DSymbol( QgsPointCloudLayer *layer );

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

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

    /**
    * Returns the parameter used to select the color of the point cloud
    * \see setRenderingParameter( const QString &parameter )
    */
    QString renderingParameter() const;

    /**
    * Sets the parameter used to select the color of the point cloud
    * \see renderingParameter()
    */
    void setRenderingParameter( const QString &parameter );

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

  private:
    float mPointSize = 2.0f;
    QString mRenderingParameter;
    QgsColorRampShader mColorRampShader;
    double mColorRampShaderMin = 0.0;
    double mColorRampShaderMax = 1.0;
};

/**
 * \ingroup 3d
 * 3D symbol that draws point cloud geometries as 3D objects using RGB colors in the dataset
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsRGBPointCloud3DSymbol : public QgsPointCloud3DSymbol
{
  public:
    //! Constructor for QgsRGBPointCloud3DSymbol
    QgsRGBPointCloud3DSymbol( QgsPointCloudLayer *layer );

    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

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
  private:
    float mPointSize;
};

#endif // QGSPOINTCLOUD3DSYMBOL_H
