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
#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QDomDocument>
#include <QColor>

class QgsColorRamp;

/**
 * \ingroup core
 * \class QgsPropertyTransformer
 * \brief Abstract base class for objects which transform the calculated value of a property.
 * Possible uses include transformers which map a value into a scaled size or color from a gradient.
 * \note Added in version 3.0
 */

class CORE_EXPORT QgsPropertyTransformer
{
  public:

    //! Transformer types
    enum Type
    {
      SizeScaleTransformer, //!< Size scaling transformer (QgsSizeScaleTransformer)
      ColorRampTransformer, //!< Color ramp transformer (QgsColorRampTransformer)
    };

    /**
     * Factory method for creating a new property transformer of the specified type.
     * @param type transformer type to create
     */
    static QgsPropertyTransformer* create( Type type );

    /**
     * Constructor for QgsPropertyTransformer
     * @param minValue minimum expected value from source property
     * @param maxValue maximum expected value from source property
     */
    QgsPropertyTransformer( double minValue = 0.0, double maxValue = 1.0 );

    virtual ~QgsPropertyTransformer() = default;

    /**
     * Returns the transformer type.
     */
    virtual Type transformerType() const = 0;

    /**
     * Returns a clone of the transformer.
     */
    virtual QgsPropertyTransformer* clone() = 0;

    /**
     * Reads transformer's state from an XML element.
     * @param transformerElem source DOM element for transformer's state
     * @param doc DOM document
     * @see writeXml()
    */
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc );

    /**
     * Writes the current state of the transformer into an XML element
     * @param transformerElem destination element for the transformer's state
     * @param doc DOM document
     * @see readXml()
    */
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const;

    /**
     * Returns the minimum value expected by the transformer.
     * @see maxValue()
     * @see setMinValue()
     */
    double minValue() const { return mMinValue; }

    /**
     * Sets the minimum value expected by the transformer.
     * @param min minimum value
     * @see setMaxValue()
     * @see minValue()
     */
    void setMinValue( double min ) { mMinValue = min; }

    /**
     * Returns the maximum value expected by the transformer.
     * @see minValue()
     * @see setMaxValue()
     */
    double maxValue() const { return mMaxValue; }

    /**
     * Sets the maximum value expected by the transformer.
     * @param max maximum value
     * @see setMinValue()
     * @see maxValue()
     */
    void setMaxValue( double max ) { mMaxValue = max; }

    /**
     * Calculates the transform of a value. Derived classes must implement this to perform their transformations
     * on input values
     * @param context expression context
     * @param value input value to transform
     */
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const = 0;

  protected:

    //! Minimum value expected by the transformer
    double mMinValue;

    //! Maximum value expected by the transformer
    double mMaxValue;

};


/**
 * \ingroup core
 * \class QgsSizeScaleTransformer
 * \brief QgsPropertyTransformer subclass for scaling a value into a size according to various
 * scaling methods.
 * \note Added in version 3.0
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
     * @param type scaling type
     * @param minValue minimum expected value
     * @param maxValue maximum expected value
     * @param minSize minimum size to return
     * @param maxSize maximum size to return
     * @param nullSize size to return for null values
     * @param exponent exponent for Exponential scaling method
     */
    QgsSizeScaleTransformer( ScaleType type = Linear,
                             double minValue = 0.0,
                             double maxValue = 1.0,
                             double minSize = 0.0,
                             double maxSize = 1.0,
                             double nullSize = 0.0,
                             double exponent = 1.0 );

    virtual Type transformerType() const override { return SizeScaleTransformer; }
    virtual QgsSizeScaleTransformer* clone() override;
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const override;
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc ) override;
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const override;

    /**
     * Calculates the size corresponding to a specific value.
     * @param value value to calculate size for
     * @returns calculated size using size scale transformer's parameters and type
     */
    double size( double value ) const;

    /**
     * Returns the minimum calculated size.
     * @see setMinSize()
     * @see maxSize()
     */
    double minSize() const { return mMinSize; }

    /**
     * Sets the minimum calculated size.
     * @param size minimum size
     * @see minSize()
     * @see setMaxSize()
     */
    void setMinSize( double size ) { mMinSize = size; }

    /**
     * Returns the maximum calculated size.
     * @see minSize()
     */
    double maxSize() const { return mMaxSize; }

    /**
     * Sets the maximum calculated size.
     * @param size maximum size
     * @see maxSize()
     * @see setMinSize()
     */
    void setMaxSize( double size ) { mMaxSize = size; }

    /**
     * Returns the size value when an expression evaluates to NULL.
     * @see setNullSize()
     */
    double nullSize() const { return mNullSize; }

    /**
     * Sets the size value for when an expression evaluates to NULL.
     * @param size null size
     * @see nullSize()
     */
    void setNullSize( double size ) { mNullSize = size; }

    /**
     * Returns the exponent for an exponential expression.
     * @see setExponent()
     * @see type()
     */
    double exponent() const { return mExponent; }

    /**
     * Sets the exponent for an exponential expression.
     * @param exponent exponent
     * @see exponent()
     */
    void setExponent( double exponent ) { mExponent = exponent; }

    /**
     * Returns the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * @see setType()
     */
    ScaleType type() const { return mType; }

    /**
     * Sets the size transformer's scaling type (the method used to calculate
     * the size from a value).
     * @param type scale type
     * @see type()
     */
    void setType( ScaleType type );

  private:
    ScaleType mType;
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
 * \note Added in version 3.0
 */

class CORE_EXPORT QgsColorRampTransformer : public QgsPropertyTransformer
{
  public:

    /**
     * Constructor for QgsColorRampTransformer.
     * @param minValue minimum expected value
     * @param maxValue maximum expected value
     * @param ramp source color ramp. Ownership is transferred to the transformer.
     * @param nullColor color to return for null values
     */
    QgsColorRampTransformer( double minValue = 0.0,
                             double maxValue = 1.0,
                             QgsColorRamp* ramp = nullptr,
                             const QColor& nullColor = QColor( 0, 0, 0, 0 ) );

    //! Copy constructor
    QgsColorRampTransformer( const QgsColorRampTransformer& other );

    QgsColorRampTransformer& operator=( const QgsColorRampTransformer& other );

    virtual Type transformerType() const override { return ColorRampTransformer; }
    virtual QgsColorRampTransformer* clone() override;
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const override;
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc ) override;
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const override;

    /**
     * Calculates the color corresponding to a specific value.
     * @param value value to calculate color for
     * @returns calculated color using transformer's parameters and type
     */
    QColor color( double value ) const;

    /**
     * Returns the color ramp used for calculating property colors.
     * @returns color ramp
     * @see setColorRamp()
     */
    QgsColorRamp* colorRamp() const;

    /**
     * Sets the color ramp to use for calculating property colors.
     * @param ramp color ramp, ownership of ramp is transferred to the transformer.
     * @see colorRamp()
     */
    void setColorRamp( QgsColorRamp* ramp );

    /**
     * Returns the color corresponding to a null value.
     * @see setNullColor()
     */
    QColor nullColor() const { return mNullColor; }

    /**
     * Sets the color corresponding to a null value.
     * @param color null color
     * @see nullSize()
     */
    void setNullColor( const QColor& color ) { mNullColor = color; }

  private:

    QScopedPointer< QgsColorRamp > mGradientRamp;
    QColor mNullColor;

};

#endif // QGSPROPERTYTRANSFORMER_H
