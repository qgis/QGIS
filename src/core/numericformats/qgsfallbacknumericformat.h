/***************************************************************************
                             qgsfallbacknumericformat.h
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
#ifndef QGSFALLBACKNUMERICFORMAT_H
#define QGSFALLBACKNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsnumericformat.h"

/**
 * \ingroup core
 * \brief A basic numeric formatter which returns a simple text representation of a value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsFallbackNumericFormat : public QgsNumericFormat
{
  public:

    QgsFallbackNumericFormat() = default;
    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;
};

#endif // QGSFALLBACKNUMERICFORMAT_H
