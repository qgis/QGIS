/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at >list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file graphbuilder.cpp
 * \brief implementation of RgGraphBuilder
 */

#include "graphbuilder.h"

// Qgis includes

RgGraphBuilder::RgGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled, double topologyTolerance ) :
    mCrs( crs ), mTopologyToleraceFactor( topologyTolerance ), mCoordinateTransformEnabled( ctfEnabled )
{

}

RgGraphBuilder::~RgGraphBuilder()
{

}

QgsCoordinateReferenceSystem& RgGraphBuilder::destinationCrs()
{
  return mCrs;
}

double RgGraphBuilder::topologyTolerance()
{
  return mTopologyToleraceFactor;
}

bool RgGraphBuilder::coordinateTransformEnabled() const
{
  return mCoordinateTransformEnabled;
}
