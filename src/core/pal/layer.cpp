/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE

#include <stddef.h>
#include <geos_c.h>

#include <iostream>
#include <cstring>
#include <cmath>


#include <pal/pal.h>
#include <pal/layer.h>
#include <pal/palexception.h>
#include <pal/internalexception.h>

#include "linkedlist.hpp"
#include "hashtable.hpp"

#include "feature.h"
#include "geomfunction.h"
#include "util.h"

#include "simplemutex.h"

namespace pal
{

  Layer::Layer( const char *lyrName, double min_scale, double max_scale, Arrangement arrangement, Units label_unit, double defaultPriority, bool obstacle, bool active, bool toLabel, Pal *pal )
      :  pal( pal ), obstacle( obstacle ), active( active ),
         toLabel( toLabel ), label_unit( label_unit ),
         min_scale( min_scale ), max_scale( max_scale ),
         arrangement( arrangement ), arrangementFlags( 0 )
  {

    this->name = new char[strlen( lyrName ) +1];
    strcpy( this->name, lyrName );

    modMutex = new SimpleMutex();

    rtree = new RTree<FeaturePart*, double, 2, double>();
    hashtable = new HashTable<Feature*> ( 5281 );

    if ( defaultPriority < 0.0001 )
      this->defaultPriority = 0.0001;
    else if ( defaultPriority > 1.0 )
      this->defaultPriority = 1.0;
    else
      this->defaultPriority = defaultPriority;

    featureParts = new LinkedList<FeaturePart*> ( ptrFeaturePartCompare );
    features = new LinkedList<Feature*> ( ptrFeatureCompare );
  }

  Layer::~Layer()
  {
    modMutex->lock();

    if ( featureParts )
    {
      while ( featureParts->size() )
      {
        delete featureParts->pop_front();
      }
      delete featureParts;
    }

    // features in the hashtable
    if ( features )
    {
      while ( features->size() )
      {
        delete features->pop_front();
      }
      delete features;
    }

    if ( name )
      delete[] name;

    delete rtree;

    delete hashtable;
    delete modMutex;
  }

  Feature* Layer::getFeature( const char* geom_id )
  {
    Feature** fptr = hashtable->find(geom_id);
    return (fptr ? *fptr : NULL);
  }


  bool Layer::isScaleValid( double scale )
  {
    return ( scale >= min_scale || min_scale == -1 )
           && ( scale <= max_scale || max_scale == -1 );
  }


  int Layer::getNbFeatures()
  {
    return features->size();
  }

  const char *Layer::getName()
  {
    return name;
  }

  Arrangement Layer::getArrangement()
  {
    return arrangement;
  }

  void Layer::setArrangement( Arrangement arrangement )
  {
    this->arrangement = arrangement;
  }


  bool Layer::isObstacle()
  {
    return obstacle;
  }

  bool Layer::isToLabel()
  {
    return toLabel;
  }

  bool Layer::isActive()
  {
    return active;
  }


  double Layer::getMinScale()
  {
    return min_scale;
  }

  double Layer::getMaxScale()
  {
    return max_scale;
  }

  double Layer::getPriority()
  {
    return defaultPriority;
  }

  void Layer::setObstacle( bool obstacle )
  {
    this->obstacle = obstacle;
  }

  void Layer::setActive( bool active )
  {
    this->active = active;
  }

  void Layer::setToLabel( bool toLabel )
  {
    this->toLabel = toLabel;
  }

  void Layer::setMinScale( double min_scale )
  {
    this->min_scale = min_scale;
  }

  void Layer::setMaxScale( double max_scale )
  {
    this->max_scale = max_scale;
  }

  void Layer::setPriority( double priority )
  {
    if ( priority >= 1.0 ) // low priority
      defaultPriority = 1.0;
    else if ( priority <= 0.0001 )
      defaultPriority = 0.0001; // high priority
    else
      defaultPriority = priority;
  }



bool Layer::registerFeature( const char *geom_id, PalGeometry *userGeom, double label_x, double label_y )
{
  if ( !geom_id || label_x < 0 || label_y < 0 )
    return false;

  modMutex->lock();

  if ( hashtable->find( geom_id ) )
  {
    modMutex->unlock();
    throw new PalException::FeatureExists();
    return false;
  }

  // Split MULTI GEOM and Collection in simple geometries
  GEOSGeometry *the_geom = userGeom->getGeosGeometry();

  Feature* f = new Feature( this, geom_id, userGeom, label_x, label_y );

  bool first_feat = true;

  // break the (possibly multi-part) geometry into simple geometries
  LinkedList <const GEOSGeometry*> *simpleGeometries = unmulti( the_geom );
  
  while ( simpleGeometries->size() > 0 )
  {
    const GEOSGeometry* geom = simpleGeometries->pop_front();

    // ignore invalid geometries (e.g. polygons with self-intersecting rings)
    if (GEOSisValid( geom ) != 1) // 0=invalid, 1=valid, 2=exception
    {
      std::cerr << "ignoring invalid feature " << geom_id << std::endl;
      continue;
    }

    int type = GEOSGeomTypeId( geom );

    if (type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON)
      throw InternalException::UnknownGeometry();

    FeaturePart* fpart = new FeaturePart(f, geom);

    // ignore invalid geometries
    if ( (type == GEOS_LINESTRING && fpart->nbPoints < 2) ||
         (type == GEOS_POLYGON && fpart->nbPoints < 3) )
    {
      delete fpart;
      continue;
    }

    // polygons: reorder coordinates
    if ( type == GEOS_POLYGON && reorderPolygon( fpart->nbPoints, fpart->x, fpart->y ) != 0 )
    {
      delete fpart;
      continue;
    }

    // feature part is ready!

    double bmin[2];
    double bmax[2];
    fpart->getBoundingBox(bmin, bmax);

    // add to list of layer's feature parts
    featureParts->push_back( fpart );

    // add to r-tree for fast spatial access
    rtree->Insert( bmin, bmax, fpart );

    first_feat = false;
  }
  delete simpleGeometries;

  userGeom->releaseGeosGeometry( the_geom );

  modMutex->unlock();

  // add feature to layer if we have added something
  if (!first_feat)
  {
    features->push_back( f );
    hashtable->insertItem( geom_id, f );
  }

  return !first_feat; // true if we've added something
}


void Layer::setLabelUnit( Units label_unit )
{
  if ( label_unit == PIXEL || label_unit == METER )
    this->label_unit = label_unit;
}

Units Layer::getLabelUnit()
{
  return label_unit;
}



} // end namespace

