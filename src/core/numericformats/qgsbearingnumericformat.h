/***************************************************************************
                             qgsbearingnumericformat.h
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
#ifndef QGSBEARINGNUMERICFORMAT_H
#define QGSBEARINGNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbasicnumericformat.h"

/**
 * \ingroup core
 * \brief A numeric formatter which returns a text representation of a direction/bearing.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsBearingNumericFormat : public QgsBasicNumericFormat
{
  public:

    /**
     * Directional formatting option, which controls how bearing direction is
     * described in the returned strings.
     */
    enum FormatDirectionOption
    {
      UseRange0To180WithEWDirectionalSuffix = 0, //!< Return values between 0 and 180, with a E or W directional suffix
      UseRangeNegative180ToPositive180, //!< Return values between -180 and 180
      UseRange0To360, //!< Return values between 0 to 360
    };

    /**
      * Default constructor
      */
    QgsBearingNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    double suggestSampleValue() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsBearingNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the directional formatting option, which controls how bearing direction is
     * described in the returned strings.
     *
     * \see setDirectionFormat()
     */
    FormatDirectionOption directionFormat() const;

    /**
     * Sets the directional formatting option, which controls how bearing direction is
     * described in the returned strings.
     *
     * \see directionFormat()
     */
    void setDirectionFormat( FormatDirectionOption format );

    void setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context ) override;

  private:

    FormatDirectionOption mDirectionFormat = UseRange0To180WithEWDirectionalSuffix;

};

#endif // QGSBEARINGNUMERICFORMAT_H
