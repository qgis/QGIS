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

#define SIP_NO_FILE

#ifndef QGS_GEOMETRY_CHECKER_H
#define QGS_GEOMETRY_CHECKER_H

#include <QFuture>
#include <QList>
#include <QMutex>
#include <QStringList>
#include "qgis_analysis.h"
#include "qgsfeatureid.h"

struct QgsGeometryCheckerContext;
class QgsGeometryCheck;
class QgsGeometryCheckError;
class QgsMapLayer;
class QgsVectorLayer;
class QMutex;

class ANALYSIS_EXPORT QgsGeometryChecker : public QObject
{
    Q_OBJECT
  public:
    QgsGeometryChecker( const QList<QgsGeometryCheck *> &checks, QgsGeometryCheckerContext *context );
    ~QgsGeometryChecker() override;
    QFuture<void> execute( int *totalSteps = nullptr );
    bool fixError( QgsGeometryCheckError *error, int method, bool triggerRepaint = false );
    const QList<QgsGeometryCheck *> getChecks() const { return mChecks; }
    QStringList getMessages() const { return mMessages; }
    void setMergeAttributeIndices( const QMap<QString, int> &mergeAttributeIndices ) { mMergeAttributeIndices = mergeAttributeIndices; }
    QgsGeometryCheckerContext *getContext() const { return mContext; }

  signals:
    void errorAdded( QgsGeometryCheckError *error );
    void errorUpdated( QgsGeometryCheckError *error, bool statusChanged );
    void progressValue( int value );

  private:
    class RunCheckWrapper
    {
      public:
        explicit RunCheckWrapper( QgsGeometryChecker *instance ) : mInstance( instance ) {}
        void operator()( const QgsGeometryCheck *check ) { mInstance->runCheck( check ); }
      private:
        QgsGeometryChecker *mInstance = nullptr;
    };

    QList<QgsGeometryCheck *> mChecks;
    QgsGeometryCheckerContext *mContext;
    QList<QgsGeometryCheckError *> mCheckErrors;
    QStringList mMessages;
    QMutex mErrorListMutex;
    QMap<QString, int> mMergeAttributeIndices;
    QAtomicInt mProgressCounter;

    void runCheck( const QgsGeometryCheck *check );

  private slots:
    void emitProgressValue();
};

#endif // QGS_GEOMETRY_CHECKER_H
