/***************************************************************************
  QgsAttributeTableLoadWorker.h - Worker loader for cached features
  -------------------
         date                 : November 2015
         copyright            : Alessandro Pasotti
         email                : elpaso (at) itopen (dot) it

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLELOADWORKER_H
#define QGSATTRIBUTETABLELOADWORKER_H

#include <QObject>

#include "qgsfeatureiterator.h"

/**
 * A worker that iterates through features in a vector cache
 *
 */
class GUI_EXPORT QgsAttributeTableLoadWorker: public QObject
{

    Q_OBJECT

  public:
    /**
     * Constructor
     * @param features    The feature iterator
     */
    QgsAttributeTableLoadWorker( const QgsFeatureIterator &features );

    /**
     * Destructor
     */
    ~QgsAttributeTableLoadWorker( );

    /**
     * @brief isRunning
     * @return return true if the worker is running
     */
    bool isRunning() { return mIsRunning; }

    /**
     * Stop the worker
     */
    void stopJob();

  public slots:
    /**
     * Start pulling features from the cache
     */
    void startJob( );

  signals:

    /**
     * No more features to fetch
     */
    void finished();

    /**
     * Launched when a feature has been fetched
     * @param features feature list
     * @param loadedCount The number of features already loaded
     */
    void featuresReady( QgsFeatureList features, int loadedCount );

  private:

    bool mIsRunning;
    bool mStopped;
    QgsFeatureIterator mFeatures;
};


#endif
