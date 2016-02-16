/***************************************************************************
 *  qgsgeometrychecker.h                                                   *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_CHECKER_H
#define QGS_GEOMETRY_CHECKER_H

#include <QFuture>
#include <QList>
#include <QMutex>
#include <QStringList>

typedef qint64 QgsFeatureId;
typedef QSet<QgsFeatureId> QgsFeatureIds;
class QgsFeaturePool;
class QgsGeometryCheck;
class QgsGeometryCheckError;
class QgsVectorLayer;
class QMutex;

class QgsGeometryChecker : public QObject
{
    Q_OBJECT
  public:
    QgsGeometryChecker( const QList<QgsGeometryCheck*>& checks, QgsFeaturePool* featurePool );
    ~QgsGeometryChecker();
    QFuture<void> execute( int* totalSteps = nullptr );
    bool fixError( QgsGeometryCheckError *error, int method );
    const QList<QgsGeometryCheck*> getChecks() const { return mChecks; }
    const QStringList& getMessages() const { return mMessages; }

  public slots:
    void setMergeAttributeIndex( int mergeAttributeIndex ) { mMergeAttributeIndex = mergeAttributeIndex; }

  signals:
    void errorAdded( QgsGeometryCheckError* error );
    void errorUpdated( QgsGeometryCheckError* error, bool statusChanged );
    void progressValue( int value );

  private:
    class RunCheckWrapper
    {
      public:
        explicit RunCheckWrapper( QgsGeometryChecker* instance ) : mInstance( instance ) {}
        void operator()( const QgsGeometryCheck* check ) { mInstance->runCheck( check ); }
      private:
        QgsGeometryChecker* mInstance;
    };

    QList<QgsGeometryCheck*> mChecks;
    QgsFeaturePool* mFeaturePool;
    QList<QgsGeometryCheckError*> mCheckErrors;
    QStringList mMessages;
    QMutex mErrorListMutex;
    int mMergeAttributeIndex;
    QAtomicInt mProgressCounter;

    void runCheck( const QgsGeometryCheck* check );

  private slots:
    void emitProgressValue();
};

#endif // QGS_GEOMETRY_CHECKER_H
