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
     * How to render the color of the point cloud
     */
    enum RenderingStyle
    {
      //! Render the mesh with a single color
      SingleColor = 0,
      //! Render the mesh with a color ramp
      ColorRamp,
    };

    /**
     * The value used to select colors from the color ramp
     */
    enum RenderingParameter
    {
      Height = 0,
      ClassID
    };

    //! Constructor for QgsPointCloud3DSymbol
    QgsPointCloud3DSymbol();
    //! Destructor for QgsPointCloud3DSymbol
    ~QgsPointCloud3DSymbol() override;

    QString type() const override { return "pointcloud"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
     * Returns whether rendering for this symbol is enabled
     * \see setIsEnabled( bool enabled )
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets whether rendering for this symbol is enabled
     * \see isEnabled()
     */
    void setIsEnabled( bool enabled );

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
     * Returns the rendering style
     * \see setRenderingStyle( QgsPointCloud3DSymbol::RenderingStyle style )
     */
    QgsPointCloud3DSymbol::RenderingStyle renderingStyle() const;

    /**
     * Sets the rendering style
     * \see renderingStyle()
     */
    void setRenderingStyle( QgsPointCloud3DSymbol::RenderingStyle style );

    /**
     * Returns the parameter used to select the color of the point cloud when using color ramp coloring
     * \see setRenderingParameter( QgsPointCloud3DSymbol::RenderingParameter parameter )
     */
    QgsPointCloud3DSymbol::RenderingParameter renderingParameter() const;

    /**
    * Sets the parameter used to select the color of the point cloud when using color ramp coloring
    * \see renderingParameter()
    */
    void setRenderingParameter( QgsPointCloud3DSymbol::RenderingParameter parameter );

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

    /**
    * Returns the color ramp shader used to render the color
    */
    QgsColorRampShader colorRampShader() const;

    /**
     * Sets the color ramp shader used to render the color
     */
    void setColorRampShader( const QgsColorRampShader &colorRampShader );

    /**
     * Returns the minimum value used when classifying colors in the color ramp shader
     * \see setColorRampShaderMinMax( double min, double max );
     */
    double colorRampShaderMin() const { return mColorRampShaderMin; }

    /**
     * Returns the maximum value used when classifying colors in the color ramp shader
     * \see setColorRampShaderMinMax( double min, double max );
     */
    double colorRampShaderMax() const { return mColorRampShaderMax; }

    /**
     * Sets the minimum and maximum values used when classifying colors in the color ramp shader
     * \see setColorRampShaderMinMax( double min, double max );
     */
    void setColorRampShaderMinMax( double min, double max );

  private:
    bool mEnabled = true;
    float mPointSize = 2.0f;

    QgsPointCloud3DSymbol::RenderingStyle mRenderingStyle = QgsPointCloud3DSymbol::ColorRamp;
    QgsPointCloud3DSymbol::RenderingParameter mRenderingParameter = QgsPointCloud3DSymbol::RenderingParameter::ClassID;
    QgsColorRampShader mColorRampShader;
    double mColorRampShaderMin = 0.0;
    double mColorRampShaderMax = 1.0;
    QColor mSingleColor = Qt::blue;
};

#endif // QGSPOINTCLOUD3DSYMBOL_H
