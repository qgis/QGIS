/***************************************************************************
      qgspostgresextentthread.h  -  Multithreaded PostgreSQL layer extents
                                    retrieval
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

#ifndef QGSPOSTGRESEXTENTTHREAD_H
#define QGSPOSTGRESEXTENTTHREAD_H

extern "C"
{
#include <libpq-fe.h>
}

#include <QThread>

/*!
 * \brief   A thread to retrieve the exact extent in a PostgreSQL/PostGIS table in its native coordinates
   \author  Brendan Morley
   \date    March 2005


   This QThread is designed to take the parsed PostgreSQL connection parameters from
   a parent QgsPostgresProvider, and select the full PostGIS extent() of the specified table.

   Upon completion, this thread expects to be able to post an event back to the parent
   QgsPostgresProvider.  This parent should take this event as a notification that the
   exact exten is now available for the parent to copy into whereever it keeps its
   layer extent.

   The parent is also responsible for notifying any GUI items that use the extent.

   Events are used instead of Qt signals/slots as events can be received asynchronously,
   which makes for better mutlithreading behaviour and less opportunity for programmer mishap.

   The intended return path is:
   1. Post a QGis::ProviderExtentCalcEvent to the calling QgsPostgresProvider with new extent.
   2. QgsPostgresProvider emits a fullExtentCalculated() signal.
   3. QgsVectorLayer receives this signal.
   4. The updateExtents slot in QgsVectorLayer is called.
   5. The QgsVectorLayer extents are refreshed from the QgsPostgresProvider.
   6. QgsVectorLayer emits a recalculateExtents() signal.
   7. QgsMapCanvas receives this signal.
   8. The recalculateExtents() slot in QgsMapCanvas is called.

   (The overview canvas is only refreshed with this new extent when the user
   resizes the extent of the main canvas)

   Optionally also from 2:
   2. QgsPostgresProvider emits a repaintRequested() signal.
   3. QgsVectorLayer receives this signal.
   4. The triggerRepaint slot in QgsVectorLayer is called.
   6. QgsVectorLayer emits a repaintRequested() signal.
   7. QgsMapCanvas receives this signal.
   8. The refresh() slot in QgsMapCanvas is called.

   TODO: Subclass overview canvas from mapcanvas so that only it needs to be refreshed

 */

class QgsPostgresExtentThread : public QThread
{

  public:

    // TODO: Combine everything into a constructor.

    virtual void run();

    void setConnInfo( QString s );

    void setTableName( QString s );

    void setSqlWhereClause( QString s );

    void setGeometryColumn( QString s );

    //!  Inform this thread of where to lodge an event upon successful completion.
    /*!

         \param o   The QObject that this thread should post an event to when the full extents
                    have been calculated

     */
    void setCallback( QObject* o )
    {
      callbackObject = o;
    }


  private:

    /**
     * Connection pointer
     */
    PGconn *connection;

    /**
     *
     */
    QString connectionInfo;

    /**
     * Name of the table with no schema
     */
    QString tableName;

    /**
     * SQL statement used to limit the features retrieved
     */
    QString sqlWhereClause;

    /**
     * Name of the geometry column in the table
     */
    QString geometryColumn;

    /**
     * Pointer to the object to call back when the extents have been calculated.
     * It is a void pointer to avoid a circular reference back to QgsPostgresProvider
     */


    QObject* callbackObject;
    //QgsPostgresProvider* callbackObject;

//  typedef void (QgsPostgresProvider::*tFunction)(QgsRect* rect);

//  QgsPostgresProvider  *cInst;
//  tFunction             pFunction;
//  QgsRectCallbackTemplate<QgsPostgresExtentThread> callbackFunction;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    QgsRect* layerExtent;

};

#endif
