/***************************************************************************
      qgsprovidercountcalcevent.h  -  Notification that the exact count
                                      of a layer has been calculated.
                             -------------------
    begin                : Feb 1, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSPROVIDERCOUNTCALCEVENT_H
#define QGSPROVIDERCOUNTCALCEVENT_H

#include <QEvent>

/** \ingroup core
 * \brief   A custom event that is designed to be fired when a layer count has been fully calculated.
   \author  Brendan Morley
   \date    March 2005


   This custom QEvent is designed to be fired when the full item count of a layer has been calculated.
   It was initially included in QGIS to help the QgsPostgresProvider provide the asynchronous
   calculation of PostgreSQL layer counts.

   Events are used instead of Qt signals/slots as events can be received asynchronously,
   which makes for better mutlithreading behaviour and less opportunity for programmer mishap.

 */

// TODO: Add the pg table this is a count OF.

class CORE_EXPORT QgsProviderCountCalcEvent : public QEvent
{

  public:

    QgsProviderCountCalcEvent( long featuresCounted );

    long featuresCounted() const;


  private:

    long n;

};

#endif
