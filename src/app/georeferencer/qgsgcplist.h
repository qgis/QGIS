/***************************************************************************
    qgsgcplist.h - GCP list class
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GCP_LIST_H
#define QGS_GCP_LIST_H

#include <QList>
#include <QVector>
#include "qgis_app.h"
#include "qgsunittypes.h"

class QgsGeorefDataPoint;
class QgsGcpPoint;
class QgsPointXY;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;
class QgsGeorefTransform;

/**
 * A container for GCP data points.
 *
 * The container does NOT own the points -- they have to be manually deleted elsewhere!!
 */
class APP_EXPORT QgsGCPList : public QList<QgsGeorefDataPoint * >
{
  public:
    QgsGCPList() = default;
    QgsGCPList( const QgsGCPList &list ) = delete;
    QgsGCPList &operator =( const QgsGCPList &list ) = delete;

    /**
     * Creates vectors of source and destination points, where the destination points are all transformed to the
     * specified \a targetCrs.
     */
    void createGCPVectors( QVector<QgsPointXY> &sourcePoints, QVector<QgsPointXY> &destinationPoints,
                           const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const;

    /**
     * Returns the count of currently enabled data points.
     */
    int countEnabledPoints() const;

    /**
     * Updates the stored residual sizes for points in the list.
     *
     * \param georefTransform transformation to use for residual calculation
     * \param targetCrs georeference output CRS
     * \param context transform context
     * \param residualUnit units for residual calculation. Supported values are QgsUnitTypes::RenderPixels or QgsUnitTypes::RenderMapUnits
     */
    void updateResiduals( QgsGeorefTransform *georefTransform,
                          const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context,
                          QgsUnitTypes::RenderUnit residualUnit );

    /**
     * Returns the container as a list of GCP points.
     */
    QList< QgsGcpPoint > asPoints() const;

    /**
     * Saves the GCPs to a text file.
     *
     * \param filePath destination file path
     * \param targetCrs target CRS for destination points
     * \param context transform context
     * \param error will be set to a descriptive error message if saving fails
     *
     * \returns TRUE on success
     */
    bool saveGcps( const QString &filePath,
                   const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context,
                   QString &error ) const;

    /**
     * Loads GCPs from a text file.
     *
     * \param filePath source file path
     * \param defaultDestinationCrs default CRS to use for destination points if no destination CRS information is present in text file.
     * \param actualDestinationCrs will be set to actual destination CRS for points, which is either the CRS information from the text file OR the defaultDestinationCrs
     * \param error will be set to a descriptive error message if loading fails
     *
     * \returns TRUE on success
     */
    static QList< QgsGcpPoint > loadGcps( const QString &filePath,
                                          const QgsCoordinateReferenceSystem &defaultDestinationCrs,
                                          QgsCoordinateReferenceSystem &actualDestinationCrs,
                                          QString &error );

};

#endif
