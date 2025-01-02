/***************************************************************************
                             qgscolorutils.h
                             ---------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#ifndef QGSCOLORUTILS_H
#define QGSCOLORUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QDomDocument>
#include <QDomElement>
#include <QColorSpace>

class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsColorUtils
 * \brief Contains utility functions for working with colors.
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsColorUtils
{
  public:

    /**
     * Writes a \a color to an XML \a element, storing it under the specified \a identifier.
     *
     * This method losslessly stores a color's definition in an XML \a element. All properties
     * of the color are stored, including the color specification and original values of the
     * color's components. It is therefore suitable for storing high color depth colors (such
     * as 16 bit colors), or colors using alternative specifications such as CMYK colors.
     *
     * The \a identifier string is used to specify the element name for the stored color,
     * allowing for multiple color definitions to be stored in a single \a element (assuming
     * each uses a unique identifier string).
     *
     * \see readXml()
     */
    static void writeXml( const QColor &color, const QString &identifier,
                          QDomDocument &document, QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Reads a color from an XML \a element, matching the specified \a identifier string.
     *
     * This method losslessly retrieves a color's definition from an XML element. All properties
     * of the color are restored, including the color specification and original values of the
     * color's components. It is therefore suitable for restoring high color depth colors (such
     * as 16 bit colors), or colors using alternative specifications such as CMYK colors.
     *
     * An invalid color will be returned if the color could not be read.
     *
     * \see writeXml()
     */
    static QColor readXml( const QDomElement &element, const QString &identifier, const QgsReadWriteContext &context );

    /**
     * Encodes a \a color into a string value.
     *
     * This method losslessly stores a color's definition into a single string value. All properties
     * of the color are stored, including the color specification and original values of the
     * color's components. It is therefore suitable for storing high color depth colors (such
     * as 16 bit colors), or colors using alternative specifications such as CMYK colors.
     *
     * \see colorFromString()
     */
    static QString colorToString( const QColor &color );

    /**
     * Decodes a \a string into a color value.
     *
     * This method losslessly retrieves a color's definition from a string value. All properties
     * of the color are restored, including the color specification and original values of the
     * color's components. It is therefore suitable for restoring high color depth colors (such
     * as 16 bit colors), or colors using alternative specifications such as CMYK colors.
     *
     * An invalid color will be returned if the color could not be read.
     *
     * \see colorToString()
     */
    static QColor colorFromString( const QString &string );

    /**
     * Loads an ICC profile from \a iccProfileFilePath and returns associated color space.
     * If an error occurred, an invalid color space is returned and \a errorMsg is updated with error
     * message
     *
     * \param iccProfileFilePath ICC profile file path
     * \param errorMsg Will be set to a user-friendly message if an error occurs while loading the ICC profile file
     * \returns loaded color space
     *
     * \since QGIS 3.40
     */
    static QColorSpace iccProfile( const QString &iccProfileFilePath, QString &errorMsg SIP_OUT );

    /**
     * Save color space \a colorSpace to an ICC profile file \a iccProfileFilePath.
     * \returns error message if an error occurred else empty string.
     *
     * \since QGIS 3.40
     */
    static QString saveIccProfile( const QColorSpace &colorSpace, const QString &iccProfileFilePath );


#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)

    /**
     * Convert and returns Qt \a colorModel to Qgis::ColorModel. \a ok is set to true if \a colorModel
     * is a valid Qgis::ColorModel.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.40
     */
    static Qgis::ColorModel toColorModel( QColorSpace::ColorModel colorModel, bool *ok = nullptr ) SIP_SKIP;

#endif

};

#endif // QGSCOLORUTILS_H
