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
     * Converts the block to plain text.
     *
     * \since QGIS 3.16
     */
    QString toPlainText() const;

    /**
     * Appends a \a fragment to the block.
     */
    void append( const QgsTextFragment &fragment );

    /**
     * Appends a \a fragment to the block.
     */
    void append( QgsTextFragment &&fragment ) SIP_SKIP;

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
     * Applies a \a capitalization style to the block's text.
     *
     * \since QGIS 3.16
     */
    void applyCapitalization( Qgis::Capitalization capitalization );

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

};

#endif // QGSTEXTBLOCK_H
