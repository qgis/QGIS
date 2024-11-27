/***************************************************************************
  qgstextblock.h
  ---------------
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

#ifndef QGSTEXTBLOCK_H
#define QGSTEXTBLOCK_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextfragment.h"
#include "qgstextblockformat.h"
#include "qgsstringutils.h"
#include <QVector>

/**
 * \class QgsTextBlock
 * \ingroup core
 *
 * \brief Represents a block of text consisting of one or more QgsTextFragment objects.
 *
 * \warning This API is not considered stable and may change in future QGIS versions.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextBlock
{

  public:

    /**
     * Constructor for an empty text block.
     */
    QgsTextBlock() = default;

    /**
     * Constructor for a QgsTextBlock consisting of a single text \a fragment.
     */
    explicit QgsTextBlock( const QgsTextFragment &fragment );

    /**
     * Constructor for QgsTextBlock consisting of a plain \a text, and optional character \a format.
     *
     * If \a text contains tab characters they will be appended as separate text fragments
     * within the block, consisting of just the tab character.
     *
     * \since QGIS 3.38
     */
    static QgsTextBlock fromPlainText( const QString &text, const QgsTextCharacterFormat &format = QgsTextCharacterFormat() );

    /**
     * Converts the block to plain text.
     *
     * \since QGIS 3.16
     */
    QString toPlainText() const;

    /**
     * Reserves the specified \a count of fragments for optimised fragment appending.
     *
     * \since QGIS 3.40
     */
    void reserve( int count );

    /**
     * Appends a \a fragment to the block.
     */
    void append( const QgsTextFragment &fragment );

    /**
     * Appends a \a fragment to the block.
     */
    void append( QgsTextFragment &&fragment ) SIP_SKIP;
#ifndef SIP_RUN

    /**
     * Inserts a \a fragment into the block, at the specified index.
     *
     * \since QGIS 3.40
     */
    void insert( int index, const QgsTextFragment &fragment );

#else
    /**
     * Inserts a \a fragment into the block, at the specified index.
     *
     * \throws IndexError if no fragment exists at the specified index.
     *
     * \since QGIS 3.40
     */
    void insert( int index, const QgsTextFragment &fragment );
    % MethodCode
    if ( a0 < 0 || a0 > sipCpp->size() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->insert( a0, *a1 );
    }
    % End
#endif

    /**
     * Inserts a \a fragment into the block, at the specified index.
     *
     * \since QGIS 3.40
     */
    void insert( int index, QgsTextFragment &&fragment ) SIP_SKIP;

    /**
     * Clears the block, removing all its contents.
     */
    void clear();

    /**
     * Returns TRUE if the block is empty.
     */
    bool empty() const;

    /**
     * Returns the number of fragments in the block.
     */
    int size() const;

    /**
     * Returns the block formatting for the fragment.
     *
     * \see setBlockFormat()
     *
     * \since QGIS 3.40
     */
    const QgsTextBlockFormat &blockFormat() const { return mBlockFormat; }

    /**
     * Sets the block \a format for the fragment.
     *
     * \see blockFormat()
     *
     * \since QGIS 3.40
     */
    void setBlockFormat( const QgsTextBlockFormat &format );

    /**
     * Applies a \a capitalization style to the block's text.
     *
     * \since QGIS 3.16
     */
    void applyCapitalization( Qgis::Capitalization capitalization );

    /**
     * Returns TRUE if the block or any of the fragments in the block have background brushes set.
     *
     * \since QGIS 3.42
     */
    bool hasBackgrounds() const;

#ifdef SIP_RUN
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->size();
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the fragment at the specified \a index.
     */
    const QgsTextFragment &at( int index ) const SIP_FACTORY;
#else

    /**
     * Returns the fragment at the specified \a index.
     *
     * \throws KeyError if no fragment exists at the specified index.
     */
    const QgsTextFragment &at( int index ) const SIP_FACTORY;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->size() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsTextFragment( sipCpp->at( a0 ) );
    }
    % End
#endif

    /**
     * Returns the fragment at the specified index.
     */
    QgsTextFragment &operator[]( int index ) SIP_FACTORY;
#ifdef SIP_RUN
    % MethodCode
    SIP_SSIZE_T idx = sipConvertFromSequenceIndex( a0, sipCpp->size() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      sipRes = new QgsTextFragment( sipCpp->operator[]( idx ) );
    % End
#endif

#ifndef SIP_RUN
    ///@cond PRIVATE
    QVector< QgsTextFragment >::const_iterator begin() const;
    QVector< QgsTextFragment >::const_iterator end() const;
    ///@endcond
#endif

  private:

    QVector< QgsTextFragment > mFragments;
    QgsTextBlockFormat mBlockFormat;
};

#endif // QGSTEXTBLOCK_H
