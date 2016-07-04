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

#ifndef PAL_UTIL_H
#define PAL_UTIL_H

#include "pal.h"
#include "rtree.hpp"
#include "pointset.h"
#include "qgsgeometry.h"

namespace pal
{

  class LabelPosition;
  class Layer;
  class FeaturePart;

  /**
   * \ingroup core
   * \brief For usage in problem solving algorithm
   * \note not available in Python bindings
   */
  class Feats
  {
    public:
      Feats()
          : feature( nullptr )
          , shape( nullptr )
          , priority( 0 )
      {}

      FeaturePart *feature;
      PointSet *shape;
      double priority;
      QList< LabelPosition*> lPos;
  };


  typedef struct _elementary_transformation
  {
    int feat;
    int  old_label;
    int  new_label;
  } ElemTrans;

  struct Point
  {
    double x, y;
  };

#define EPSILON 1e-9

  /**
   * \class pal::Util
   * \note not available in Python bindings
   * \ingroup core
   */
  class Util
  {
    public:
      /**
       * \brief Sort an array of pointers
       * \param items arays of pointers to sort
       * \param N number of items
       * \param greater function to compare two items
       **/
      static void sort( void** items, int N, bool ( *greater )( void *l, void *r ) );

      static QLinkedList<const GEOSGeometry*>* unmulti( const GEOSGeometry* the_geom );
  };


} // namespace

#endif
