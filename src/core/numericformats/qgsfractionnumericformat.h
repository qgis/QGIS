/***************************************************************************
                             qgsfractionnumericformat.h
                             --------------------------
    begin                : March 2020
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
#ifndef QGSFRACTIONNUMERICFORMAT_H
#define QGSFRACTIONNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsnumericformat.h"
#include <cmath>

/**
 * \ingroup core
 * \brief A numeric formatter which returns a vulgar fractional representation of a decimal value (e.g. "1/2" instead of 0.5).
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsFractionNumericFormat : public QgsNumericFormat
{
  public:

    /**
      * Default constructor
      */
    QgsFractionNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;
    double suggestSampleValue() const override;

    /**
     * Returns TRUE if dedicated unicode characters should be used, when the are available for the
     * particular fraction (e.g. ½, ¼).
     * \see setUseDedicatedUnicodeCharacters()
     * \see useUnicodeSuperSubscript()
     */
    bool useDedicatedUnicodeCharacters() const;

    /**
     * Sets whether dedicated unicode characters should be used, when the are available for the
     * particular fraction (e.g. ½, ¼).
     * \see useDedicatedUnicodeCharacters()
     * \see setUseUnicodeSuperSubscript()
     */
    void setUseDedicatedUnicodeCharacters( bool enabled );

    /**
     * Returns TRUE if unicode superscript and subscript characters should be used, (e.g. "⁶/₇").
     * \see setUseUnicodeSuperSubscript()
     * \see useDedicatedUnicodeCharacters()
     */
    bool useUnicodeSuperSubscript() const;

    /**
     * Sets whether unicode superscript and subscript characters should be used, (e.g. "⁶/₇").
     * \see useUnicodeSuperSubscript()
     * \see setUseDedicatedUnicodeCharacters()
     */
    void setUseUnicodeSuperSubscript( bool enabled );

    /**
     * Returns TRUE if the thousands grouping separator will be shown.
     * \see setShowThousandsSeparator()
     */
    bool showThousandsSeparator() const;

    /**
     * Sets whether the thousands grouping separator will be shown.
     * \see showThousandsSeparator()
     */
    void setShowThousandsSeparator( bool show );

    /**
     * Returns TRUE if a leading plus sign will be shown for positive values.
     * \see setShowPlusSign()
     */
    bool showPlusSign() const;

    /**
     * Sets whether a leading plus sign will be shown for positive values.
     * \see showPlusSign()
     */
    void setShowPlusSign( bool show );

    /**
     * Returns any override for the thousands separator character. If an invalid QChar is returned,
     * then the QGIS locale separator is used instead.
     *
     * \see setThousandsSeparator()
     */
    QChar thousandsSeparator() const;

    /**
     * Sets an override \a character for the thousands separator character. If an invalid QChar is set,
     * then the QGIS locale separator is used instead.
     *
     * \see thousandsSeparator()
     */
    void setThousandsSeparator( QChar character );

    /**
     * Converts a double \a value to a vulgar fraction (e.g. ⅓, ¼, etc) by attempting to calculate
     * the corresponding \a numerator and \a denominator, within the specified \a tolerance.
     *
     * This method is based of Richard's algorithm (1981) from "Continued Fractions without Tears" (University of Minnesota).
     *
     * \param value input value to convert
     * \param numerator will be set to calculated fraction numerator
     * \param denominator will be set to the calculated fraction denominator
     * \param sign will be set to the sign of the result (as -1 or +1 values)
     * \param tolerance acceptable tolerance. Larger values will give "nicer" fractions.
     * \returns TRUE if \a value was successfully converted to a fraction
     */
    static bool doubleToVulgarFraction( const double value, unsigned long long &numerator SIP_OUT, unsigned long long &denominator SIP_OUT, int &sign SIP_OUT, const double tolerance = 1e-10 )
    {
      sign = value < 0 ? -1 : 1;
      double g = std::fabs( value );
      unsigned long long a = 0;
      unsigned long long b = 1;
      unsigned long long c = 1;
      unsigned long long d = 0;
      unsigned long long s;
      unsigned int iteration = 0;
      do
      {
        s = std::floor( g );
        numerator = a + s * c;
        denominator = b + s * d;
        a = c;
        b = d;
        c = numerator;
        d = denominator;
        g = 1.0 / ( g - s );
        if ( qgsDoubleNear( static_cast< double >( sign )*static_cast< double >( numerator ) / denominator, value, tolerance ) )
        {
          return true;
        }
      }
      while ( iteration++ < 100 ); // limit to 100 iterations, should be sufficient for realistic purposes
      return false;
    }

    /**
     * Converts numbers in an \a input string to unicode superscript equivalents.
     * \see toUnicodeSubscript()
     */
    static QString toUnicodeSuperscript( const QString &input );

    /**
     * Converts numbers in an \a input string to unicode subscript equivalents.
     * \see toUnicodeSuperscript()
     */
    static QString toUnicodeSubscript( const QString &input );

  protected:

    /**
     * Sets the format's \a configuration.
     */
    virtual void setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context );

  private:

    bool mUseDedicatedUnicode = false;
    bool mUseUnicodeSuperSubscript = true;
    bool mShowThousandsSeparator = true;
    bool mShowPlusSign = false;
    QChar mThousandsSeparator;
};

#endif // QGSFRACTIONNUMERICFORMAT_H
