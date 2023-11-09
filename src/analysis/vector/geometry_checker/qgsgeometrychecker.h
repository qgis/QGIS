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
#include "qgsfeedback.h"
#include "qgsfeatureid.h"

typedef qint64 QgsFeatureId;
class QgsGeometryCheckContext;
class QgsGeometryCheck;
class QgsGeometryCheckError;
class QgsMapLayer;
class QgsVectorLayer;
class QgsFeaturePool;
class QMutex;

/**
 * \ingroup analysis
 *
 * \brief Manages and runs a set of geometry checks.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryChecker : public QObject
{
    Q_OBJECT
  public:
    QgsGeometryChecker( const QList<QgsGeometryCheck *> &checks, QgsGeometryCheckContext *context SIP_TRANSFER, const QMap<QString, QgsFeaturePool *> &featurePools );
    ~QgsGeometryChecker() override;
    QFuture<void> execute( int *totalSteps = nullptr );
    bool fixError( QgsGeometryCheckError *error, int method, bool triggerRepaint = false );
    const QList<QgsGeometryCheck *> getChecks() const { return mChecks; }
    QStringList getMessages() const { return mMessages; }
    void setMergeAttributeIndices( const QMap<QString, int> &mergeAttributeIndices ) { mMergeAttributeIndices = mergeAttributeIndices; }
    QgsGeometryCheckContext *getContext() const { return mContext; }
    const QMap<QString, QgsFeaturePool *> &featurePools() const {return mFeaturePools;}

  signals:
    void errorAdded( QgsGeometryCheckError *error );
    void errorUpdated( QgsGeometryCheckError *error, bool statusChanged );
    void progressValue( int value );

  private:
    class RunCheckWrapper
    {
      public:
        explicit RunCheckWrapper( QgsGeometryChecker *instance );
        void operator()( const QgsGeometryCheck *check );
      private:
        QgsGeometryChecker *mInstance = nullptr;
    };

    QList<QgsGeometryCheck *> mChecks;
    QgsGeometryCheckContext *mContext = nullptr;
    QList<QgsGeometryCheckError *> mCheckErrors;
    QStringList mMessages;
    QMutex mErrorListMutex;
    QMap<QString, int> mMergeAttributeIndices;
    QgsFeedback mFeedback;
    QMap<QString, QgsFeaturePool *> mFeaturePools;

    void runCheck( const QMap<QString, QgsFeaturePool *> &featurePools, const QgsGeometryCheck *check );

  private slots:
    void emitProgressValue();
};

#endif // QGS_GEOMETRY_CHECKER_H
