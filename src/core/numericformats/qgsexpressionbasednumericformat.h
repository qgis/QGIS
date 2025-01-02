/***************************************************************************
                             qgsexpressionbasednumericformat.h
                             --------------------------
    begin                : August 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXPRESSIONBASEDNUMERICFORMAT_H
#define QGSEXPRESSIONBASEDNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsnumericformat.h"
#include "qgsexpression.h"

/**
 * \ingroup core
 * \brief A numeric formatter which uses a QgsExpression to calculate the text representation of a value.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsExpressionBasedNumericFormat : public QgsNumericFormat
{
  public:

    QgsExpressionBasedNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Sets the \a expression used to calculate the text representation of a value.
     *
     * The expression can utilize `@value` to retrieve the numeric value to represent.
     *
     * \see expression()
     */
    void setExpression( const QString &expression ); // cppcheck-suppress functionConst

    /**
     * Returns the expression used to calculate the text representation of a value.
     *
     * \see setExpression()
     */
    QString expression() const { return mExpression.expression(); }

  private:

    mutable QgsExpression mExpression;
};

#endif // QGSEXPRESSIONBASEDNUMERICFORMAT_H
