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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsnumericformat.h"
#include <sstream>
#include <iostream>
#include <memory>

/**
 * \ingroup core
 * A numeric formatter which returns a simple text representation of a value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsBasicNumericFormat : public QgsNumericFormat
{
  public:

    /**
      * Default constructor
      */
    QgsBasicNumericFormat();

    //! QgsBasicNumericFormat cannot be copied
    QgsBasicNumericFormat( const QgsBasicNumericFormat & ) = delete;
    //! QgsBasicNumericFormat cannot be copied
    QgsBasicNumericFormat &operator=( const QgsBasicNumericFormat & ) = delete;

    QString id() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration ) const override SIP_FACTORY;
    QVariantMap configuration() const override;

    /**
     * Sets the format's \a configuration.
     */
    void setConfiguration( const QVariantMap &configuration );

    /**
     * Returns the maximum number of decimal places to show.
     *
     * \see setNumberDecimalPlaces()
     * \see showTrailingZeros()
     */
    int numberDecimalPlaces() const;

    /**
     * Sets the maximum number of decimal places to show.
     *
     * \see numberDecimalPlaces()
     * \see setShowTrailingZeros()
     */
    void setNumberDecimalPlaces( int numberDecimalPlaces );

    /**
     * Returns TRUE if the thousands grouping separator will be shown.
     * \see setShowThousandsSeparator()
     */
    bool showThousandsSeparator() const;

    /**
     * Sets whether the thousands grouping separator will be shown.
     * \see showThousandsSeparator()
     */
    void setShowThousandsSeparator( bool showThousandsSeparator );

    /**
     * Returns TRUE if a leading plus sign will be shown for positive values.
     * \see setShowPlusSign()
     */
    bool showPlusSign() const;

    /**
     * Sets whether a leading plus sign will be shown for positive values.
     * \see showPlusSign()
     */
    void setShowPlusSign( bool showPlusSign );

    /**
     * Returns TRUE if trailing zeros will be shown (up to the specified
     * numberDecimalPlaces()).
     *
     * \see setShowTrailingZeros()
     * \see numberDecimalPlaces()
     */
    bool showTrailingZeros() const;

    /**
     * Sets whether trailing zeros will be shown (up to the specified
     * numberDecimalPlaces()).
     *
     * \see showTrailingZeros()
     * \see setNumberDecimalPlaces()
     */
    void setShowTrailingZeros( bool showTrailingZeros );

  protected:

    bool mUseScientific = false;

  private:

#ifdef SIP_RUN
    QgsBasicNumericFormat( const QgsBasicNumericFormat &other );
#endif

    int mNumberDecimalPlaces = 6;
    bool mShowThousandsSeparator = true;
    bool mShowPlusSign = false;
    bool mShowTrailingZeros = false;
    mutable QChar mPrevThousandsSep;
    mutable QChar mPrevDecimalSep;
    mutable std::unique_ptr< std::ostringstream > mOs;
};

#endif // QGSBASICNUMERICFORMAT_H
