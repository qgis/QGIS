/***************************************************************************
      qgsproviderextentcalcevent.h  -  Notification that the exact extent
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

#ifndef QGSPROVIDEREXTENTCALCEVENT_H
#define QGSPROVIDEREXTENTCALCEVENT_H

#include <QEvent>
class QgsRectangle;

/** \ingroup core
 * A custom event that is designed to be fired when a layer extent has been fully calculated.
   \author  Brendan Morley
   \date    March 2005


   This custom QEvent is designed to be fired when the full extent of a layer has been calculated.
   It was initially included in QGIS to help the QgsPostgresProvider provide the asynchronous
   calculation of PostgreSQL layer extents.

   Events are used instead of Qt signals/slots as events can be received asynchronously,
   which makes for better mutlithreading behaviour and less opportunity for programmer mishap.

 */

// TODO: Add the pg table this is a extent OF.

class CORE_EXPORT QgsProviderExtentCalcEvent : public QEvent
{

  public:

    QgsProviderExtentCalcEvent( QgsRectangle* layerExtent );

    QgsRectangle* layerExtent() const;


  private:

    QgsRectangle* le;

};

#endif
