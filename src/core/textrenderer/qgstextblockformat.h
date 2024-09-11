/***************************************************************************
  qgstextblockformat.h
  -----------------
   begin                : September 2024
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

#ifndef QGSTEXTBLOCKFORMAT_H
#define QGSTEXTBLOCKFORMAT_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

#include <QFont>
#include <QColor>

class QTextBlockFormat;
class QgsRenderContext;

/**
 * \class QgsTextBlockFormat
  * \ingroup core
  * \brief Stores information relating to individual block formatting.
  *
  * These options encapsulate formatting options which override the default
  * settings from a QgsTextFormat for individual text blocks.
  *
  * \warning This API is not considered stable and may change in future QGIS versions.
  *
  * \since QGIS 3.40
 */
class CORE_EXPORT QgsTextBlockFormat
{
  public:

    QgsTextBlockFormat() = default;

    /**
     * Constructor for QgsTextBlockFormat, based on the specified QTextBlockFormat \a format.
     */
    QgsTextBlockFormat( const QTextBlockFormat &format );

    //! Status values for boolean format properties
    enum class BooleanValue
    {
      NotSet, //!< Property is not set
      SetTrue, //!< Property is set and TRUE
      SetFalse, //!< Property is set and FALSE
    };

    /**
     * Override all the default/unset properties of the current block format
     * with the settings from another format.
     *
     * This will replace any default/unset existing settings with the
     * settings from \a other.
     *
     * Any settings which are default/unset in \a other will be left unchanged.
     *
     * \param other The format to override with.
     */
    void overrideWith( const QgsTextBlockFormat &other );

    /**
     * Returns TRUE if the format has an explicit horizontal alignment set.
     *
     * If FALSE is returned then the horizontal alignment will be inherited.
     *
     * \see setHasHorizontalAlignmentSet()
     * \see horizontalAlignment()
     */
    bool hasHorizontalAlignmentSet() const { return mHasHorizontalAlignSet; }

    /**
     * Sets whether the format has an explicit horizontal alignment \a set.
     *
     * If \a set is FALSE then the horizontal alignment will be inherited.
     *
     * \see hasHorizontalAlignmentSet()
     * \see setHorizontalAlignment()
     */
    void setHasHorizontalAlignmentSet( bool set ) { mHasHorizontalAlignSet = set; }

    /**
     * Returns the format horizontal alignment.
     *
     * This property is only respected if hasHorizontalAlignmentSet() is TRUE.
     *
     * \see hasHorizontalAlignmentSet()
     * \see setHorizontalAlignment()
     */
    Qgis::TextHorizontalAlignment horizontalAlignment() const { return mHorizontalAlign; }

    /**
     * Sets the format horizontal \a alignment.
     *
     * This property is only respected if hasHorizontalAlignmentSet() is TRUE.
     *
     * \see hasHorizontalAlignmentSet()
     * \see horizontalAlignment()
     */
    void setHorizontalAlignment( Qgis::TextHorizontalAlignment alignment ) { mHorizontalAlign = alignment; }

    /**
     * Updates the specified \a font in place, applying block formatting options which
     * are applicable on a font level when rendered in the given \a context.
     *
     * The optional \a scaleFactor parameter can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::calculateScaleFactorForFormat() and then manually calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     */
    void updateFontForFormat( QFont &font, const QgsRenderContext &context, double scaleFactor = 1.0 ) const;

  private:

    bool mHasHorizontalAlignSet = false;
    Qgis::TextHorizontalAlignment mHorizontalAlign = Qgis::TextHorizontalAlignment::Left;

};

#endif // QGSTEXTBLOCKFORMAT_H
