/***************************************************************************
    qgspostgresfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESFEATUREITERATOR_H
#define QGSPOSTGRESFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QQueue>


class QgsPostgresProvider;
class QgsPostgresResult;

class QgsPostgresFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsPostgresFeatureIterator( QgsPostgresProvider* p, const QgsFeatureRequest& request );

    ~QgsPostgresFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsPostgresProvider* P;

    QString whereClauseRect();
    bool getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature );
    void getFeatureAttribute( int idx, QgsPostgresResult& queryResult, int row, int& col, QgsFeature& feature );
    bool declareCursor( const QString& whereClause );

    QString mCursorName;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from PostgreSQL
     */
    QQueue<QgsFeature> mFeatureQueue;

    //! Maximal size of the feature queue
    int mFeatureQueueSize;

    //!< Number of retrieved features
    int mFetched;

    static const int sFeatureQueueSize;

};

#endif // QGSPOSTGRESFEATUREITERATOR_H
