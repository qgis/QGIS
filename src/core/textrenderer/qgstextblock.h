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

#include <QVector>

#ifndef SIP_RUN

/**
 * \class QgsTextBlock
 *
 * Represents a block of text consisting of one or more QgsTextFragment objects.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTextBlock : public QVector< QgsTextFragment >
{

  public:

    QgsTextBlock() = default;

    /**
     * Constructor for a QgsTextBlock consisting of a single text \a fragment.
     */
    explicit QgsTextBlock( const QgsTextFragment &fragment );

};
#endif

#endif // QGSTEXTBLOCK_H
