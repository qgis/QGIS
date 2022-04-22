/***************************************************************************
    qgsfontutils.h
    ---------------------
    begin                : June 5, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFONTUTILS_H
#define QGSFONTUTILS_H

#include <QFont>
#include <QString>
#include <QDomElement>

#include "qgis_core.h"
#include "qgis_sip.h"

class QMimeData;

/**
 * \ingroup core
 * \class QgsFontUtils
 */
class CORE_EXPORT QgsFontUtils
{
  public:

    /**
     * Check whether exact font is on system
     * \param f The font to test for match
     */
    static bool fontMatchOnSystem( const QFont &f );

    /**
     * Check whether font family is on system in a quick manner, which does not compare [foundry]
     * \param family The family to test
     * \returns Whether family was found on system
     * \note This is good for use in loops of large lists, e.g. registering many features for labeling
     */
    static bool fontFamilyOnSystem( const QString &family );

    /**
     * Check whether font family on system has specific style
     * \param family The family to test
     * \param style The style to test for
     * \returns Whether family has style
     * \since QGIS 2.1
     */
    static bool fontFamilyHasStyle( const QString &family, const QString &style );

    /**
     * Attempts to resolve the style name corresponding to the specified \a font object.
     *
     * If a font has been modified by calling QFont::setBold or QFont::setItalic, then its QFont::styleName
     * may return an empty string. This method attempts to determine the correct style name which
     * corresponds to the font's bold and italic settings.
     *
     * Returns an empty string if a matching style name could not be found.
     *
     * \since QGIS 3.26
     */
    static QString resolveFontStyleName( const QFont &font );

    /**
     * Check whether font family is on system
     * \param family The family to test
     * \param chosen The actual family (possibly from different foundry) returned by system
     * \param match Whether the family [foundry] returned by system is a match
     * \returns Whether family was found on system
     */
    static bool fontFamilyMatchOnSystem( const QString &family, QString *chosen = nullptr, bool *match = nullptr );

    /**
     * Updates font with named style and retain all font properties
     * \param f The font to update
     * \param fontstyle The style to try and switch the font to
     * \param fallback If no matching fontstyle found for font, assign most similar or first style found to font
     * \returns Whether the font was updated (also returns TRUE if the requested style matches font's current style)
     * \note This is a more featured replacement for a Qt 4.8+ function: void QFont::setStyleName ( const QString & styleName )
     */
    static bool updateFontViaStyle( QFont &f, const QString &fontstyle, bool fallback = false );

    /**
     * Gets standard test font family
     * \since QGIS 2.1
     */
    static QString standardTestFontFamily();

    /**
     * Loads standard test fonts from filesystem or qrc resource
     * \param loadstyles List of styles to load, e.g. All, Roman, Oblique, Bold, Bold Oblique
     * \returns Whether any font was loaded
     * \note Done by default on debug app/server startup to ensure fonts available for unit tests (Roman and Bold)
     * \since QGIS 2.1
     */
    static bool loadStandardTestFonts( const QStringList &loadstyles );

    /**
     * Gets standard test font with specific style
     * \param style Style to load, e.g. Roman, Oblique, Bold, Bold Oblique
     * \param pointsize Font point size to set
     * \returns QFont
     * \since QGIS 2.1
     */
    static QFont getStandardTestFont( const QString &style = "Roman", int pointsize = 12 );

    /**
     * Returns a DOM element containing the properties of the font.
     * \param font font
     * \param document DOM document
     * \param elementName name for DOM element
     * \returns DOM element containing font settings
     * \see setFromXmlElement
     * \since QGIS 2.10
     */
    static QDomElement toXmlElement( const QFont &font, QDomDocument &document, const QString &elementName );

    /**
     * Sets the properties of a font to match the properties stored in an XML element. Calling
     * this will overwrite the current properties of the font.
     * \param font font to update
     * \param element DOM element
     * \returns TRUE if properties were successfully read from element
     * \see toXmlElement
     * \see setFromXmlChildNode
     * \since QGIS 2.10
     */
    static bool setFromXmlElement( QFont &font, const QDomElement &element );

    /**
     * Sets the properties of a font to match the properties stored in an XML child node. Calling
     * this will overwrite the current properties of the font.
     * \param font font to update
     * \param element DOM element
     * \param childNode name of child node
     * \returns TRUE if child node exists and properties were successfully read from node
     * \see setFromXmlElement
     * \see toXmlElement
     * \since QGIS 2.10
     */
    static bool setFromXmlChildNode( QFont &font, const QDomElement &element, const QString &childNode );

    /**
     * Returns new mime data representing the specified \a font settings.
     * Caller takes responsibility for deleting the returned object.
     * \see fromMimeData()
     * \since QGIS 3.0
     */
    static QMimeData *toMimeData( const QFont &font ) SIP_FACTORY;

    /**
     * Attempts to parse the provided mime \a data as a QFont.
     * If data can be parsed as a QFont, \a ok will be set to TRUE.
     * \see toMimeData()
     * \since QGIS 3.0
     */
    static QFont fromMimeData( const QMimeData *data, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the localized named style of a font, if such a translation is available.
     * \param namedStyle a named style, i.e. "Bold", "Italic", etc
     * \returns The localized named style
     * \see untranslateNamedStyle
     * \since QGIS 2.12
     */
    static QString translateNamedStyle( const QString &namedStyle );

    /**
     * Returns the english named style of a font, if possible.
     * \param namedStyle a localized named style, i.e. "Fett", "Kursiv", etc
     * \returns The english named style
     * \see translateNamedStyle
     * \since QGIS 2.12
     */
    static QString untranslateNamedStyle( const QString &namedStyle );

    /**
     * Returns a CSS string representing the specified font as closely as possible.
     * \param font QFont to convert
     * \param pointToPixelMultiplier scaling factor to apply to convert point sizes to pixel font sizes.
     * The CSS returned by this function will always use pixels for font sizes, so this parameter
     * should be set to a suitable value to convert point sizes to pixels (e.g., taking into account
     * destination DPI)
     * \returns partial CSS string, e.g., "font-family: Comic Sans; font-size: 12px;"
     * \since QGIS 2.16
     */
    static QString asCSS( const QFont &font, double pointToPixelMultiplier = 1.0 );

    /**
     * Adds a font \a family to the list of recently used font families.
     * \see recentFontFamilies()
     * \since QGIS 3.0
     */
    static void addRecentFontFamily( const QString &family );

    /**
     * Returns a list of recently used font families.
     * \see addRecentFontFamily()
     * \since QGIS 3.0
     */
    static QStringList recentFontFamilies();
};

// clazy:excludeall=qstring-allocations

#endif // QGSFONTUTILS_H
