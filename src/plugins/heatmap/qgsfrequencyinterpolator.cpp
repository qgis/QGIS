/***************************************************************************
                                qgsidwinterpolator.cpp
                                ----------------------
    begin                : January 01, 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfrequencyinterpolator.h"
#include <cmath>
#include <limits>

QgsFrequencyInterpolator::QgsFrequencyInterpolator( const QList<LayerData>& layerData ): QgsInterpolator( layerData ), mBufferRadius( 5.0 ), mDecayLimit( 0.5 )
{

}

QgsFrequencyInterpolator::QgsFrequencyInterpolator(): QgsInterpolator( QList<LayerData>() ), mBufferRadius( 5.0 ), mDecayLimit( 0.5 )
{

}

QgsFrequencyInterpolator::~QgsFrequencyInterpolator()
{

}

int QgsFrequencyInterpolator::interpolatePoint( double x, double y, double& result )
{
    double distance;
    double minDistance = 100.0; // An aribitrarily high value

    if( !mDataIsCached )
    {
        cacheBaseData();
    }

    QVector<vertexData>::iterator vertex_it = mCachedBaseData.begin();

    for( ; vertex_it != mCachedBaseData.end(); ++vertex_it)
    {
        distance = sqrt(( vertex_it->x - x ) * ( vertex_it->x - x ) + ( vertex_it->y - y ) * ( vertex_it->y - y ));
        if( distance < minDistance )
        {
            minDistance = distance;
        }
    }

    result = 1/minDistance;

    return 0;

}
