/***************************************************************************
    qgsglobefeatureutils.h
     --------------------------------------
    Date                 : 11.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBEFEATUREUTILS_H
#define QGSGLOBEFEATUREUTILS_H

#include <osgEarthFeatures/Feature>
#include <osg/ValueObject>

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

class QgsGlobeFeatureUtils
{
  public:
    static inline QgsPoint qgsPointFromPoint( const osg::Vec3d &pt )
    {
      return QgsPoint( QgsWkbTypes::PointZ, pt.x(), pt.y(), pt.z() );
    }

    static inline osg::Vec3d pointFromQgsPoint( const QgsPoint &pt )
    {
      return osg::Vec3d( pt.x(), pt.y(), pt.z() );
    }

    static inline osgEarth::Features::LineString *lineStringFromQgsLineString( const QgsLineString *lineString )
    {
      QgsLineString *linearString = lineString->curveToLine();
      osgEarth::Features::LineString *retLineString = new osgEarth::Features::LineString();
      for ( int iVtx = 0, nVtx = linearString->vertexCount(); iVtx < nVtx; ++iVtx )
      {
        retLineString->push_back( pointFromQgsPoint( linearString->vertexAt( QgsVertexId( 0, 0, iVtx ) ) ) );
      }
      delete linearString;
      return retLineString;
    }

    static inline osgEarth::Features::Polygon *polygonFromQgsPolygon( const QgsPolygon *polygon )
    {
      QgsPolygon *linearPolygon = polygon->toPolygon();
      // a ring for osg earth is open (first point != last point)
      // an outer ring is oriented CCW, an inner ring is oriented CW
      osgEarth::Features::Polygon *retPoly = new osgEarth::Features::Polygon();

      // the outer ring
      for ( int iVtx = 0, nVtx = linearPolygon->vertexCount( 0, 0 ); iVtx < nVtx; ++iVtx )
      {
        retPoly->push_back( pointFromQgsPoint( linearPolygon->vertexAt( QgsVertexId( 0, 0, iVtx ) ) ) );
      }
      retPoly->rewind( osgEarth::Symbology::Ring::ORIENTATION_CCW );

      for ( int iRing = 1, nRings = linearPolygon->ringCount( 0 ); iRing < nRings; ++iRing )
      {
        osgEarth::Features::Ring *innerRing = new osgEarth::Features::Ring();
        for ( int iVtx = 0, nVtx = linearPolygon->vertexCount( 0, iRing ); iVtx < nVtx; ++iVtx )
        {
          innerRing->push_back( pointFromQgsPoint( linearPolygon->vertexAt( QgsVertexId( 0, iRing, iVtx ) ) ) );
        }
        innerRing->rewind( osgEarth::Symbology::Ring::ORIENTATION_CW );
        retPoly->getHoles().push_back( osg::ref_ptr<osgEarth::Features::Ring>( innerRing ) );
      }
      delete linearPolygon;
      return retPoly;
    }

    static inline osgEarth::Features::Geometry *geometryFromQgsGeometry( const QgsGeometry &geom )
    {
#if 0
      // test srid
      std::cout << "geom = " << &geom << std::endl;
      std::cout << "wkb = " << geom.asWkb() << std::endl;
      uint32_t srid;
      memcpy( &srid, geom.asWkb() + 2 + sizeof( void * ), sizeof( uint32_t ) );
      std::cout << "srid = " << srid << std::endl;
#endif

      switch ( QgsWkbTypes::flatType( geom.constGet()->wkbType() ) )
      {
        case QgsWkbTypes::Point:
        {
          osgEarth::Features::PointSet *pointSet = new osgEarth::Features::PointSet();
          pointSet->push_back( pointFromQgsPoint( *static_cast<const QgsPoint *>( geom.constGet() ) ) );
          return pointSet;
        }

        case QgsWkbTypes::MultiPoint:
        {
          osgEarth::Features::PointSet *pointSet = new osgEarth::Features::PointSet();
          const QgsMultiPoint *multiPoint = static_cast<const QgsMultiPoint *>( geom.constGet() );
          for ( int i = 0, n = multiPoint->numGeometries(); i < n; ++i )
          {
            pointSet->push_back( pointFromQgsPoint( *static_cast<const QgsPoint *>( multiPoint->geometryN( i ) ) ) );
          }
          return pointSet;
        }

        case QgsWkbTypes::LineString:
        case QgsWkbTypes::CircularString:
        case QgsWkbTypes::CompoundCurve:
        {
          return lineStringFromQgsLineString( static_cast<const QgsLineString *>( geom.constGet() ) );
        }

        case QgsWkbTypes::MultiLineString:
        {
          osgEarth::Features::MultiGeometry *multiGeometry = new osgEarth::Features::MultiGeometry();
          const QgsMultiLineString *multiLineString = static_cast<const QgsMultiLineString *>( geom.constGet() );
          for ( int i = 0, n = multiLineString->numGeometries(); i < n; ++i )
          {
            multiGeometry->getComponents().push_back( lineStringFromQgsLineString( static_cast<const QgsLineString *>( multiLineString->geometryN( i ) ) ) );
          }
          return multiGeometry;
        }

        case QgsWkbTypes::Polygon:
        case QgsWkbTypes::CurvePolygon:
        {
          return polygonFromQgsPolygon( static_cast<const QgsPolygon *>( geom.constGet() ) );
        }

        case QgsWkbTypes::MultiPolygon:
        {
          osgEarth::Features::MultiGeometry *multiGeometry = new osgEarth::Features::MultiGeometry();
          const QgsMultiPolygon *multiPolygon = static_cast<const QgsMultiPolygon *>( geom.constGet() );
          for ( int i = 0, n = multiPolygon->numGeometries(); i < n; ++i )
          {
            multiGeometry->getComponents().push_back( polygonFromQgsPolygon( static_cast<const QgsPolygon *>( multiPolygon->geometryN( i ) ) ) );
          }
          return multiGeometry;
        }

        default:
          break;
      }
      return 0;
    }

    static osgEarth::Features::Feature *featureFromQgsFeature( QgsVectorLayer *layer, QgsFeature &feat )
    {
      osgEarth::Features::Geometry *nGeom = geometryFromQgsGeometry( feat.geometry() );
      osgEarth::SpatialReference *ref = osgEarth::SpatialReference::create( layer->crs().toWkt().toStdString() );
      osgEarth::Features::Feature *retFeat = new osgEarth::Features::Feature( nGeom, ref, osgEarth::Style(), feat.id() );

      const QgsFields fields = layer->fields();
      const QgsAttributes &attrs = feat.attributes();

      for ( int idx = 0, numFlds = fields.size(); idx < numFlds; ++idx )
      {
        setFeatureField( retFeat, fields.at( idx ), attrs[idx] );
      }
      retFeat->setUserValue( "qgisLayerId", layer->id().toStdString() );

      return retFeat;
    }

    static void setFeatureField( osgEarth::Features::Feature *feature, const QgsField &field, const QVariant &value )
    {
      std::string name = field.name().toStdString();
      switch ( field.type() )
      {
        case QVariant::Bool:
          if ( !value.isNull() )
            feature->set( name, value.toBool() );
          else
            feature->setNull( name, osgEarth::Features::ATTRTYPE_BOOL );

          break;

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
          if ( !value.isNull() )
            feature->set( name, value.toInt() );
          else
            feature->setNull( name, osgEarth::Features::ATTRTYPE_INT );

          break;

        case QVariant::Double:
          if ( !value.isNull() )
            feature->set( name, value.toDouble() );
          else
            feature->setNull( name, osgEarth::Features::ATTRTYPE_DOUBLE );

          break;

        case QVariant::Char:
        case QVariant::String:
        default:
          if ( !value.isNull() )
            feature->set( name, value.toString().toStdString() );
          else
            feature->setNull( name, osgEarth::Features::ATTRTYPE_STRING );

          break;
      }
    }

    static osgEarth::Features::FeatureSchema schemaForFields( const QgsFields &fields )
    {
      osgEarth::Features::FeatureSchema schema;

      for ( int idx = 0, numFlds = fields.size(); idx < numFlds; ++idx )
      {
        const QgsField &fld = fields.at( idx );
        std::string name = fld.name().toStdString();

        switch ( fld.type() )
        {
          case QVariant::Bool:
            schema.insert( std::make_pair( name, osgEarth::Features::ATTRTYPE_BOOL ) );
            break;

          case QVariant::Int:
          case QVariant::UInt:
          case QVariant::LongLong:
          case QVariant::ULongLong:
            schema.insert( std::make_pair( name, osgEarth::Features::ATTRTYPE_INT ) );
            break;

          case QVariant::Double:
            schema.insert( std::make_pair( name, osgEarth::Features::ATTRTYPE_DOUBLE ) );
            break;

          case QVariant::Char:
          case QVariant::String:
          default:
            schema.insert( std::make_pair( name, osgEarth::Features::ATTRTYPE_STRING ) );
            break;
        }
      }
      return schema;
    }
};

#endif // QGSGLOBEFEATUREUTILS_H
