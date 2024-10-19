/***************************************************************************
  qgstextfragment.h
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTFRAGMENT_H
#define QGSTEXTFRAGMENT_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextcharacterformat.h"
#include "qgis.h"

class QTextFragment;

/**
 * \class QgsTextFragment
  * \ingroup core
  * \brief Stores a fragment of document along with formatting overrides to be used when rendering the fragment.
  *
  * Text fragments consist of either a block of text or another atomic component of a document (such as an image).
  *
  * Each fragment has an associated characterFormat(), which specifies the text formatting overrides
  * to use when rendering the fragment. Additionally, the characterFormat() may contain properties
  * for other fragment types, such as image paths and sizes for image fragments.
  *
  * \warning This API is not considered stable and may change in future QGIS versions.
  *
  * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextFragment
{
  public:

    /**
     * Constructor for QgsTextFragment, with the specified \a text and optional character \a format.
     */
    explicit QgsTextFragment( const QString &text = QString(), const QgsTextCharacterFormat &format = QgsTextCharacterFormat() );

    /**
     * Constructor for QgsTextFragment, based on the specified QTextFragment \a fragment.
     */
    explicit QgsTextFragment( const QTextFragment &fragment );

    /**
     * Returns the text content of the fragment.
     *
     * \see setText()
     */
    QString text() const;

    /**
     * Sets the \a text content of the fragment.
     *
     * \see text()
     */
    void setText( const QString &text );

    /**
     * Returns TRUE if the fragment consists of just a tab character.
     *
     * \since QGIS 3.38
     */
    bool isTab() const { return mText == '\t'; }

    /**
     * Returns TRUE if the fragment consists of just whitespace characters, and does not
     * contain any content to render.
     *
     * \since QGIS 3.38
     */
    bool isWhitespace() const { return isTab() || mText.trimmed().isEmpty(); }

    /**
     * Returns the character formatting for the fragment.
     *
     * \see setCharacterFormat()
     */
    const QgsTextCharacterFormat &characterFormat() const { return mCharFormat; }

    /**
     * Sets the character \a format for the fragment.
     *
     * \see characterFormat()
     */
    void setCharacterFormat( const QgsTextCharacterFormat &format );

    /**
     * Returns TRUE if the fragment represents an image.
     *
     * \since QGIS 3.40
     */
    bool isImage() const;

    /**
     * Returns the horizontal advance associated with this fragment, when rendered using
     * the specified base \a font within the specified render \a context.
     *
     * Set \a fontHasBeenUpdatedForFragment to TRUE if \a font already represents the character
     * format for this fragment.
     *
     * The optional \a scaleFactor parameter can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::calculateScaleFactorForFormat() and then manually calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     */
    double horizontalAdvance( const QFont &font, const QgsRenderContext &context, bool fontHasBeenUpdatedForFragment = false, double scaleFactor = 1.0 ) const;

    /**
     * Applies a \a capitalization style to the fragment's text.
     *
     * \since QGIS 3.16
     */
    void applyCapitalization( Qgis::Capitalization capitalization );

  private:

    QString mText;
    bool mIsImage = false;
    QgsTextCharacterFormat mCharFormat;
};

#endif // QGSTEXTFRAGMENT_H
