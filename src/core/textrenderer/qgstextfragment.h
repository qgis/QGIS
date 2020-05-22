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

class QTextFragment;

/**
 * \class QgsTextFragment
  * \ingroup core
  * Stores a fragment of text along with formatting overrides to be used when rendering the fragment.
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
     * Returns the horizontal advance associated with this fragment, when rendered using
     * the specified base \a font.
     *
     * Set \a fontHasBeenUpdatedForFragment to TRUE if \a font already represents the character
     * format for this fragment.
     */
    double horizontalAdvance( const QFont &font, bool fontHasBeenUpdatedForFragment = false ) const;

  private:

    QString mText;
    QgsTextCharacterFormat mCharFormat;
};

#endif // QGSTEXTFRAGMENT_H
