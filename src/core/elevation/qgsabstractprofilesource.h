/***************************************************************************
                         qgsabstractprofilesource.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSABSTRACTPROFILESOURCE_H
#define QGSABSTRACTPROFILESOURCE_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsProfileRequest;
class QgsAbstractProfileGenerator;

#include <QUuid>

/**
 * \brief Interface for classes which can generate elevation profiles.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileSource
{

  public:

    virtual ~QgsAbstractProfileSource();

    /**
     * Given a profile \a request, returns a new profile generator ready for generating
     * elevation profiles.
     *
     * The caller takes ownership of the returned generator.
     *
     * May return NULLPTR if the source cannot generate a profile at this time.
     */
    virtual QgsAbstractProfileGenerator *createProfileGenerator( const QgsProfileRequest &request ) = 0 SIP_FACTORY;

    /**
     * Returns a unique identifier for this profile source.
     *
     * For map layer sources, the source ID will match the layer's QgsMapLayer::id().
     * Other (non-map-layer) sources will have a different unique ID with its own custom interpretation.
     *
     * \since QGIS 4.0.0
     */
    virtual QString profileSourceId() const { return mSourceId; }; // TODO QGIS 5.0: Make it pure virtual

    /**
     * Returns a name for displaying this profile source in the elevation profile layer tree.
     *
     * For map layer sources, the name displayed in the elevation profile tree will be taken from and synchronized to the layer's name.
     *
     * \since QGIS 4.0.0
     */
    virtual QString profileSourceName() const { return QString(); }; // TODO QGIS 5.0: Make it pure virtual

  private:
    QString mSourceId = QUuid::createUuid().toString(); // Support legacy sources with a uuid

};

#endif // QGSABSTRACTPROFILESOURCE_H
