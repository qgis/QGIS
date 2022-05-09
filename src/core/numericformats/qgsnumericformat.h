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
#include <QDomDocument>


class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief A context for numeric formats
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsNumericFormatContext
{
    Q_GADGET

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

    /**
     * Returns the percent character.
     *
     * \see setPercent()
     */
    QChar percent() const
    {
      return mPercent;
    }

    /**
     * Sets the percent \a character.
     *
     * \see percent()
     */
    void setPercent( const QChar &character )
    {
      mPercent = character;
    }

    /**
     * Returns the zero digit character.
     *
     * \see setZeroDigit()
     */
    QChar zeroDigit() const
    {
      return mZeroDigit;
    }

    /**
     * Returns the zero digit \a character.
     *
     * \see zeroDigit()
     */
    void setZeroDigit( const QChar &character )
    {
      mZeroDigit = character;
    }

    /**
     * Returns the negative sign character.
     *
     * \see setNegativeSign()
     */
    QChar negativeSign() const
    {
      return mNegativeSign;
    }

    /**
     * Sets the negative sign \a character.
     *
     * \see negativeSign()
     */
    void setNegativeSign( const QChar &character )
    {
      mNegativeSign = character;
    }

    /**
     * Returns the positive sign character.
     *
     * \see setPositiveSign()
     */
    QChar positiveSign() const
    {
      return mPositiveSign;
    }

    /**
     * Sets the positive sign \a character.
     *
     * \see positiveSign()
     */
    void setPositiveSign( const QChar &character )
    {
      mPositiveSign = character;
    }

    /**
     * Returns the exponential character.
     *
     * \see setExponential()
     */
    QChar exponential() const
    {
      return mExponential;
    }

    /**
     * Sets the exponential \a character.
     *
     * \see exponential()
     */
    void setExponential( const QChar &character )
    {
      mExponential = character;
    }

    /**
     * Interpretation of numeric values.
     *
     * \since QGIS 3.26
     */
    enum class Interpretation
    {
      Generic, //!< Generic
      Latitude, //!< Latitude values
      Longitude, //!< Longitude values
    };
    Q_ENUM( Interpretation )

    /**
     * Returns the interpretation of the numbers being converted.
     *
     * \see setInterpretation()
     *
     * \since QGIS 3.26
     */
    Interpretation interpretation() const
    {
      return mInterpretation;
    }

    /**
     * Sets the \a interpretation of the numbers being converted.
     *
     * \see interpretation()
     *
     * \since QGIS 3.26
     */
    void setInterpretation( Interpretation interpretation )
    {
      mInterpretation = interpretation;
    }

  private:
    QChar mThousandsSep;
    QChar mDecimalSep;
    QChar mPercent;
    QChar mZeroDigit;
    QChar mNegativeSign;
    QChar mPositiveSign;
    QChar mExponential;

    Interpretation mInterpretation = Interpretation::Generic;
};

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsbasicnumericformat.h>
#include <qgsbearingnumericformat.h>
#include <qgscurrencynumericformat.h>
#include <qgsfallbacknumericformat.h>
#include <qgspercentagenumericformat.h>
#include <qgsscientificnumericformat.h>
#include <qgscoordinatenumericformat.h>
% End
#endif

/**
 * \ingroup core
 * \brief A numeric formatter allows for formatting a numeric value for display, using
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
    if ( dynamic_cast< QgsBearingNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsBearingNumericFormat;
    else if ( dynamic_cast< QgsGeographicCoordinateNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsGeographicCoordinateNumericFormat;
    else if ( dynamic_cast< QgsFallbackNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsFallbackNumericFormat;
    else if ( dynamic_cast< QgsPercentageNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsPercentageNumericFormat;
    else if ( dynamic_cast< QgsScientificNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsScientificNumericFormat;
    else if ( dynamic_cast< QgsCurrencyNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsCurrencyNumericFormat;
    else if ( dynamic_cast< QgsBasicNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsBasicNumericFormat;
    else if ( dynamic_cast< QgsFractionNumericFormat * >( sipCpp ) )
      sipType = sipType_QgsFractionNumericFormat;
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
     * Returns the translated, user-visible name for this format.
     */
    virtual QString visibleName() const = 0;

    /**
     * Returns a sorting key value, where formats with a lower sort key will be shown earlier in lists.
     *
     * Generally, subclasses should return QgsNumericFormat::sortKey() as their sorting key.
     */
    virtual int sortKey();

    /**
     * Returns a suggested sample value which nicely represents the current format configuration.
     */
    virtual double suggestSampleValue() const;

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
    virtual QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const = 0 SIP_FACTORY;

    /**
     * Returns the current configuration of the formatter. This value can be used in a call to create()
     * in order to recreate this formatter in its current state.
     */
    virtual QVariantMap configuration( const QgsReadWriteContext &context ) const = 0;

    /**
     * Writes the format to an XML \a element.
     * \see QgsNumericFormatRegistry::createFromXml()
     */
    void writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    bool operator==( const QgsNumericFormat &other ) const;
    bool operator!=( const QgsNumericFormat &other ) const;

  protected:

    static constexpr int DEFAULT_SORT_KEY = 100;
};

#endif // QGSNUMERICFORMAT_H
