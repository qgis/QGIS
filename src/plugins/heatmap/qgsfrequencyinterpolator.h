/***************************************************************************
                        qgsfrequencyinterpolator.h
                        --------------------------
    begin           : Janauary 01, 2012
    copyright       : (C) 2012 by Arunmozhi
    email           : aruntheguy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFREQUENCYINTERPOLATOR_H
#define QGSFREQUENCYINTERPOLATOR_H

#include <qgsinterpolator.h>

class ANALYSIS_EXPORT QgsFrequencyInterpolator: public QgsInterpolator
{
    public:
       QgsFrequencyInterpolator( const QList<LayerData>& layerData );
       ~QgsFrequencyInterpolator();

       int interpolatePoint( double x, double y, double& result );

       void setBufferRadius( double p ) { mBufferRadius = p; }
       void setDecayLimit( double d ) { mDecayLimit = d; }

    private:
       QgsFrequencyInterpolator(); //forbidden

       double mBufferRadius;
       double mDecayLimit;
};

#endif
