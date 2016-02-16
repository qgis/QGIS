/***************************************************************************
 qgsscaleexpression.h
 ---------------------
 begin                : November 2014
 copyright            : (C) 2014 by Vincent Mora
 email                : vincent dor mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEEXPRESSION_H
#define QGSSCALEEXPRESSION_H

#include "qgsexpression.h"

/** \ingroup core
* \class QgsScaleExpression
* \brief Class storing parameters of a scale expression, which is a subclass of
* QgsExpression for expressions which return a size or width.
* \note Added in version 2.9
*/

class CORE_EXPORT QgsScaleExpression : public QgsExpression
{
  public:

    enum Type
    {
      Linear,
      Area,
      Flannery,
      Exponential,
      Unknown
    };

    /** Constructor for QgsScaleExpression which parses an expression string
     * to determine whether it's a scale expression
     * @param expression expression string
     */
    QgsScaleExpression( const QString &expression );

    /** Constructor for QgsScaleExpression which creates an expression from
     * specified parameters
     * @param type scale method
     * @param baseExpression expression (or field) used for value
     * @param minValue minimum value, corresponds to specified minimum size
     * @param maxValue maximum value, corresponds to specified maximum size
     * @param minSize minimum size
     * @param maxSize maximum size
     * @param nullSize size in case expression evaluates to NULL
     * @param exponent to use in case of Exponential type
     */
    QgsScaleExpression( Type type, const QString& baseExpression, double minValue, double maxValue, double minSize, double maxSize, double nullSize = 0, double exponent = 1 );

    operator bool() const { return ! mExpression.isEmpty(); }

    /** Calculates the size corresponding to a specific value.
     * @param value
     * @returns calculated size using expression's parameters and type
     */
    double size( double value ) const;

    /** Returns the minimum size calculated by the expression
     * @see maxSize
     */
    double minSize() const { return mMinSize; }

    /** Returns the maximum size calculated by the expression
     * @see minSize
     */
    double maxSize() const { return mMaxSize; }

    /** Returns the minimum value expected by the expression. The minimum
     * value corresponds to the expression's minimum size.
     * @see maxValue
     */
    double minValue() const { return mMinValue; }

    /** Returns the maximum value expected by the expression. The maximum
     * value corresponds to the expression's maximum size.
     * @see minValue
     */
    double maxValue() const { return mMaxValue; }

    /** Returns the size value when expression evaluates to NULL.
     * @see nullSize
     */
    double nullSize() const { return mNullSize; }

    /** Returns the exponent of the exponential expression.
     * @see exponent
     */
    double exponent() const { return mExponent; }

    /** Returns the base expression string (or field reference) used for
     * calculating the values to be mapped to a size.
     */
    QString baseExpression() const { return mExpression; }

    /** Returns the scale expression's type (method used to calculate
     * the size from a value).
     */
    Type type() const { return mType; }

  private:
    QString mExpression;
    Type mType;
    double mMinSize;
    double mMaxSize;
    double mMinValue;
    double mMaxValue;
    double mNullSize;
    double mExponent;

    void init();
    static QString createExpression( Type type, const QString& baseExpr, double minValue, double maxValue, double minSize, double maxSize, double nullSize, double exponent );

};

#endif



