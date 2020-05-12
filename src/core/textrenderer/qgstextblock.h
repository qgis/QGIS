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

#include <QVector>

class QgsTextFragment;

/**
 * \class QgsTextBlock
 *
 * Represents a block of text consisting of one or more QgsTextFragment objects.
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
