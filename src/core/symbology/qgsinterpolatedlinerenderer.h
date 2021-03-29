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
#include "qgsrendercontext.h"
#include "qgsrenderer.h"
#include "qgsunittypes.h"

class QgsLayerTreeLayer;

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

    //! Returns the coloring method used
    QgsInterpolatedLineColor::ColoringMethod coloringMethod() const;

    //! Returns the color ramp shader
    QgsColorRampShader colorRampShader() const;

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

    void graduatedColorsExact( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const;
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

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
    QgsFeatureRenderer *clone() const;

    //! Sets the stroke width used to render
    void setInterpolatedWidth( const QgsInterpolatedLineWidth &strokeWidth );

    //! Returns the stroke width used to render
    QgsInterpolatedLineWidth interpolatedLineWidth() const;

    //! Sets the unit of the stroke width
    void setWidthUnit( const QgsUnitTypes::RenderUnit &strokeWidthUnit );

    //! Returns the unit of the stroke width
    QgsUnitTypes::RenderUnit widthUnit() const;

    //! Sets the stroke color used to render
    void setInterpolatedColor( const QgsInterpolatedLineColor &strokeColoring );

    //! Returns the stroke color used to render
    QgsInterpolatedLineColor interpolatedColor() const;

    /**
     * Render a line in the \a context between \a point1 and \a point2
     * with color and width that vary depending on \a value1 and \a value2
     */
    void render( double value1, double value2, const QgsPointXY &point1, const QgsPointXY &point2, QgsRenderContext &context ) const;

    //! Sets if the rendering must be done as the element is selected
    void setSelected( bool selected );

  protected:
    QgsInterpolatedLineWidth mStrokeWidth;
    QgsInterpolatedLineColor mStrokeColoring;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;

  private:
    void adjustLine( const double &value, const double &value1, const double &value2, double &width, double &adjusting ) const;
    bool mSelected = false;
};

/**
 * \ingroup core
 * \class QgsInterpolatedLineFeatureRenderer
 * \brief A renderer which draws vector layer line feature as interpolated line
 * The interpolation is done between two values defined at the extremities
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsInterpolatedLineFeatureRenderer : public QgsInterpolatedLineRenderer, public QgsFeatureRenderer
{
  public:

    //! Constructor
    QgsInterpolatedLineFeatureRenderer();

    QgsInterpolatedLineFeatureRenderer *clone() const override SIP_FACTORY;

    //! Direct copies are forbidden. Use clone() instead.
    QgsInterpolatedLineFeatureRenderer( const QgsInterpolatedLineFeatureRenderer & ) = delete;
    //! Direct copies are forbidden. Use clone() instead.
    QgsInterpolatedLineFeatureRenderer &operator=( const QgsInterpolatedLineFeatureRenderer & ) = delete;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false )  override SIP_THROW( QgsCsException );
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;
    QList<QgsLayerTreeModelLegendNode *> extraLegendNodes( QgsLayerTreeLayer *nodeLayer ) const override;
    bool willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    //! Creates a new heatmap renderer instance from XML
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    //! Creates an new renderer from \a renderer
    static QgsInterpolatedLineFeatureRenderer *convertFromRenderer( QgsFeatureRenderer *renderer ) SIP_FACTORY;

    //! Sets the expressions (as string) that define the extremety values af the line feature
    void setExpressionsString( QString first, QString second );

    //! Returns the epression related to the first extremity value
    QString firstExpression() const;

    //! Returns the expression related to the second extremity value
    QString secondExpression() const;


  private:
    QString  mFirstExpressionString;
    QString mSecondExpressionString;

    int mFirstAttributeIndex = -1;
    int mSecondAttributeIndex = -1;
    std::unique_ptr<QgsExpression> mFirstExpression;
    std::unique_ptr<QgsExpression> mSecondExpression;

    bool prepareForRendering( const QgsFeature &feature, QgsRenderContext &context, QVector<QgsPolylineXY> &lineStrings, double &val1, double &val2, double &variationPerMapUnit ) const;
};

#endif // QGSINTERPOLATEDLINERENDERER_H
