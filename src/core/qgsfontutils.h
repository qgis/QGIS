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

/** \ingroup core
 * \class QgsFontUtils
 */
class CORE_EXPORT QgsFontUtils
{
  public:
    /** Check whether exact font is on system
     * @param f The font to test for match
     */
    static bool fontMatchOnSystem( const QFont& f );

    /** Check whether font family is on system in a quick manner, which does not compare [foundry]
     * @param family The family to test
     * @returns Whether family was found on system
     * @note This is good for use in loops of large lists, e.g. registering many features for labeling
     */
    static bool fontFamilyOnSystem( const QString& family );

    /** Check whether font family on system has specific style
     * @param family The family to test
     * @param style The style to test for
     * @returns Whether family has style
     * @note Added in QGIS 2.1
     */
    static bool fontFamilyHasStyle( const QString& family, const QString& style );

    /** Check whether font family is on system
     * @param family The family to test
     * @param chosen The actual family (possibly from different foundry) returned by system
     * @param match Whether the family [foundry] returned by system is a match
     * @returns Whether family was found on system
     */
    static bool fontFamilyMatchOnSystem( const QString& family, QString* chosen = nullptr, bool* match = nullptr );

    /** Updates font with named style and retain all font properties
     * @param f The font to update
     * @param fontstyle The style to try and switch the font to
     * @param fallback If no matching fontstyle found for font, assign most similar or first style found to font
     * @returns Whether the font was updated (also returns true if the requested style matches font's current style)
     * @note This is a more featured replacement for a Qt 4.8+ function: void QFont::setStyleName ( const QString & styleName )
     */
    static bool updateFontViaStyle( QFont& f, const QString& fontstyle, bool fallback = false );

    /** Get standard test font family
     * @note Added in QGIS 2.1
     */
    static QString standardTestFontFamily();

    /** Loads standard test fonts from filesystem or qrc resource
     * @param loadstyles List of styles to load, e.g. All, Roman, Oblique, Bold, Bold Oblique
     * @returns Whether any font was loaded
     * @note Done by default on debug app/server startup to ensure fonts available for unit tests (Roman and Bold)
     * @note Added in QGIS 2.1
     */
    static bool loadStandardTestFonts( const QStringList& loadstyles );

    /** Get standard test font with specific style
     * @param style Style to load, e.g. Roman, Oblique, Bold, Bold Oblique
     * @param pointsize Font point size to set
     * @returns QFont
     * @note Added in QGIS 2.1
     */
    static QFont getStandardTestFont( const QString& style = "Roman", int pointsize = 12 );

    /** Returns a DOM element containing the properties of the font.
     * @param font font
     * @param document DOM document
     * @param elementName name for DOM element
     * @returns DOM element containing font settings
     * @note added in QGIS 2.10
     * @see setFromXmlElement
     */
    static QDomElement toXmlElement( const QFont& font, QDomDocument &document, const QString &elementName );

    /** Sets the properties of a font to match the properties stored in an XML element. Calling
     * this will overwrite the current properties of the font.
     * @param font font to update
     * @param element DOM element
     * @returns true if properties were successfully read from element
     * @note added in QGIS 2.10
     * @see toXmlElement
     * @see setFromXmlChildNode
     */
    static bool setFromXmlElement( QFont& font, const QDomElement& element );

    /** Sets the properties of a font to match the properties stored in an XML child node. Calling
     * this will overwrite the current properties of the font.
     * @param font font to update
     * @param element DOM element
     * @param childNode name of child node
     * @returns true if child node exists and properties were successfully read from node
     * @note added in QGIS 2.10
     * @see setFromXmlElement
     * @see toXmlElement
     */
    static bool setFromXmlChildNode( QFont& font, const QDomElement& element, const QString& childNode );

    /** Returns the localized named style of a font, if such a translation is available.
     * @param namedStyle a named style, i.e. "Bold", "Italic", etc
     * @returns The localized named style
     * @note added in QGIS 2.12
     * @see untranslateNamedStyle
     */
    static QString translateNamedStyle( const QString& namedStyle );

    /** Returns the english named style of a font, if possible.
     * @param namedStyle a localized named style, i.e. "Fett", "Kursiv", etc
     * @returns The english named style
     * @note added in QGIS 2.12
     * @see translateNamedStyle
     */
    static QString untranslateNamedStyle( const QString& namedStyle );

    /** Returns a CSS string representing the specified font as closely as possible.
     * @param font QFont to convert
     * @param pointToPixelMultiplier scaling factor to apply to convert point sizes to pixel font sizes.
     * The CSS returned by this function will always use pixels for font sizes, so this parameter
     * should be set to a suitable value to convert point sizes to pixels (eg taking into account
     * desination DPI)
     * @returns partial CSS string, eg "font-family: Comic Sans; font-size: 12px;"
     * @note added in QGIS 2.16
     */
    static QString asCSS( const QFont& font, double pointToPixelMultiplier = 1.0 );
};

#endif // QGSFONTUTILS_H
