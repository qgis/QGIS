/***************************************************************************
                             qgsnumericformat.h
                             -------------------
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
#ifndef QGSNUMERICFORMAT_H
#define QGSNUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>
#include <QVariantMap>

/**
 * \ingroup core
 * A context for numeric formats
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsNumericFormatContext
{
  public:

    /**
     * Constructor for QgsNumericFormatContext.
     *
     * The context will be populated based on the user's current locale settings.
     */
    QgsNumericFormatContext();


    /**
     * Returns the thousands separator character.
     *
     * \see setThousandsSeparator()
     */
    QChar thousandsSeparator() const
    {
      return mThousandsSep;
    }

    /**
     * Sets the thousands \a separator character.
     *
     * \see thousandsSeparator()
     */
    void setThousandsSeparator( const QChar &separator )
    {
      mThousandsSep = separator;
    }

    /**
     * Returns the decimal separator character.
     *
     * \see setDecimalSeparator()
     */
    QChar decimalSeparator() const
    {
      return mDecimalSep;
    }

    /**
     * Returns the decimal separator character.
     *
     * \see setDecimalSeparator()
     */
    void setDecimalSeparator( const QChar &separator )
    {
      mDecimalSep = separator;
    }

  private:
    QChar mThousandsSep;
    QChar mDecimalSep;
};

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsbasicnumericformat.h>
#include <qgsbearingnumericformat.h>
#include <qgscurrencynumericformat.h>
#include <qgsfallbacknumericformat.h>
#include <qgspercentagenumericformat.h>
#include <qgsscientificnumericformat.h>
% End
#endif

/**
 * \ingroup core
 * A numeric formatter allows for formatting a numeric value for display, using
 * a variety of different formatting techniques (e.g. as scientific notation, currency values,
 * percentage values, etc)
 *
 * This is an abstract base class and will always need to be subclassed.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsNumericFormat
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsBasicNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsBasicNumericFormat;
    else if ( dynamic_cast< QgsBearingNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsBearingNumericFormat;
    else if ( dynamic_cast< QgsFallbackNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsFallbackNumericFormat;
    else if ( dynamic_cast< QgsPercentageNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsPercentageNumericFormat;
    else if ( dynamic_cast< QgsScientificNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsScientificNumericFormat;
    else if ( dynamic_cast< QgsCurrencyNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsCurrencyNumericFormat;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
      * Default constructor
      */
    QgsNumericFormat() = default;

    virtual ~QgsNumericFormat() = default;

    /**
     * Returns a unique id for this numeric format.
     *
     * This id is used to identify this numeric format in the registry with QgsNumericFormatRegistry::format().
     */
    virtual QString id() const = 0;

    /**
     * Returns a formatted string representation of a numeric double value.
     */
    virtual QString formatDouble( double value, const QgsNumericFormatContext &context ) const = 0;

    /**
     * Clones the format, returning a new object.
     *
     * The caller takes ownership of the returned object.
     */
    virtual QgsNumericFormat *clone() const = 0 SIP_FACTORY;

    /**
     * Creates a new copy of the format, using the supplied \a configuration.
     *
     * The caller takes ownership of the returned object.
     */
    virtual QgsNumericFormat *create( const QVariantMap &configuration ) const = 0 SIP_FACTORY;

    /**
     * Returns the current configuration of the formatter. This value can be used in a call to create()
     * in order to recreate this formatter in its current state.
     */
    virtual QVariantMap configuration() const = 0;
};

#endif // QGSNUMERICFORMAT_H
