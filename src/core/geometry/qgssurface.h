/***************************************************************************
                         qgssurface.h
                         --------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSURFACE_H
#define QGSSURFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractgeometry.h"
#include "qgsbox3d.h"

class QgsPolygon;

/**
 * \ingroup core
 * \class QgsSurface
 * \brief Surface geometry type.
 */
class CORE_EXPORT QgsSurface: public QgsAbstractGeometry
{
  public:

    QgsBox3D boundingBox3D() const override
    {
      if ( mBoundingBox.isNull() )
      {
        mBoundingBox = calculateBoundingBox3D();
      }
      return mBoundingBox;
    }

    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;


#ifndef SIP_RUN

    /**
     * Cast the \a geom to a QgsSurface.
     * Should be used by qgsgeometry_cast<QgsSurface *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsSurface *cast( const QgsAbstractGeometry *geom )
    {
      if ( !geom )
        return nullptr;

      const Qgis::WkbType flatType = QgsWkbTypes::flatType( geom->wkbType() );
      if ( flatType == Qgis::WkbType::CurvePolygon
           || flatType == Qgis::WkbType::Polygon
           || flatType == Qgis::WkbType::Triangle )
        return static_cast<const QgsSurface *>( geom );
      return nullptr;
    }
#endif
  protected:

    void clearCache() const override;

    mutable QgsBox3D mBoundingBox;
    mutable bool mHasCachedValidity = false;
    mutable QString mValidityFailureReason;
};

#endif // QGSSURFACE_H
