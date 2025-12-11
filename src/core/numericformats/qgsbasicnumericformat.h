/***************************************************************************
                             qgsbasicnumericformat.h
                             --------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBASICNUMERICFORMAT_H
#define QGSBASICNUMERICFORMAT_H

#include <iostream>
#include <memory>
#include <sstream>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsnumericformat.h"

/**
 * \ingroup core
 * \brief A numeric formatter which returns a simple text representation of a value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsBasicNumericFormat : public QgsNumericFormat
{
  public:

    /**
     * Sets rounding type and behavior of the numberDecimalPlaces() setting.
     */
    enum RoundingType
    {
      DecimalPlaces, //!< Maximum number of decimal places
      SignificantFigures, //!< Maximum number of significant figures
    };

    /**
      * Default constructor
      */
    QgsBasicNumericFormat();

    [[nodiscard]] QString id() const override;
    [[nodiscard]] QString visibleName() const override;
    int sortKey() override;
    [[nodiscard]] QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    [[nodiscard]] QgsNumericFormat *clone() const override SIP_FACTORY;
    [[nodiscard]] QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    [[nodiscard]] QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the maximum number of decimal places to show.
     *
     * \see setNumberDecimalPlaces()
     * \see showTrailingZeros()
     */
    [[nodiscard]] int numberDecimalPlaces() const;

    /**
     * Sets the maximum number of decimal \a places to show.
     *
     * \see numberDecimalPlaces()
     * \see setShowTrailingZeros()
     */
    virtual void setNumberDecimalPlaces( int places );

    /**
     * Returns TRUE if the thousands grouping separator will be shown.
     * \see setShowThousandsSeparator()
     */
    [[nodiscard]] bool showThousandsSeparator() const;

    /**
     * Sets whether the thousands grouping separator will be shown.
     * \see showThousandsSeparator()
     */
    void setShowThousandsSeparator( bool show );

    /**
     * Returns TRUE if a leading plus sign will be shown for positive values.
     * \see setShowPlusSign()
     */
    [[nodiscard]] bool showPlusSign() const;

    /**
     * Sets whether a leading plus sign will be shown for positive values.
     * \see showPlusSign()
     */
    void setShowPlusSign( bool show );

    /**
     * Returns TRUE if trailing zeros will be shown (up to the specified
     * numberDecimalPlaces()).
     *
     * \see setShowTrailingZeros()
     * \see numberDecimalPlaces()
     */
    [[nodiscard]] bool showTrailingZeros() const;

    /**
     * Sets whether trailing zeros will be shown (up to the specified
     * numberDecimalPlaces()).
     *
     * \see showTrailingZeros()
     * \see setNumberDecimalPlaces()
     */
    void setShowTrailingZeros( bool show );

    /**
     * Returns the rounding type, which controls the behavior of the numberDecimalPlaces() setting.
     *
     * \see setRoundingType()
     */
    [[nodiscard]] RoundingType roundingType() const;

    /**
     * Sets the rounding \a type, which controls the behavior of the numberDecimalPlaces() setting.
     *
     * \see roundingType()
     */
    void setRoundingType( RoundingType type );

    /**
     * Returns any override for the thousands separator character. If an invalid QChar is returned,
     * then the QGIS locale separator is used instead.
     *
     * \see setThousandsSeparator()
     */
    [[nodiscard]] QChar thousandsSeparator() const;

    /**
     * Sets an override \a character for the thousands separator character. If an invalid QChar is set,
     * then the QGIS locale separator is used instead.
     *
     * \see thousandsSeparator()
     */
    void setThousandsSeparator( QChar character );

    /**
     * Returns any override for the decimal separator character. If an invalid QChar is returned,
     * then the QGIS locale separator is used instead.
     *
     * \see setDecimalSeparator()
     */
    [[nodiscard]] QChar decimalSeparator() const;

    /**
     * Sets an override \a character for the decimal separator character. If an invalid QChar is set,
     * then the QGIS locale separator is used instead.
     *
     * \see decimalSeparator()
     */
    void setDecimalSeparator( QChar character );

  protected:

    /**
     * Sets the format's \a configuration.
     */
    virtual void setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context );

    bool mUseScientific = false;

  private:

    int mNumberDecimalPlaces = 6;
    bool mShowThousandsSeparator = true;
    bool mShowPlusSign = false;
    bool mShowTrailingZeros = false;

    RoundingType mRoundingType = DecimalPlaces;

    QChar mThousandsSeparator;
    QChar mDecimalSeparator;
};

#endif // QGSBASICNUMERICFORMAT_H
