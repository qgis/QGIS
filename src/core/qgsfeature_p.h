/***************************************************************************
                      qgsfeature_p.h
                     ---------------
Date                 : May-2015
Copyright            : (C) 2015 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE_PRIVATE_H
#define QGSFEATURE_PRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

#include "qgsfields.h"

#include "qgsgeometry.h"

class QgsFeaturePrivate : public QSharedData
{
  public:

    explicit QgsFeaturePrivate( QgsFeatureId id )
      : fid( id )
      , valid( false )
    {
    }

    QgsFeaturePrivate( const QgsFeaturePrivate &other )
      : QSharedData( other )
      , fid( other.fid )
      , attributes( other.attributes )
      , geometry( other.geometry )
      , valid( other.valid )
      , fields( other.fields )
    {
    }

    ~QgsFeaturePrivate()
    {
    }

    //! Feature ID
    QgsFeatureId fid;

    //! Attributes accessed by field index
    QgsAttributes attributes;

    //! Geometry, may be empty if feature has no geometry
    QgsGeometry geometry;

    //! Flag to indicate if this feature is valid
    bool valid;

    //! Optional field map for name-based attribute lookups
    QgsFields fields;

  private:
    QgsFeaturePrivate &operator=( const QgsFeaturePrivate & ) = delete;

};

/// @endcond

#endif //QGSFEATURE_PRIVATE_H
