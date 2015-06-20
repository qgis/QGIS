/***************************************************************************
                      qgsfeature_p.h
                     ---------------
Date                 : May-2015
Copyright            : (C) 2015 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE_PRIVATE_H
#define QGSFEATURE_PRIVATE_H

/// @cond

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgsfield.h"

#include "qgsgeometry.h"

class QgsFeaturePrivate : public QSharedData
{
  public:

    QgsFeaturePrivate( QgsFeatureId id )
        : fid( id )
        , geometry( 0 )
        , ownsGeometry( false )
        , valid( false )
    {
    }

    QgsFeaturePrivate( const QgsFeaturePrivate& other )
        : QSharedData( other )
        , fid( other.fid )
        , attributes( other.attributes )
        , geometry( other.ownsGeometry && other.geometry ? new QgsGeometry( *other.geometry ) : other.geometry )
        , ownsGeometry( other.ownsGeometry )
        , valid( other.valid )
        , fields( other.fields )
    {
    }

    ~QgsFeaturePrivate()
    {
      if ( ownsGeometry )
        delete geometry;
    }

    //! feature id
    QgsFeatureId fid;

    /** attributes accessed by field index */
    QgsAttributes attributes;

    /** pointer to geometry in binary WKB format

       This is usually set by a call to OGRGeometry::exportToWkb()
     */
    QgsGeometry *geometry;

    /** Indicator if the mGeometry is owned by this QgsFeature.
        If so, this QgsFeature takes responsibility for the mGeometry's destruction.
     */
    bool ownsGeometry;

    //! Flag to indicate if this feature is valid
    bool valid;

    //! Optional field map for name-based attribute lookups
    QgsFields fields;

};

/// @endcond

#endif //QGSFEATURE_PRIVATE_H
