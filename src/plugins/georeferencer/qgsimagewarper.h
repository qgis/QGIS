/***************************************************************************
     qgsimagewarper.h
     --------------------------------------
   Date                 : Sun Sep 16 12:03:20 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIMAGEWARPER_H
#define QGSIMAGEWARPER_H

#include <gdalwarper.h>
#include <QString>

#include <vector>
#include "qgspoint.h"

class QgsImageWarper
{
  public:

    enum ResamplingMethod
    {
      NearestNeighbour = GRA_NearestNeighbour,
      Bilinear = GRA_Bilinear,
      Cubic = GRA_Cubic,
    };


    QgsImageWarper() { };
    QgsImageWarper( double angle) : mAngle( angle ) { };

    void warp( const QString& input, const QString& output,
               double& xOffset, double& yOffset,
               ResamplingMethod resampling = Bilinear, 
               bool useZeroAsTrans = true, 
               const QString& compression = "NONE" );

      bool warpgcp( const QString& input, const QString& output,
                    const char *worldExt,
                    std::vector<QgsPoint> mapCoords,
                    std::vector<QgsPoint> pixelCoords,
                    const int nReqOrder = 1, ResamplingMethod resampling = Bilinear, 
                    bool useZeroAsTrans = true, const QString& compression = "NONE",
                    bool bUseTPS = false);

  private:

    struct TransformParameters
    {
      double angle;
      double x0;
      double y0;
    };

      bool openSrcDSAndGetWarpOpt(const QString &input, const QString &output,
                                  const ResamplingMethod &resampling, const GDALTransformerFunc &pfnTransform,
                                  GDALDatasetH &hSrcDS, GDALWarpOptions *&psWarpOptions);

    static int transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                          double *x, double *y, double *z, int *panSuccess );

    double mAngle;

};


#endif
