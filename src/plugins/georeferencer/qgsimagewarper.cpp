/***************************************************************************
     qgsimagewarper.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:03:14 AKDT 2007
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

#include <cmath>
#include <iostream>

#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal.h>
#include <gdalwarper.h>

#include <QFile>

#include "qgsimagewarper.h"

void QgsImageWarper::warp(const QString& input, const QString& output,
			  double& xOffset, double& yOffset, 
			  ResamplingMethod resampling, bool useZeroAsTrans, const QString& compression) {
  // Open input file
  GDALAllRegister();
  GDALDatasetH hSrcDS = GDALOpen(QFile::encodeName(input).constData(),GA_ReadOnly);
  // Setup warp options. 
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->nBandCount = GDALGetRasterCount(hSrcDS);
  psWarpOptions->panSrcBands = 
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  psWarpOptions->panDstBands = 
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
  for (int i = 0; i < psWarpOptions->nBandCount; ++i) {
    psWarpOptions->panSrcBands[i] = i + 1;
    psWarpOptions->panDstBands[i] = i + 1;
  }    
  psWarpOptions->pfnProgress = GDALTermProgress;   
  psWarpOptions->pfnTransformer = &QgsImageWarper::transform;
  psWarpOptions->eResampleAlg = GDALResampleAlg(resampling);
  
  // check the bounds for the warped raster
  // order: upper right, lower right, lower left (y points down)
  double x[] = { GDALGetRasterXSize(hSrcDS), GDALGetRasterXSize(hSrcDS), 0 };
  double y[] = { 0, GDALGetRasterYSize(hSrcDS), GDALGetRasterYSize(hSrcDS) };
  int s[] = { 0, 0, 0 };
  TransformParameters tParam = { mAngle, 0, 0 };
  transform(&tParam, FALSE, 3, x, y, NULL, s);
  double minX = 0, minY = 0, maxX = 0, maxY = 0;
  for (int i = 0; i < 3; ++i) {
    minX = minX < x[i] ? minX : x[i];
    minY = minY < y[i] ? minY : y[i];
    maxX = maxX > x[i] ? maxX : x[i];
    maxY = maxY > y[i] ? maxY : y[i];
  }
  int newXSize = int(maxX - minX) + 1;
  int newYSize = int(maxY - minY) + 1;
  xOffset = -minX;
  yOffset = -minY;
  tParam.x0 = xOffset;
  tParam.y0 = yOffset;
  psWarpOptions->pTransformerArg = &tParam;

  // create the output file
  GDALDriverH driver = GDALGetDriverByName("GTiff");
  char **papszOptions = NULL;
  papszOptions = CSLSetNameValue(papszOptions, "INIT_DEST", "NO_DATA");
  papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", compression);
  GDALDatasetH hDstDS = 
      GDALCreate(driver,
                 QFile::encodeName(output).constData(), newXSize, newYSize, 
                 GDALGetRasterCount(hSrcDS),
                 GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1)),
                 papszOptions );
 
  for( int i = 0; i < GDALGetRasterCount(hSrcDS); ++i )
  {
    GDALRasterBandH hSrcBand = GDALGetRasterBand(hSrcDS,i+1);
    GDALRasterBandH hDstBand = GDALGetRasterBand(hDstDS,i+1);
    GDALColorTableH cTable = GDALGetRasterColorTable(hSrcDS);
    if (cTable)
    {
      GDALSetRasterColorTable(hDstDS,cTable);
    }

    double noData = GDALGetRasterNoDataValue(hSrcBand,NULL);
    if (noData == -1e10 && useZeroAsTrans) 
    {
        GDALSetRasterNoDataValue(hDstBand,0);
    }
    else
    {
        GDALSetRasterNoDataValue(hDstBand,noData);
    }
  }
  psWarpOptions->hDstDS = hDstDS;

  // Initialize and execute the warp operation. 
  GDALWarpOperation oOperation;
  oOperation.Initialize(psWarpOptions);
  oOperation.ChunkAndWarpImage(0, 0, GDALGetRasterXSize(hDstDS), 
			       GDALGetRasterYSize(hDstDS));
  GDALDestroyWarpOptions(psWarpOptions);

  GDALClose( hSrcDS );
  GDALClose( hDstDS );
}


int QgsImageWarper::transform(void *pTransformerArg, int bDstToSrc, 
			      int nPointCount, double *x, double *y, 
			      double *z, int *panSuccess) {
  TransformParameters* t = static_cast<TransformParameters*>(pTransformerArg);
  double a = cos(t->angle), b = sin(t->angle), x0 = t->x0, y0 = t->y0;
  for (int i = 0; i < nPointCount; ++i) {
    double xT = x[i], yT = y[i];
    if (bDstToSrc == FALSE) {
      x[i] = x0 + a * xT - b * yT;
      y[i] = y0 + b * xT + a * yT;
    }
    else {
      x[i] = (a * (xT - x0) + b * (yT - y0)) * 1 / (pow(a, 2) + pow(b, 2));
      y[i] = (-b* (xT - x0) + a * (yT - y0)) * 1 / (pow(a, 2) + pow(b, 2));
    }
    panSuccess[i] = TRUE;
  }
  return TRUE;
}
