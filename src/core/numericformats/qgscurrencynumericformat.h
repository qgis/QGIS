/***************************************************************************
                             qgscurrencynumericformat.h
                             ----------------------------
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
#ifndef QGSCURRENCYNUMERICFORMAT_H
#define QGSCURRENCYNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbasicnumericformat.h"

/**
 * \ingroup core
 * \brief A numeric formatter which returns a text representation of a currency value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsCurrencyNumericFormat : public QgsBasicNumericFormat
{
  public:

    /**
      * Default constructor
      */
    QgsCurrencyNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    double suggestSampleValue() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the currency prefix, e.g. "$".
     * \see setPrefix()
     */
    QString prefix() const;

    /**
     * Sets the currency \a prefix, e.g. "$".
     * \see prefix()
     */
    void setPrefix( const QString &prefix );

    /**
     * Returns the currency suffix, e.g. "AUD".
     * \see setSuffix()
     */
    QString suffix() const;

    /**
     * Sets the currency \a suffix, e.g. "AUD".
     * \see suffix()
     */
    void setSuffix( const QString &suffix );

  private:

    QString mPrefix;
    QString mSuffix;

};

#endif // QGSCURRENCYNUMERICFORMAT_H
