/***************************************************************************
    qgsappcoordinateoperationhandlers.h
    -------------------------
    begin                : May 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPCOORDINATEOPERATIONHANDLERS_H
#define QGSAPPCOORDINATEOPERATIONHANDLERS_H

#include <QObject>
#include "qgscoordinatereferencesystem.h"
#include "qgsdatumtransform.h"

/**
 * Alerts users when a transform grids are required (or desired) for an operation between two
 * CRSes, yet they are not available on the current system.
 */
class QgsAppMissingGridHandler : public QObject
{
    Q_OBJECT
  public:

    QgsAppMissingGridHandler( QObject *parent );

  signals:

    void missingRequiredGrid( const QgsCoordinateReferenceSystem &sourceCrs,
                              const QgsCoordinateReferenceSystem &destinationCrs,
                              const QgsDatumTransform::GridDetails &grid );

    void missingPreferredGrid( const QgsCoordinateReferenceSystem &sourceCrs,
                               const QgsCoordinateReferenceSystem &destinationCrs,
                               const QgsDatumTransform::TransformDetails &preferredOperation,
                               const QgsDatumTransform::TransformDetails &availableOperation );

    void coordinateOperationCreationError( const QgsCoordinateReferenceSystem &sourceCrs,
                                           const QgsCoordinateReferenceSystem &destinationCrs,
                                           const QString &error );

    void missingGridUsedByContextHandler( const QgsCoordinateReferenceSystem &sourceCrs,
                                          const QgsCoordinateReferenceSystem &destinationCrs,
                                          const QgsDatumTransform::TransformDetails &desired );

  private slots:

    void onMissingRequiredGrid( const QgsCoordinateReferenceSystem &sourceCrs,
                                const QgsCoordinateReferenceSystem &destinationCrs,
                                const QgsDatumTransform::GridDetails &grid );

    void onMissingPreferredGrid( const QgsCoordinateReferenceSystem &sourceCrs,
                                 const QgsCoordinateReferenceSystem &destinationCrs,
                                 const QgsDatumTransform::TransformDetails &preferredOperation,
                                 const QgsDatumTransform::TransformDetails &availableOperation );

    void onCoordinateOperationCreationError( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        const QString &error );

    void onMissingGridUsedByContextHandler( const QgsCoordinateReferenceSystem &sourceCrs,
                                            const QgsCoordinateReferenceSystem &destinationCrs,
                                            const QgsDatumTransform::TransformDetails &desired );
  private:

    bool shouldWarnAboutPair( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest );
    bool shouldWarnAboutPairForCurrentProject( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &dest );

    QList< QPair< QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem > > mAlreadyWarnedPairs;
    QList< QPair< QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem > > mAlreadyWarnedPairsForProject;

};

#endif // QGSAPPCOORDINATEOPERATIONHANDLERS_H
