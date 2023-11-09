/***************************************************************************
  qgstextcharacterformat.h
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

#ifndef QGSTEXTCHARACTERFORMAT_H
#define QGSTEXTCHARACTERFORMAT_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

#include <QFont>
#include <QColor>

class QTextCharFormat;
class QgsRenderContext;

/**
 * \class QgsTextCharacterFormat
  * \ingroup core
  * \brief Stores information relating to individual character formatting.
  *
  * These options encapsulate formatting options which override the default
  * settings from a QgsTextFormat for individual characters (or sets of characters).
  *
  * \warning This API is not considered stable and may change in future QGIS versions.
  *
  * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextCharacterFormat
{
  public:

    /**
     * Constructor for QgsTextCharacterFormat.
     */
    QgsTextCharacterFormat() = default;

    /**
     * Constructor for QgsTextCharacterFormat, based on the specified QTextCharFormat \a format.
     */
    QgsTextCharacterFormat( const QTextCharFormat &format );

    //! Status values for boolean format properties
    enum class BooleanValue
    {
      NotSet, //!< Property is not set
      SetTrue, //!< Property is set and TRUE
      SetFalse, //!< Property is set and FALSE
    };

    /**
     * Returns the character's text color, or an invalid color if no color override
     * is set and the default format color should be used.
     *
     * \see setTextColor()
     */
    QColor textColor() const;

    /**
     * Sets the character's text \a color.
     *
     * Set \a color to an invalid color if no color override
     * is desired and the default format color should be used.
     *
     * \see textColor()
     */
    void setTextColor( const QColor &textColor );

    /**
     * Returns the font point size, or -1 if the font size is not set
     * and should be inherited.
     *
     * \see setFontPointSize()
     * \since QGIS 3.28
     */
    double fontPointSize() const;

    /**
     * Sets the font point \a size.
     *
     * Set \a size to -1 if the font size is not set
     * and should be inherited.
     *
     * \see fontPointSize()
     * \since QGIS 3.28
     */
    void setFontPointSize( double size );

    /**
     * Returns the font family name, or an empty string if the
     * family is not set and should be inherited.
     *
     * \see setFamily()
     * \since QGIS 3.28
     */
    QString family() const;

    /**
     * Sets the font \a family name.
     *
     * Set to an empty string if the family should be inherited.
     *
     * \see family()
     * \since QGIS 3.28
     */
    void setFamily( const QString &family );

    /**
     * Returns the font weight, or -1 if the font weight is not set
     * and should be inherited.
     *
     * \see setFontWeight()
     * \since QGIS 3.28
     */
    int fontWeight() const;

    /**
     * Sets the font \a weight.
     *
     * Set \a weight to -1 if the font weight is not set
     * and should be inherited.
     *
     * \see fontWeight()
     * \since QGIS 3.28
     */
    void setFontWeight( int fontWeight );

    /**
     * Returns whether the format has italic enabled.
     *
     * \see setItalic()
     * \since QGIS 3.28
     */
    BooleanValue italic() const;

    /**
     * Sets whether the format has italic \a enabled.
     *
     * \see italic()
     * \since QGIS 3.28
     */
    void setItalic( BooleanValue enabled );

    /**
     * Returns whether the format has strikethrough enabled.
     *
     * \see setStrikeOut()
     */
    BooleanValue strikeOut() const;

    /**
     * Sets whether the format has strikethrough \a enabled.
     *
     * \see strikeOut()
     */
    void setStrikeOut( BooleanValue enabled );

    /**
     * Returns whether the format has underline enabled.
     *
     * \see setUnderline()
     */
    BooleanValue underline() const;

    /**
     * Sets whether the format has underline \a enabled.
     *
     * \see underline()
     */
    void setUnderline( BooleanValue enabled );

    /**
     * Returns whether the format has overline enabled.
     *
     * \see setUnderline()
     */
    BooleanValue overline() const;

    /**
     * Sets whether the format has overline \a enabled.
     *
     * \see overline()
     */
    void setOverline( BooleanValue enabled );

    /**
     * Returns TRUE if the format has an explicit vertical alignment set.
     *
     * If FALSE is returned then the vertical alignment will be inherited.
     *
     * \see setHasVerticalAlignmentSet()
     * \see verticalAlignment()
     *
     * \since QGIS 3.30
     */
    bool hasVerticalAlignmentSet() const { return mHasVerticalAlignSet; }

    /**
     * Sets whether the format has an explicit vertical alignment \a set.
     *
     * If \a set is FALSE then the vertical alignment will be inherited.
     *
     * \see hasVerticalAlignmentSet()
     * \see setVerticalAlignment()
     *
     * \since QGIS 3.30
     */
    void setHasVerticalAlignmentSet( bool set ) { mHasVerticalAlignSet = set; }

    /**
     * Returns the format vertical alignment.
     *
     * This property is only respected if hasVerticalAlignmentSet() is TRUE.
     *
     * \see hasVerticalAlignmentSet()
     * \see setVerticalAlignment()
     *
     * \since QGIS 3.30
     */
    Qgis::TextCharacterVerticalAlignment verticalAlignment() const { return mVerticalAlign; }

    /**
     * Sets the format vertical \a alignment.
     *
     * This property is only respected if hasVerticalAlignmentSet() is TRUE.
     *
     * \see hasVerticalAlignmentSet()
     * \see verticalAlignment()
     *
     * \since QGIS 3.30
     */
    void setVerticalAlignment( Qgis::TextCharacterVerticalAlignment alignment ) { mVerticalAlign = alignment; }

    /**
     * Updates the specified \a font in place, applying character formatting options which
     * are applicable on a font level when rendered in the given \a context.
     *
     * The optional \a scaleFactor parameter can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::FONT_WORKAROUND_SCALE and then manually calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     */
    void updateFontForFormat( QFont &font, const QgsRenderContext &context, double scaleFactor = 1.0 ) const;

  private:

    QColor mTextColor;
    int mFontWeight = -1;
    QString mStyleName;
    BooleanValue mItalic = BooleanValue::NotSet;
    double mFontPointSize = -1;
    QString mFontFamily;

    bool mHasVerticalAlignSet = false;
    Qgis::TextCharacterVerticalAlignment mVerticalAlign = Qgis::TextCharacterVerticalAlignment::Normal;

    BooleanValue mStrikethrough = BooleanValue::NotSet;
    BooleanValue mUnderline = BooleanValue::NotSet;
    BooleanValue mOverline = BooleanValue::NotSet;
};

#endif // QGSTEXTCHARACTERFORMAT_H
