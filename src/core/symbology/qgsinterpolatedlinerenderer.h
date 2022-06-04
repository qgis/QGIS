/***************************************************************************
  qgsinterpolatedlinerenderer.h
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERPOLATEDLINERENDERER_H
#define QGSINTERPOLATEDLINERENDERER_H

#include <QDebug>

#include "qgis.h"
#include "qgscolorrampshader.h"
#include "qgsreadwritecontext.h"
#include "qgsrenderer.h"
#include "qgsunittypes.h"
#include "qgssymbollayer.h"

class QgsLayerTreeLayer;
class QgsRenderContext;

/**
 * \ingroup core
 *
 * \brief Class defining color to render mesh datasets. The color can vary depending on the dataset value.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsInterpolatedLineColor
{
  public:

    /**
     * Defines how the color is defined
     */
    enum ColoringMethod
    {
      //! Render with a single color
      SingleColor = 0,
      //! Render with a color ramp
      ColorRamp
    };

    //! Default constructor
    QgsInterpolatedLineColor();
    //! Constructor  with variable color depending on magnitude
    QgsInterpolatedLineColor( const QgsColorRampShader &colorRampShader );
    //! Constructor  with fixed color
    QgsInterpolatedLineColor( const QColor &color );

    //! Sets the color ramp to define the coloring
    void setColor( const QgsColorRampShader &colorRampShader );

    //! Sets the single color to define the coloring
    void setColor( const QColor &color );

    //! Returns the color corresponding to the magnitude
    QColor color( double magnitude ) const;

    /**
     *  Sets the coloring method used
     *  \since QGIS 3.20
     */
    void setColoringMethod( ColoringMethod coloringMethod );

    //! Returns the coloring method used
    QgsInterpolatedLineColor::ColoringMethod coloringMethod() const;

    //! Returns the color ramp shader
    QgsColorRampShader colorRampShader() const;

    /**
     *  Returns the single color that is used if SingleColor coloring mode is set
     *  \since QGIS 3.20
     */
    QColor singleColor() const;

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     *  Returns the break values, graduated colors and the associated gradients between two values
     *
     * - If the color is fixed or only one color for the interval (value1, value2), returns only one color in \a breakColors
     *   and void lists for  \a breakValues, \a gradients
     * - If the color ramp is classified with 'exact', returns void \a gradients
     * - If the color ramp is classified with 'discrete', return \a gradients with uniform colors
     * - if nothing to render (out of range), return all lists void
     */
    void graduatedColors( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const;

  private:
    QgsColorRampShader mColorRampShader;
    QColor mSingleColor = Qt::black;

    QgsInterpolatedLineColor::ColoringMethod mColoringMethod = SingleColor;

    QLinearGradient makeSimpleLinearGradient( const QColor &color1, const QColor &color2 ) const;

    //! Returns the index of the color ramp shader with value inferior to value
    int itemColorIndexInf( double value ) const;

    void graduatedColorsExact( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, const QList<QLinearGradient> &gradients ) const;
    void graduatedColorsInterpolated( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const;
    void graduatedColorsDiscrete( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const;
};

/**
 * \ingroup core
 *
 * \class QgsInterpolatedLineWidth
 * \brief Represents a width than can vary depending on values
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsInterpolatedLineWidth
{
  public:
    //! Returns the minimum value used to defined the variable width
    double minimumValue() const;
    //! Sets the minimum value used to defined the variable width
    void setMinimumValue( double minimumValue );

    //! Returns the maximum value used to defined the variable width
    double maximumValue() const;
    //! Sets the maximum value used to defined the variable width
    void setMaximumValue( double maximumValue );

    //! Returns the minimum width used to defined the variable width
    double minimumWidth() const;
    //! Sets the minimum width used to defined the variable width
    void setMinimumWidth( double minimumWidth );

    //! Returns the maximum width used to defined the variable width
    double maximumWidth() const;
    //! Sets the maximum width used to defined the variable width
    void setMaximumWidth( double maximumWidth );

    //! Returns whether the variable width ignores out of range value
    bool ignoreOutOfRange() const;
    //! Sets whether the variable width ignores out of range value
    void setIgnoreOutOfRange( bool ignoreOutOfRange );

    //! Returns whether absolute value are used as input
    bool useAbsoluteValue() const;
    //! Sets whether absolute value are used as input
    void setUseAbsoluteValue( bool useAbsoluteValue );

    //! Returns whether the width is variable
    bool isVariableWidth() const;
    //! Returns whether the width is variable
    void setIsVariableWidth( bool isVariableWidth );

    //! Returns the fixed width
    double fixedStrokeWidth() const;
    //! Sets the fixed width
    void setFixedStrokeWidth( double fixedWidth );

    //! Returns the variable width depending on value, if not varying returns the fixed width
    double strokeWidth( double value ) const;

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

  private:
    bool mIsWidthVariable = false;

    double mFixedWidth = DEFAULT_LINE_WIDTH;

    double mMinimumValue = 0;
    double mMaximumValue = 10;
    double mMinimumWidth = DEFAULT_LINE_WIDTH;
    double mMaximumWidth = 3;
    bool mIgnoreOutOfRange = false;
    bool mUseAbsoluteValue = false;

    mutable double mLinearCoef = 1;
    mutable bool mNeedUpdateFormula = true;
    void updateLinearFormula() const;
};

/**
 * \ingroup core
 * \class QgsInterpolatedLineRenderer
 * \brief Represents a simple line renderer with width and color varying depending on values.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsInterpolatedLineRenderer
{
  public:

    //! Sets the stroke width used to render
    void setInterpolatedWidth( const QgsInterpolatedLineWidth &strokeWidth );

    /**
    *  Returns the stroke width used to render
    *  \since QGIS 3.20
    */
    QgsInterpolatedLineWidth interpolatedLineWidth() const;

    //! Sets the unit of the stroke width
    void setWidthUnit( QgsUnitTypes::RenderUnit strokeWidthUnit );

    /**
    *   Returns the unit of the stroke width
    *  \since QGIS 3.20
    */
    QgsUnitTypes::RenderUnit widthUnit() const;

    //! Sets the stroke color used to render
    void setInterpolatedColor( const QgsInterpolatedLineColor &strokeColoring );

    /**
    *  Returns the stroke color used to render
    *  \since QGIS 3.20
    */
    QgsInterpolatedLineColor interpolatedColor() const;

    /**
     * Renders a line in the \a context between \a point1 and \a point2
     * with color and width that vary depending on \a value1 and \a value2
     *
     * This method assumes that \a point1 and \a point2 are in map units. See renderInDeviceCoordinates() for an equivalent
     * method which renders lines in painter coordinates.
     */
    void render( double value1, double value2, const QgsPointXY &point1, const QgsPointXY &point2, QgsRenderContext &context ) const;

    /**
     * Renders a line in the \a context between \a point1 and \a point2
     * with color that varies depending on \a valueColor1 and \a valueColor2 and and width that varies between \a valueWidth1 and \a valueWidth2
     *
     * This method assumes that \a point1 and \a point2 are in map units. See renderInDeviceCoordinates() for an equivalent
     * method which renders lines in painter coordinates.
     *
     * \since QGIS 3.20
     */
    void render( double valueColor1, double valueColor2, double valueWidth1, double valueWidth2, const QgsPointXY &point1, const QgsPointXY &point2, QgsRenderContext &context ) const;

    /**
     * Renders a line in the \a context between \a point1 and \a point2 in device (painter) coordinates
     * with color that varies depending on \a valueColor1 and \a valueColor2 and and width that varies between \a valueWidth1 and \a valueWidth2.
     *
     * \since QGIS 3.22
     */
    void renderInDeviceCoordinates( double valueColor1, double valueColor2, double valueWidth1, double valueWidth2, QPointF point1, QPointF point2, QgsRenderContext &context ) const;

    /**
     * Sets if the rendering must be done as the element is selected
     *
     * \since QGIS 3.20
     */
    void setSelected( bool selected );

  private:

    QgsInterpolatedLineWidth mStrokeWidth;
    QgsInterpolatedLineColor mStrokeColoring;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    void adjustLine( double value, double value1, double value2, double &width, double &adjusting ) const;
    bool mSelected = false;


    friend class QgsInterpolatedLineSymbolLayer;
};

/**
 * \ingroup core
 * \class QgsInterpolatedLineSymbolLayer
 * \brief A symbol layer that represents vector layer line feature as interpolated line
 * The interpolation is done between two values defined at the extremities
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsInterpolatedLineSymbolLayer : public QgsLineSymbolLayer
{
  public:

    //! Constructor
    QgsInterpolatedLineSymbolLayer();

    //! Creates the symbol layer
    static QgsSymbolLayer *create( const QVariantMap &properties ) SIP_FACTORY;

    Qgis::SymbolLayerFlags flags() const override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsInterpolatedLineSymbolLayer *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;
    QColor color() const override;

    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    bool isCompatibleWithSymbol( QgsSymbol *symbol ) const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    /**
     * Sets the expressions (as string) that define the extremety values af the line feature for width.
     *
     * \deprecated use setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartWidthValue ) and setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndWidthValue ) instead
     */
    Q_DECL_DEPRECATED void setExpressionsStringForWidth( const QString &start, const QString &end ) SIP_DEPRECATED;

    /**
     * Returns the epression related to the start extremity value for width.
     *
     * \deprecated use dataDefinedProperty( QgsSymbolLayer::PropertyLineStartWidthValue ) instead.
     */
    Q_DECL_DEPRECATED QString startValueExpressionForWidth() const SIP_DEPRECATED;

    /**
     * Returns the expression related to the end extremity value for width.
     *
     * \deprecated use dataDefinedProperty( QgsSymbolLayer::PropertyLineEndWidthValue ) instead.
     */
    Q_DECL_DEPRECATED QString endValueExpressionForWidth() const SIP_DEPRECATED;

    /**
     * Sets the width unit.
     *
     * \see widthUnit()
     */
    void setWidthUnit( QgsUnitTypes::RenderUnit strokeWidthUnit );

    /**
     * Returns the width unit.
     *
     * \see setWidthUnit()
     */
    QgsUnitTypes::RenderUnit widthUnit() const;

    /**
     * Sets the interpolated width used to render the width of lines, \a see QgsInterpolatedLineWidth.
     *
     * \see interpolatedWidth()
     */
    void setInterpolatedWidth( const QgsInterpolatedLineWidth &interpolatedLineWidth );

    /**
     * Returns the interpolated width used to render the width of lines, see \a QgsInterpolatedLineWidth.
     *
     * \see setInterpolatedWidth()
     */
    QgsInterpolatedLineWidth interpolatedWidth() const;

    /**
     * Sets the expressions (as string) that define the extremety values af the line feature for color.
     *
     * \deprecated use setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartColorValue ) and setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndColorValue ) instead
     */
    Q_DECL_DEPRECATED void setExpressionsStringForColor( const QString &start, const QString &end ) SIP_DEPRECATED;

    /**
     * Returns the epression related to the start extremity value for width for color
     *
     * \deprecated use dataDefinedProperty( QgsSymbolLayer::PropertyLineStartColorValue ) instead.
     */
    Q_DECL_DEPRECATED QString startValueExpressionForColor() const SIP_DEPRECATED;

    /**
     * Returns the expression related to the end extremity value for width for color
     *
     * \deprecated use dataDefinedProperty( QgsSymbolLayer::PropertyLineEndColorValue ) instead.
     */
    Q_DECL_DEPRECATED QString endValueExpressionForColor() const SIP_DEPRECATED;

    /**
     * Sets the interpolated color used to render the colors of lines, \a see QgsInterpolatedLineColor.
     *
     * \see interpolatedColor()
     */
    void setInterpolatedColor( const QgsInterpolatedLineColor &interpolatedLineColor );

    /**
     * Returns the interpolated color used to render the colors of lines, see \a QgsInterpolatedLineColor.
     *
     * \see setInterpolatedColor()
     */
    QgsInterpolatedLineColor interpolatedColor() const;

  private:
#ifdef SIP_RUN
    QgsInterpolatedLineSymbolLayer( const QgsInterpolatedLineSymbolLayer &copy );
#endif

    QgsInterpolatedLineRenderer mLineRender;

    QVector< QPolygonF > mLineParts;
    bool mRenderingFeature = false;

    void render( const QVector< QPolygonF > &parts, QgsRenderContext &context );

    QVariant colorRampShaderProperties() const;
    static QgsColorRampShader createColorRampShaderFromProperties( const QVariant &properties );
};


#endif // QGSINTERPOLATEDLINERENDERER_H
