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
#include <QSizeF>
#include <QBrush>

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
     * Override all the default/unset properties of the current character format
     * with the settings from another format.
     *
     * This will replace any default/unset existing settings with the
     * settings from \a other.
     *
     * Any settings which are default/unset in \a other will be left unchanged.
     *
     * \param other The format to override with.
     *
     * \since QGIS 3.36
     */
    void overrideWith( const QgsTextCharacterFormat &other );

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
     * \note A format should have either fontPointSize() or fontPercentageSize() set, not both.
     *
     * \see fontPercentageSize()
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
     * \note A format should have either fontPointSize() or fontPercentageSize() set, not both.
     *
     * \see fontPointSize()
     * \see setFontPercentageSize()
     * \since QGIS 3.28
     */
    void setFontPointSize( double size );

    /**
     * Returns the font percentage size (as fraction of inherited font size), or -1 if the font size percentage is not set.
     *
     * \note A format should have either fontPointSize() or fontPercentageSize() set, not both.
     *
     * \see fontPointSize()
     * \see setFontPercentageSize()
     * \since QGIS 3.40
     */
    double fontPercentageSize() const;

    /**
     * Sets  the font percentage \a size (as fraction of inherited font size).
     *
     * Set \a size to -1 if the font percentange size is not set.
     *
     * \note A format should have either fontPointSize() or fontPercentageSize() set, not both.
     *
     * \see fontPercentageSize()
     * \see setFontPointSize()
     * \since QGIS 3.40
     */
    void setFontPercentageSize( double size );

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
     * Returns the font word spacing, in points, or NaN if word spacing is not set and should be inherited.
     *
     * \see setWordSpacing()
     * \since QGIS 3.40
     */
    double wordSpacing() const;

    /**
     * Sets the font word \a spacing, in points, or NaN if word spacing is not set and should be inherited.
     *
     * \see wordSpacing()
     * \since QGIS 3.40
     */
    void setWordSpacing( double spacing );

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
     * Returns the path to the image to render, if the format applies to a document image fragment.
     *
     * \see QgsTextFragment::isImage()
     * \see imageSize()
     * \see setImagePath()
     *
     * \since QGIS 3.40
     */
    QString imagePath() const;

    /**
     * Sets the \a path to the image to render, if the format applies to a document image fragment.
     *
     * \see QgsTextFragment::isImage()
     * \see setImageSize()
     * \see imagePath()
     *
     * \since QGIS 3.40
     */
    void setImagePath( const QString &path );

    /**
     * Returns the image size, if the format applies to a document image fragment.
     *
     * The image size is always considered to be in Qgis::RenderUnit::Points.
     *
     * \see QgsTextFragment::isImage()
     * \see imagePath()
     * \see setImageSize()
     *
     * \since QGIS 3.40
     */
    QSizeF imageSize() const;

    /**
     * Sets the image \a size, if the format applies to a document image fragment.
     *
     * The image size is always considered to be in Qgis::RenderUnit::Points.
     *
     * \see QgsTextFragment::isImage()
     * \see setImagePath()
     * \see imageSize()
     *
     * \since QGIS 3.40
     */
    void setImageSize( const QSizeF &size );

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
     * Returns TRUE if the fragment has a background set.
     *
     * \see backgroundBrush()
     * \since QGIS 3.42
     */
    bool hasBackground() const;

    /**
     * Returns the brush used for rendering the background of the fragment.
     *
     * Alternatively, the format may have a backgroundImagePath() set.
     *
     * \see hasBackground()
     * \see setBackgroundBrush()
     * \since QGIS 3.42
     */
    QBrush backgroundBrush() const;

    /**
     * Sets the \a brush used for rendering the background of the fragment.
     *
     * Alternatively, the format may have a backgroundImagePath() set.
     *
     * \see backgroundBrush()
     * \since QGIS 3.42
     */
    void setBackgroundBrush( const QBrush &brush );

    /**
     * Returns the path for the image to be used for rendering the background of the fragment.
     *
     * Alternatively, the format may have a backgroundBrush() set.
     *
     * \see hasBackground()
     * \see setBackgroundImagePath()
     * \since QGIS 3.42
     */
    QString backgroundImagePath() const;

    /**
     * Sets the \a path for the image to be used for rendering the background of the fragment.
     *
     * Alternatively, the format may have a backgroundBrush() set.
     *
     * \see backgroundImagePath()
     * \since QGIS 3.42
     */
    void setBackgroundImagePath( const QString &path );

    /**
     * Updates the specified \a font in place, applying character formatting options which
     * are applicable on a font level when rendered in the given \a context.
     *
     * The optional \a scaleFactor parameter can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::calculateScaleFactorForFormat() and then manually calculations
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
    double mFontPercentageSize = -1;
    QString mFontFamily;
    double mWordSpacing = std::numeric_limits< double >::quiet_NaN();

    bool mHasVerticalAlignSet = false;
    Qgis::TextCharacterVerticalAlignment mVerticalAlign = Qgis::TextCharacterVerticalAlignment::Normal;

    QString mImagePath;
    QSizeF mImageSize;

    BooleanValue mStrikethrough = BooleanValue::NotSet;
    BooleanValue mUnderline = BooleanValue::NotSet;
    BooleanValue mOverline = BooleanValue::NotSet;

    QBrush mBackgroundBrush;
    QString mBackgroundPath;

};

#endif // QGSTEXTCHARACTERFORMAT_H
