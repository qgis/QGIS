/***************************************************************************
                             qgssymbolconverter.h
                             -----------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLCONVERTER_H
#define QGSSYMBOLCONVERTER_H

#include "qgis.h"
#include "qgis_core.h"

class QgsSymbol;

/**
 * \class QgsSymbolConverterContext
 * \ingroup core
 * \brief Represents the context in which a QgsSymbolConverter conversion occurs.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSymbolConverterContext
{
  public:

    QgsSymbolConverterContext();

};

/**
 * \class QgsAbstractSymbolConverter
 * \ingroup core
 * \brief An abstract base class for converting between QgsSymbol objects and QVariant representations.
 *
 * This interface provides a standard mechanism for serializing QGIS symbols to a generic
 * data container (based on QVariant) and deserializing them back into native QgsSymbol objects.
 *
 * Concrete implementations of this class are intended to handle specific external styling
 * formats, such as SLD (Styled Layer Descriptor) or Mapbox GL styles.
 *
 * \since QGIS 4.2
*/
class CORE_EXPORT QgsAbstractSymbolConverter
{

  public:
    virtual ~QgsAbstractSymbolConverter();

    /**
     * Returns the capabilities of the converter.
     */
    virtual Qgis::SymbolConverterCapabilities capabilities() const = 0;

    /**
     * Converts a \a symbol into a QVariant representation.
     *
     * The resulting QVariant may be a dictionary (QVariantMap), a JSON string,
     * an XML representation of the symbol, or some binary QByteArray value,
     * depending on the concrete implementation (e.g., SLD vs. Mapbox GL).
     *
     * Returns an invalid variant if the \a symbol could not be converted,
     * or if serialization of symbols is not supported for this
     * converter.
     */
    virtual QVariant toVariant( const QgsSymbol *symbol, QgsSymbolConverterContext &context ) const = 0;

    /**
     * Creates a new QgsSymbol from a QVariant representation.
     *
     * \param variant The QVariant containing the serialized symbol data.
     * \param context conversion context
     *
     * \returns A new QgsSymbol instance representing the data in the variant.
     * The caller takes ownership of the returned object. Returns NULLPTR if the
     * variant cannot be converted or parsed, or if the converter does not support
     * deserialization of symbols.
     */
    virtual std::unique_ptr< QgsSymbol > createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const = 0;

};

#endif //QGSSYMBOLCONVERTER_H
