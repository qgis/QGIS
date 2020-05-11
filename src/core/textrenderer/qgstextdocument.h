/***************************************************************************
  qgstextdocument.h
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

#ifndef QGSTEXTDOCUMENT_H
#define QGSTEXTDOCUMENT_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextblock.h"

#include <QVector>


#ifndef SIP_RUN

/**
 * \class QgsTextDocument
 *
 * Represents a document consisting of one or more QgsTextBlock objects.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextDocument : public QVector< QgsTextBlock >
{

  public:

    QgsTextDocument() = default;

    /**
     * Constructor for a QgsTextDocument consisting of a single text \a block.
     */
    explicit QgsTextDocument( const QgsTextBlock &block );

    /**
     * Constructor for a QgsTextDocument consisting of a single text \a fragment.
     */
    explicit QgsTextDocument( const QgsTextFragment &fragment );

    /**
     * Constructor for QgsTextDocument consisting of a set of plain text \a lines.
     */
    static QgsTextDocument fromPlainText( const QStringList &lines );

    /**
     * Constructor for QgsTextDocument consisting of a set of HTML formatted \a lines.
     */
    static QgsTextDocument fromHtml( const QStringList &lines );

    /**
     * Returns a list of plain text lines of text representing the document.
     */
    QStringList toPlainText() const;

    /**
     * Splits lines of text in the document to separate lines, using a specified wrap character (\a wrapCharacter) or newline characters.
     *
     * The \a autoWrapLength argument can be used to specify an ideal length of line to automatically
     * wrap text to (automatic wrapping is disabled if \a autoWrapLength is 0). This automatic wrapping is performed
     * after processing wrapping using \a wrapCharacter. When auto wrapping is enabled, the \a useMaxLineLengthWhenAutoWrapping
     * argument controls whether the lines should be wrapped to an ideal maximum of \a autoWrapLength characters, or
     * if FALSE then the lines are wrapped to an ideal minimum length of \a autoWrapLength characters.
     */
    void splitLines( const QString &wrapCharacter, int autoWrapLength = 0, bool useMaxLineLengthWhenAutoWrapping = true );

};

#endif


#endif // QGSTEXTDOCUMENT_H
