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

#include <iostream>
#include <pal/palstat.h>

namespace pal
{

  PalStat::PalStat()
  {
    nbLayers = 0;
    nbObjects = 0;
    nbLabelledObjects = 0;
    layersName = NULL;
    layersNbObjects = NULL;
    layersNbLabelledObjects = NULL;
  }

  PalStat::~PalStat()
  {
    int i;

    for ( i = 0; i < nbLayers; i++ )
    {
      delete[] layersName[i];
    }

    delete[] layersName;
    delete[] layersNbObjects;
    delete[] layersNbLabelledObjects;
  }

  int PalStat::getNbObjects()
  {
    return nbObjects;
  }

  int PalStat::getNbLabelledObjects()
  {
    return nbLabelledObjects;
  }

  int PalStat::getNbLayers()
  {
    return nbLayers;
  }

  const char * PalStat::getLayerName( int layerId )
  {
    if ( layerId >= 0 && layerId < nbLayers )
      return layersName[layerId];
    else
      return NULL;
  }


  int PalStat::getLayerNbObjects( int layerId )
  {
    if ( layerId >= 0 && layerId < nbLayers )
      return layersNbObjects[layerId];
    else
      return -1;
  }


  int PalStat::getLayerNbLabelledObjects( int layerId )
  {
    if ( layerId >= 0 && layerId < nbLayers )
      return layersNbLabelledObjects[layerId];
    else
      return -1;
  }


} // namespace

