/***************************************************************************
                             qgsscientificnumericformat.h
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
#ifndef QGSSCIENTIFICNUMERICFORMAT_H
#define QGSSCIENTIFICNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbasicnumericformat.h"

/**
 * \ingroup core
 * A numeric formatter which returns a scientific notation representation of a value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsScientificNumericFormat : public QgsBasicNumericFormat
{
  public:

    /**
      * Default constructor
      */
    QgsScientificNumericFormat();

    //! QgsScientificNumericFormat cannot be copied
    QgsScientificNumericFormat( const QgsScientificNumericFormat & ) = delete;
    //! QgsScientificNumericFormat cannot be copied
    QgsScientificNumericFormat &operator=( const QgsScientificNumericFormat & ) = delete;

    QString id() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration ) const override SIP_FACTORY;
    QVariantMap configuration() const override;

    /**
     * Sets the maximum number of decimal \a places to show.
     *
     * The \a places argument must be at least 1.
     *
     * \see numberDecimalPlaces()
     * \see setShowTrailingZeros()
     */
    void setNumberDecimalPlaces( int places ) override;

  private:

#ifdef SIP_RUN
    QgsScientificNumericFormat( const QgsScientificNumericFormat &other );
#endif

};

#endif // QGSSCIENTIFICNUMERICFORMAT_H
