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

class QgsGeorefTransform;
class QProgressDialog;
class QWidget;

class QgsImageWarper
{
  public:
    QgsImageWarper(QWidget *theParent);

    enum ResamplingMethod
    {
      NearestNeighbour = GRA_NearestNeighbour,
      Bilinear         = GRA_Bilinear,
      Cubic            = GRA_Cubic,
      CubicSpline      = GRA_CubicSpline,
      Lanczos          = GRA_Lanczos
    };

    bool warpFile( const QString& input, const QString& output, const QgsGeorefTransform &georefTransform,
                   ResamplingMethod resampling = Bilinear, bool useZeroAsTrans = true, const QString& compression = "NONE");
  private:
    struct TransformChain {
      GDALTransformerFunc GDALTransformer;
      void *              GDALTransformerArg;
      double              adfGeotransform[6];
      double              adfInvGeotransform[6];
    };

    //! \sa addGeoToPixelTransform
    static int GeoToPixelTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                    double *x, double *y, double *z, int *panSuccess   );

    /**
     * \brief Appends a transform from geocoordinates to pixel/line coordinates to the given GDAL transformer.
     *
     * The resulting transform is the functional composition of the given GDAL transformer and the
     * inverse geo transform. 
     * \sa destroyGeoToPixelTransform
     * \returns Argument to use with the static GDAL callback \ref GeoToPixelTransform
     */
    void *addGeoToPixelTransform(GDALTransformerFunc GDALTransformer, void *GDALTransformerArg, double *padfGeotransform) const;
    void destroyGeoToPixelTransform(void *GeoToPixelTransfomArg) const;

    bool openSrcDSAndGetWarpOpt(const QString &input, const QString &output,
                                const ResamplingMethod &resampling, const GDALTransformerFunc &pfnTransform,
                                GDALDatasetH &hSrcDS, GDALWarpOptions *&psWarpOptions);

    bool createDestinationDataset(const QString &outputName, GDALDatasetH hSrcDS, GDALDatasetH &hDstDS, uint resX, uint resY,       
                                  double *adfGeoTransform, bool useZeroAsTrans, const QString& compression);

    QWidget *mParent;
    void      *createWarpProgressArg(QProgressDialog *progressDialog) const;
    //! \brief GDAL progress callback, used to display warping progress via a QProgressDialog
    static int CPL_STDCALL updateWarpProgress(double dfComplete, const char *pszMessage, void *pProgressArg);
};


#endif
