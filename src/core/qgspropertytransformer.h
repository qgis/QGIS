/***************************************************************************
     qgspropertytransformer.h
     ------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROPERTYTRANSFORMER_H
#define QGSPROPERTYTRANSFORMER_H

#include "qgis_core.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgspointxy.h"
#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QDomDocument>
#include <QColor>
#include <memory>
#include <algorithm>

class QgsColorRamp;


/**
 * \ingroup core
 * \class QgsCurveTransform
 * \brief Handles scaling of input values to output values by using a curve created
 * from smoothly joining a number of set control points.
 *
 * QgsCurveTransform assists in creation of curve type transforms, typically seen in
 * raster image editing software (eg the curves dialog in GIMP or Photoshop).
 * Transforms are created by passing a number of set control points through which
 * the transform curve must pass. The curve is guaranteed to exactly pass through
 * these control points. Between control points the curve is smoothly interpolated
 * so that no disjoint sections or "corners" are present.
 *
 * If the first or last control point are not located at x = 0 and x = 1 respectively,
 * then values outside this range will be mapped to the y value of either the first
 * or last control point. In other words, the curve will have a flat segment
 * for values outside of the control point range.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsCurveTransform
{
  public:

    /**
     * Constructs a default QgsCurveTransform which linearly maps values
     * between 0 and 1 unchanged. I.e. y == x.
     */
    QgsCurveTransform();

    /**
     * Constructs a QgsCurveTransform using a specified list of \a controlPoints.
     * Behavior is undefined if duplicate x values exist in the control points
     * list.
     */
    QgsCurveTransform( const QList< QgsPointXY > &controlPoints );

    ~QgsCurveTransform();

    /**
     * Copy constructor
     */
    QgsCurveTransform( const QgsCurveTransform &other );

    QgsCurveTransform &operator=( const QgsCurveTransform &other );

    /**
     * Returns a list of the control points for the transform.
     * \see setControlPoints()
     */
    QList< QgsPointXY > controlPoints() const { return mControlPoints; }

    /**
     * Sets the list of control points for the transform. Any existing
     * points are removed.
     * \see controlPoints()
     */
    void setControlPoints( const QList< QgsPointXY > &points );

    /**
     * Adds a control point to the transform. Behavior is undefined if duplicate
     * x values exist in the control points list.
     * \see removeControlPoint()
     */
    void addControlPoint( double x, double y );

    /**
     * Removes a control point from the transform. This will have no effect if a
     * matching control point does not exist.
     * \see addControlPoint()
     */
    void removeControlPoint( double x, double y );

    /**
     * Returns the mapped y value corresponding to the specified \a x value.
     */
    double y( double x ) const;

    /**
     * Returns a list of y values corresponding to a list of \a x values.
     * Calling this method is faster then calling the double variant multiple
     * times.
     */
    QVector< double > y( const QVector< double > &x ) const;

    /**
     * Reads the curve's state from an XML element.
     * \param elem source DOM element for transform's state
     * \param doc DOM document
     * \see writeXml()
    */
    bool readXml( const QDomElement &elem, const QDomDocument &doc );

    /**
     * Writes the current state of the transform into an XML element
     * \param transformElem destination element for the transform's state
     * \param doc DOM document
     * \see readXml()
    */
    bool writeXml( QDomElement &transformElem, QDomDocument &doc ) const;

    /**
     * Saves this curve transformer to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     *
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Load this curve transformer from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to load it from an XML document.
     *
     * \see toVariant()
     */
    bool loadVariant( const QVariant &transformer );

  private:

    void calcSecondDerivativeArray();

    QList< QgsPointXY > mControlPoints;

    double *mSecondDerivativeArray = nullptr;
};


/**
 * \ingroup core
 * \class QgsPropertyTransformer
 * \brief Abstract base class for objects which transform the calculated value of a property.
 * Possible uses include transformers which map a value into a scaled size or color from a gradient.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPropertyTransformer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->transformerType() == QgsPropertyTransformer::GenericNumericTransformer )
      sipType = sipType_QgsGenericNumericTransformer;
    else if ( sipCpp->transformerType() == QgsPropertyTransformer::SizeScaleTransformer )
      sipType = sipType_QgsSizeScaleTransformer;
    else if ( sipCpp->transformerType() == QgsPropertyTransformer::ColorRampTransformer )
      sipType = sipType_QgsColorRampTransformer;
    else
      sipType = sipType_QgsPropertyTransformer;
    SIP_END
#endif

  public:

    //! Transformer types
    enum Type
    {
      GenericNumericTransformer, //!< Generic transformer for numeric values (QgsGenericNumericTransformer)
      SizeScaleTransformer, //!< Size scaling transformer (QgsSizeScaleTransformer)
      ColorRampTransformer, //!< Color ramp transformer (QgsColorRampTransformer)
    };

    /**
     * Factory method for creating a new property transformer of the specified type.
     * \param type transformer type to create
     */
    static QgsPropertyTransformer *create( Type type ) SIP_FACTORY;

    /**
     * Constructor for QgsPropertyTransformer
     * \param minValue minimum expected value from source property
     * \param maxValue maximum expected value from source property
     */
    QgsPropertyTransformer( double minValue = 0.0, double maxValue = 1.0 );

    /**
     * Copy constructor.
     */
    QgsPropertyTransformer( const QgsPropertyTransformer &other );
    QgsPropertyTransformer &operator=( const QgsPropertyTransformer &other );

    virtual ~QgsPropertyTransformer() = default;

    /**
     * Returns the transformer type.
     */
    virtual Type transformerType() const = 0;

    /**
     * Returns a clone of the transformer.
     */
    virtual QgsPropertyTransformer *clone() const = 0 SIP_FACTORY;

    /**
     * Loads this transformer from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to read it from an XML document.
     *
     * \see toVariant()
     */
    virtual bool loadVariant( const QVariant &transformer );

    /**
     * Saves this transformer to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     *
     * \see loadVariant()
     */
    virtual QVariant toVariant() const;

    /**
     * Returns the minimum value expected by the transformer.
     * \see maxValue()
     * \see setMinValue()
     */
    double minValue() const { return mMinValue; }

    /**
     * Sets the minimum value expected by the transformer.
     * \param min minimum value
     * \see setMaxValue()
     * \see minValue()
     */
    void setMinValue( double min ) { mMinValue = min; }

    /**
     * Returns the maximum value expected by the transformer.
     * \see minValue()
     * \see setMaxValue()
     */
    double maxValue() const { return mMaxValue; }

    /**
     * Sets the maximum value expected by the transformer.
     * \param max maximum value
     * \see setMinValue()
     * \see maxValue()
     */
    void setMaxValue( double max ) { mMaxValue = max; }

    /**
     * Returns the curve transform applied to input values before they are transformed
     * by the individual transform subclasses.
     * \see setCurveTransform()
     */
    QgsCurveTransform *curveTransform() const { return mCurveTransform.get(); }

    /**
     * Sets a curve transform to apply to input values before they are transformed
     * by the individual transform subclasses. Ownership of \a transform is transferred
     * to the property transformer.
     * \see curveTransform()
     */
    void setCurveTransform( QgsCurveTransform *transform SIP_TRANSFER ) { mCurveTransform.reset( transform ); }

    /**
     * Calculates the transform of a value. Derived classes must implement this to perform their transformations
     * on input values
     * \param context expression context
     * \param value input value to transform
     */
    virtual QVariant transform( const QgsExpressionContext &context, const QVariant &value ) const = 0;

    /**
     * Converts the transformer to a QGIS expression string. The \a baseExpression string consists
     * of a sub-expression reflecting the parent property's state.
     */
    virtual QString toExpression( const QString &baseExpression ) const = 0;

    /**
     * Attempts to parse an expression into a corresponding property transformer.
     * \param expression expression to parse
     * \param baseExpression will be set to the component of the source expression which
     * is used to calculate the input to the property transformer. This will be set to an
     * empty string if a field reference is the transformer input.
     * \param fieldName will be set to a field name which is used to calculate the input
     * to the property transformer. This will be set to an
     * empty string if an expression is the transformer input.
     * \returns corresponding property transformer, or NULLPTR if expression could not
     * be parsed to a transformer.
     */
    static QgsPropertyTransformer *fromExpression( const QString &expression, QString &baseExpression SIP_OUT, QString &fieldName SIP_OUT ) SIP_FACTORY;

  protected:

    /**
     * Applies base class numeric transformations. Derived classes should call this
     * to transform an \a input numeric value before they apply any transform to the result.
     * This applies any curve transforms which may exist on the transformer.
     */
    double transformNumeric( double input ) const;
#ifndef SIP_RUN
    //! Minimum value expected by the transformer
    double mMinValue;

    //! Maximum value expected by the transformer
    double mMaxValue;

    //! Optional curve transform
    std::unique_ptr< QgsCurveTransform > mCurveTransform;
#endif
};

/**
 * \ingroup core
 * \class QgsGenericNumericTransformer
 * \brief QgsPropertyTransformer subclass for scaling an input numeric value into an output numeric value.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsGenericNumericTransformer : public QgsPropertyTransformer
{
  public:

    /**
     * Constructor for QgsGenericNumericTransformer.
     * \param minValue minimum expected input value
     * \param maxValue maximum expected input value
     * \param minOutput minimum value to return
     * \param maxOutput maximum value to return
     * \param nullOutput value to return for null inputs
     * \param exponent optional exponential for non-linear scaling
     */
    QgsGenericNumericTransformer( double minValue = 0.0,
                                  double maxValue = 1.0,
                                  double minOutput = 0.0,
                                  double maxOutput = 1.0,
                                  double nullOutput = 0.0,
                                  double exponent = 1.0 );

    Type transformerType() const override { return GenericNumericTransformer; }
    QgsGenericNumericTransformer *clone() const override SIP_FACTORY;
    QVariant toVariant() const override;
    bool loadVariant( const QVariant &definition ) override;
    QVariant transform( const QgsExpressionContext &context, const QVariant &value ) const override;
    QString toExpression( const QString &baseExpression ) const override;

    /**
     * Attempts to parse an expression into a corresponding QgsSizeScaleTransformer.
     * \param expression expression to parse
     * \param baseExpression will be set to the component of the source expression which
     * is used to calculate the input to the property transformer. This will be set to an
     * empty string if a field reference is the transformer input.
     * \param fieldName will be set to a field name which is used to calculate the input
     * to the property transformer. This will be set to an
     * empty string if an expression is the transformer input.
     * \returns corresponding QgsSizeScaleTransformer, or NULLPTR if expression could not
     * be parsed to a size scale transformer.
     */
    static QgsGenericNumericTransformer *fromExpression( const QString &expression, QString &baseExpression SIP_OUT, QString &fieldName SIP_OUT ) SIP_FACTORY;

    /**
     * Calculates the size corresponding to a specific \a input value.
     * \returns calculated size using size scale transformer's parameters and type
     */
    double value( double input ) const;

    /**
     * Returns the minimum calculated size.
     * \see setMinOutputValue()
     * \see maxOutputValue()
     */
    double minOutputValue() const { return mMinOutput; }

    /**
     * Sets the minimum calculated size.
     * \param size minimum size
     * \see minOutputValue()
     * \see setMaxOutputValue()
     */
    void setMinOutputValue( double size ) { mMinOutput = size; }

    /**
     * Returns the maximum calculated size.
     * \see minOutputValue()
     */
    double maxOutputValue() const { return mMaxOutput; }

    /**
     * Sets the maximum calculated size.
     * \param size maximum size
     * \see maxOutputValue()
     * \see setMinOutputValue()
     */
    void setMaxOutputValue( double size ) { mMaxOutput = size; }

    /**
     * Returns the size value when an expression evaluates to NULL.
     * \see setNullOutputValue()
     */
    double nullOutputValue() const { return mNullOutput; }

    /**
     * Sets the size value for when an expression evaluates to NULL.
     * \param size null size
     * \see nullOutputValue()
     */
    void setNullOutputValue( double size ) { mNullOutput = size; }

    /**
     * Returns the exponent for an exponential expression.
     * \see setExponent()
     */
    double exponent() const { return mExponent; }

    /**
     * Sets the exponent for an exponential expression.
     * \param exponent exponent
     * \see exponent()
     */
    void setExponent( double exponent ) { mExponent = exponent; }

  private:
    double mMinOutput;
    double mMaxOutput;
    double mNullOutput;
    double mExponent;

};

/**
 * \ingroup core
 * \class QgsSizeScaleTransformer
 * \brief QgsPropertyTransformer subclass for scaling a value into a size according to various
 * scaling methods.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsSizeScaleTransformer : public QgsPropertyTransformer
{
  public:

    //! Size scaling methods
    enum ScaleType
    {
      Linear, //!< Linear scaling
      Area, //!< Area based scaling
      Flannery, //!< Flannery scaling method
      Exponential, //!< Scale using set exponent
    };

    /**
     * Constructor for QgsSizeScaleTransformer.
     * \param type scaling type
     * \param minValue minimum expected value
     * \param maxValue maximum expected value
     * \param minSize minimum size to return
     * \param maxSize maximum size to return
     * \param nullSize size to return for null values
     * \param exponent exponent for Exponential scaling method
     */
    QgsSizeScaleTransformer( ScaleType type = Linear,
                             double minValue = 0.0,
                             double maxValue = 1.0,
                             double minSize = 0.0,
                             double maxSize = 1.0,
                             double nullSize = 0.0,
                             double exponent = 1.0 );

    Type transformerType() const override { return SizeScaleTransformer; }
    QgsSizeScaleTransformer *clone() const override SIP_FACTORY;
    QVariant toVariant() const override;
    bool loadVariant( const QVariant &definition ) override;
    QVariant transform( const QgsExpressionContext &context, const QVariant &value ) const override;
    QString toExpression( const QString &baseExpression ) const override;

    /**
     * Attempts to parse an expression into a corresponding QgsSizeScaleTransformer.
     * \param expression expression to parse
     * \param baseExpression will be set to the component of the source expression which
     * is used to calculate the input to the property transformer. This will be set to an
     * empty string if a field reference is the transformer input.
     * \param fieldName will be set to a field name which is used to calculate the input
     * to the property transformer. This will be set to an
     * empty string if an expression is the transformer input.
     * \returns corresponding QgsSizeScaleTransformer, or NULLPTR if expression could not
     * be parsed to a size scale transformer.
     */
    static QgsSizeScaleTransformer *fromExpression( const QString &expression, QString &baseExpression SIP_OUT, QString &fieldName SIP_OUT ) SIP_FACTORY;

    /**
     * Calculates the size corresponding to a specific value.
     * \param value value to calculate size for
     * \returns calculated size using size scale transformer's parameters and type
     */
    double size( double value ) const;

    /**
     * Returns the minimum calculated size.
     * \see setMinSize()
     * \see maxSize()
     */
    double minSize() const { return mMinSize; }

    /**
     * Sets the minimum calculated size.
     * \param size minimum size
     * \see minSize()
     * \see setMaxSize()
     */
    void setMinSize( double size ) { mMinSize = size; }

    /**
     * Returns the maximum calculated size.
     * \see minSize()
     */
    double maxSize() const { return mMaxSize; }

    /**
     * Sets the maximum calculated size.
     * \param size maximum size
     * \see maxSize()
     * \see setMinSize()
     */
    void setMaxSize( double size ) { mMaxSize = size; }

    /**
     * Returns the size value when an expression evaluates to NULL.
     * \see setNullSize()
     */
    double nullSize() const { return mNullSize; }

    /**
     * Sets the size value for when an expression evaluates to NULL.
     * \param size null size
     * \see nullSize()
     */
    void setNullSize( double size ) { mNullSize = size; }

    /**
     * Returns the exponent for an exponential expression.
     * \see setExponent()
     * \see type()
     */
    double exponent() const { return mExponent; }

    /**
     * Sets the exponent for an exponential expression.
     * \param exponent exponent
     * \see exponent()
     */
    void setExponent( double exponent ) { mExponent = exponent; }

    /**
     * Returns the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * \see setType()
     */
    ScaleType type() const { return mType; }

    /**
     * Sets the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * \param type scale type
     * \see type()
     */
    void setType( ScaleType type );

  private:
    ScaleType mType = Linear;
    double mMinSize;
    double mMaxSize;
    double mNullSize;
    double mExponent;

};

/**
 * \ingroup core
 * \class QgsColorRampTransformer
 * \brief QgsPropertyTransformer subclass for transforming a numeric value into a color from a
 * color ramp.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsColorRampTransformer : public QgsPropertyTransformer
{
  public:

    /**
     * Constructor for QgsColorRampTransformer.
     * \param minValue minimum expected value
     * \param maxValue maximum expected value
     * \param ramp source color ramp. Ownership is transferred to the transformer.
     * \param nullColor color to return for null values
     */
    QgsColorRampTransformer( double minValue = 0.0,
                             double maxValue = 1.0,
                             QgsColorRamp *ramp SIP_TRANSFER = nullptr,
                             const QColor &nullColor = QColor( 0, 0, 0, 0 ) );

    //! Copy constructor
    QgsColorRampTransformer( const QgsColorRampTransformer &other );

    QgsColorRampTransformer &operator=( const QgsColorRampTransformer &other );

    Type transformerType() const override { return ColorRampTransformer; }
    QgsColorRampTransformer *clone() const override SIP_FACTORY;
    QVariant toVariant() const override;
    bool loadVariant( const QVariant &definition ) override;
    QVariant transform( const QgsExpressionContext &context, const QVariant &value ) const override;
    QString toExpression( const QString &baseExpression ) const override;

    /**
     * Calculates the color corresponding to a specific value.
     * \param value value to calculate color for
     * \returns calculated color using transformer's parameters and type
     */
    QColor color( double value ) const;

    /**
     * Returns the color ramp used for calculating property colors.
     * \returns color ramp
     * \see setColorRamp()
     */
    QgsColorRamp *colorRamp() const;

    /**
     * Sets the color ramp to use for calculating property colors.
     * \param ramp color ramp, ownership of ramp is transferred to the transformer.
     * \see colorRamp()
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Returns the color corresponding to a null value.
     * \see setNullColor()
     */
    QColor nullColor() const { return mNullColor; }

    /**
     * Sets the color corresponding to a null value.
     * \param color null color
     * \see nullColor()
     */
    void setNullColor( const QColor &color ) { mNullColor = color; }

    /**
     * Returns the color ramp's name.
     * \see setRampName()
     */
    QString rampName() const { return mRampName; }

    /**
     * Sets the color ramp's \a name. The ramp name must be set to match
     * a color ramp available in the style database for conversion to expression
     * to work correctly.
     * \see rampName()
     */
    void setRampName( const QString &name ) { mRampName = name; }

  private:

    std::unique_ptr< QgsColorRamp > mGradientRamp;
    QColor mNullColor;
    QString mRampName;

};

#endif // QGSPROPERTYTRANSFORMER_H
