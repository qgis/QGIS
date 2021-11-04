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
#include "qgsstringutils.h"
#include "qgstextblock.h"

#include <QVector>

class QgsTextFragment;

/**
 * \class QgsTextDocument
 * \ingroup core
 *
 * \brief Represents a document consisting of one or more QgsTextBlock objects.
 *
 * \warning This API is not considered stable and may change in future QGIS versions.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextDocument
{

  public:

    QgsTextDocument();
    ~QgsTextDocument();

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
     * Appends a \a block to the document.
     */
    void append( const QgsTextBlock &block );

    /**
     * Appends a \a block to the document.
     */
    void append( QgsTextBlock &&block ) SIP_SKIP;

    /**
     * Reserves the specified \a count of blocks for optimised block appending.
     */
    void reserve( int count );

#ifndef SIP_RUN

    /**
     * Returns the block at the specified \a index.
     */
    const QgsTextBlock &at( int index ) const SIP_FACTORY;
#else

    /**
     * Returns the block at the specified \a index.
     *
     * \throws KeyError if no block exists at the specified index.
     */
    const QgsTextBlock &at( int index ) const SIP_FACTORY;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->size() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsTextBlock( sipCpp->at( a0 ) );
    }
    % End
#endif

    /**
     * Returns the block at the specified index.
     */
    QgsTextBlock &operator[]( int index ) SIP_FACTORY;
#ifdef SIP_RUN
    % MethodCode
    SIP_SSIZE_T idx = sipConvertFromSequenceIndex( a0, sipCpp->size() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      sipRes = new QgsTextBlock( sipCpp->operator[]( idx ) );
    % End
#endif

    /**
     * Returns the number of blocks in the document.
     */
    int size() const;

#ifdef SIP_RUN
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->size();
    % End
#endif

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

    /**
     * Applies a \a capitalization style to the document's text.
     *
     * \since QGIS 3.16
     */
    void applyCapitalization( Qgis::Capitalization capitalization );

#ifndef SIP_RUN
    ///@cond PRIVATE
    QVector< QgsTextBlock >::const_iterator begin() const;
    QVector< QgsTextBlock >::const_iterator end() const;
    ///@endcond
#endif

  private:

    QVector< QgsTextBlock > mBlocks;

};

#endif // QGSTEXTDOCUMENT_H
