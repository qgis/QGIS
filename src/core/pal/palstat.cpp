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

#include "palstat.h"

pal::PalStat::PalStat()
{
  nbLayers = 0;
  nbObjects = 0;
  nbLabelledObjects = 0;
  layersNbObjects = nullptr;
  layersNbLabelledObjects = nullptr;
}

pal::PalStat::~PalStat()
{
  delete[] layersNbObjects;
  delete[] layersNbLabelledObjects;
}

int pal::PalStat::getNbObjects()
{
  return nbObjects;
}

int pal::PalStat::getNbLabelledObjects()
{
  return nbLabelledObjects;
}

int pal::PalStat::getNbLayers()
{
  return nbLayers;
}

QString pal::PalStat::getLayerName( int layerId )
{
  if ( layerId >= 0 && layerId < nbLayers )
    return layersName.at( layerId );
  else
    return QString();
}

int pal::PalStat::getLayerNbObjects( int layerId )
{
  if ( layerId >= 0 && layerId < nbLayers )
    return layersNbObjects[layerId];
  else
    return -1;
}

int pal::PalStat::getLayerNbLabelledObjects( int layerId )
{
  if ( layerId >= 0 && layerId < nbLayers )
    return layersNbLabelledObjects[layerId];
  else
    return -1;
}

