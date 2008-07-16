/***************************************************************************
      qgspostgrescountthread.h  -  Multithreaded PostgreSQL layer count
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

#ifndef QGSPOSTGRESCOUNTTHREAD_H
#define QGSPOSTGRESCOUNTTHREAD_H

extern "C"
{
#include <libpq-fe.h>
}

#include <QThread>

/*!
 * \brief   A thread to retrieve the exact number of items in a PostgreSQL/PostGIS table.
   \author  Brendan Morley
   \date    March 2005

  
   This QThread is designed to take the parsed PostgreSQL connection parameters from
   a parent QgsPostgresProvider, and select the full PostgreSQL count(*) of the specified table.
  
   Upon completion, this thread expects to be able to post an event back to the parent
   QgsPostgresProvider.  This parent should take this event as a notification that the 
   exact count is now available for the parent to copy into whereever it keeps its
   layer count.
  
   The parent is also responsible for notifying any GUI items that use the count.
  
   Events are used instead of Qt signals/slots as events can be received asynchronously,
   which makes for better mutlithreading behaviour and less opportunity for programmer mishap.

 */ 

class QgsPostgresCountThread : public QThread
{

public:

  /*
   * BM
   * The idea behind this one is to retrieve the extents asynchronously
   *
   */
  virtual void run();
  
  void setConnInfo( QString s );
  
  void setTableName( QString s );

  void setSqlWhereClause( QString s );

  void setGeometryColumn( QString s );
  
  // Feed this the object (e.g. Map Layer) for which you want to update the extents.
  //void setCallback( QgsPostgresProvider& o )
  void setCallback( QObject* o ) 
  {
    callbackObject = o;
  } 

// signals:    
      
  // Presumably this reimplements the equivalent in QgsPostgresProvider > QgsVectorLayer
  /** This is used to send a request that any mapcanvas using this layer update its extents */
//  void recalculateExtents();


private:
  
  /**
   * Connection pointer
   */
  PGconn *connection;
    
  /**
   * 
   */
  QString connInfo;

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
  
  /**
   * Integer that contains the row count (including non-geometry rows) of the layer
   */
  long numberFeatures;
    
};

#endif
