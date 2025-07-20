/***************************************************************************
                             qgscoordinatenumericformat.h
                             --------------------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATENUMERICFORMAT_H
#define QGSCOORDINATENUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbasicnumericformat.h"

/**
 * \ingroup core
 * \brief A numeric formatter which returns a text representation of a geographic coordinate (latitude or longitude).
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsGeographicCoordinateNumericFormat : public QgsBasicNumericFormat
{
    Q_GADGET

  public:

    /**
     * Angle format options.
     */
    enum class AngleFormat
    {
      DegreesMinutesSeconds, //!< Degrees, minutes and seconds, eg 30 degrees 45'30
      DegreesMinutes, //!< Degrees and decimal minutes, eg 30 degrees 45.55'
      DecimalDegrees, //!< Decimal degrees, eg 30.7555 degrees
    };
    Q_ENUM( AngleFormat )

    /**
      * Default constructor
      */
    QgsGeographicCoordinateNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    double suggestSampleValue() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsGeographicCoordinateNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the angle format, which controls how bearing the angles are formatted
     * described in the returned strings.
     *
     * \see setAngleFormat()
     */
    AngleFormat angleFormat() const;

    /**
     * Sets the directional formatting option, which controls how bearing the angles are formatted
     * described in the returned strings.
     *
     * \see angleFormat()
     */
    void setAngleFormat( AngleFormat format );

    /**
     * Returns TRUE if leading zeros in the minutes or seconds values should be shown.
     *
     * \see setShowLeadingZeros()
     */
    bool showLeadingZeros() const;

    /**
     * Sets whether leading zeros in the minutes or seconds values should be shown.
     *
     * \see showLeadingZeros()
     */
    void setShowLeadingZeros( bool show );

    /**
     * Returns TRUE if leading zeros for the degree values should be shown.
     *
     * \see setShowDegreeLeadingZeros()
     */
    bool showDegreeLeadingZeros() const;

    /**
     * Sets whether leading zeros for the degree values should be shown.
     *
     * \see showDegreeLeadingZeros()
     */
    void setShowDegreeLeadingZeros( bool show );

    /**
     * Returns TRUE if directional suffixes (e.g. "N") should be included.
     *
     * \see setShowDirectionalSuffix()
     */
    bool showDirectionalSuffix() const;

    /**
     * Sets whether directional suffixes (e.g. "N") should be included.
     *
     * \see showDirectionalSuffix()
     */
    void setShowDirectionalSuffix( bool show );

    void setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context ) override;

  private:

    AngleFormat mAngleFormat = AngleFormat::DecimalDegrees;
    bool mShowLeadingZeros = false;
    bool mShowLeadingDegreeZeros = false;
    bool mUseSuffix = true;

    QString formatLongitude( double value, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;
    QString formatLatitude( double value, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;

    QString formatLatitudeAsDegreesMinutesSeconds( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;
    QString formatLongitudeAsDegreesMinutesSeconds( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;

    QString formatLatitudeAsDegreesMinutes( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;
    QString formatLongitudeAsDegreesMinutes( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;

    QString formatLatitudeAsDegrees( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;
    QString formatLongitudeAsDegrees( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const;

    void trimTrailingZeros( QString &input, const QgsNumericFormatContext &context ) const;

};

#endif // QGSCOORDINATENUMERICFORMAT_H
