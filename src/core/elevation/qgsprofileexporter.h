/***************************************************************************
                         qgsprofileexporter.h
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROFILEEXPORTER_H
#define QGSPROFILEEXPORTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilerequest.h"

class QgsAbstractProfileSource;
class QgsAbstractProfileGenerator;

/**
 * \brief Handles exports of elevation profiles in various formats.
 *
 * \ingroup core
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProfileExporter
{

  public:

    /**
     * Constructor for QgsProfileExporter, using the provided list of profile \a sources to generate the
     * results.
     *
     * After construction, call run() to initiate the profile generation.
     */
    QgsProfileExporter( const QList< QgsAbstractProfileSource * > &sources,
                        const QgsProfileRequest &request,
                        Qgis::ProfileExportType type );

    //! QgsProfileExporter cannot be copied
    QgsProfileExporter( const QgsProfileExporter &other ) = delete;

    //! QgsProfileExporter cannot be copied
    QgsProfileExporter &operator=( const QgsProfileExporter &other ) = delete;

    ~QgsProfileExporter();

    /**
     * Runs the profile generation. This method must be called before retrieving any results from the
     * exporter.
     */
    void run( QgsFeedback *feedback = nullptr );

    /**
     * Returns a list of vector layer containing the exported profile results.
     *
     * While this method attempts to condense all results into a single layer, multiple layers may be returned
     * when the geometry types of exported features differs.
     *
     * Ownership of the returned layers is transferred to the caller.
     */
    QList< QgsVectorLayer * > toLayers() SIP_FACTORY;

  private:

#ifdef SIP_RUN
    QgsProfileExporter( const QgsProfileExporter &other );
#endif

    Qgis::ProfileExportType mType = Qgis::ProfileExportType::Features3D;
    QgsProfileRequest mRequest;
    std::vector< std::unique_ptr< QgsAbstractProfileGenerator > > mGenerators;
    QVector< QgsAbstractProfileResults::Feature > mFeatures;

};

#endif // QGSPROFILEEXPORTER_H
