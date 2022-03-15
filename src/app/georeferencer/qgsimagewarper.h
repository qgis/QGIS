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

#include <QCoreApplication>
#include <QString>

#include <gdalwarper.h>
#include <vector>
#include "qgspointxy.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsogrutils.h"
#include "qgstaskmanager.h"

class QgsGeorefTransform;
class QgsFeedback;

class QgsImageWarper
{
    Q_DECLARE_TR_FUNCTIONS( QgsImageWarper )
    Q_GADGET

  public:

    QgsImageWarper();

    enum class ResamplingMethod : int
    {
      NearestNeighbour = GRA_NearestNeighbour,
      Bilinear = GRA_Bilinear,
      Cubic = GRA_Cubic,
      CubicSpline = GRA_CubicSpline,
      Lanczos = GRA_Lanczos
    };
    Q_ENUM( ResamplingMethod )

    //! Task results
    enum class Result
    {
      Success, //!< Warping completed successfully
      Canceled, //!< Task was canceled before completion
      InvalidParameters, //!< Invalid transform parameters
      SourceError, //!< Error reading source
      TransformError, //!< Error creating GDAL transformer
      DestinationCreationError, //!< Error creating destination file
      WarpFailure, //!< Failed warping source
    };
    Q_ENUM( Result )

    /**
     * Warp the file specified by \a input and write the resulting raster to the file \a output.
     * \param input input file name
     * \param output output file name
     * \param georefTransform specifies the warp transformation which should be applied to \a input.
     * \param resampling specifies image resampling algorithm to use.
     * \param useZeroAsTrans specifies whether to mark transparent areas with a value of "zero".
     * \param compression image compression method
     * \param crs output file CRS
     * \param feedback optional feedback object
     * \param destResX The desired horizontal resolution of the output file, in target georeferenced units. A value of zero means automatic selection.
     * \param destResY The desired vertical resolution of the output file, in target georeferenced units. A value of zero means automatic selection.
     */
    Result warpFile( const QString &input,
                     const QString &output,
                     const QgsGeorefTransform &georefTransform,
                     ResamplingMethod resampling,
                     bool useZeroAsTrans,
                     const QString &compression,
                     const QgsCoordinateReferenceSystem &crs,
                     QgsFeedback *feedback,
                     double destResX = 0.0, double destResY = 0.0 );

  private:
    struct TransformChain
    {
      GDALTransformerFunc GDALTransformer;
      void               *GDALTransformerArg = nullptr;
      double              adfGeotransform[6];
      double              adfInvGeotransform[6];
    };

    //! \sa addGeoToPixelTransform
    static int GeoToPixelTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                    double *x, double *y, double *z, int *panSuccess );

    /**
     * \brief Appends a transform from geocoordinates to pixel/line coordinates to the given GDAL transformer.
     *
     * The resulting transform is the functional composition of the given GDAL transformer and the
     * inverse geo transform.
     * \sa destroyGeoToPixelTransform
     * \returns Argument to use with the static GDAL callback \ref GeoToPixelTransform
     */
    void *addGeoToPixelTransform( GDALTransformerFunc GDALTransformer, void *GDALTransformerArg, double *padfGeotransform ) const;
    void destroyGeoToPixelTransform( void *GeoToPixelTransformArg ) const;

    bool openSrcDSAndGetWarpOpt( const QString &input, ResamplingMethod resampling,
                                 const GDALTransformerFunc &pfnTransform, gdal::dataset_unique_ptr &hSrcDS,
                                 gdal::warp_options_unique_ptr &psWarpOptions );

    bool createDestinationDataset( const QString &outputName, GDALDatasetH hSrcDS, gdal::dataset_unique_ptr &hDstDS, uint resX, uint resY,
                                   double *adfGeoTransform, bool useZeroAsTrans, const QString &compression, const QgsCoordinateReferenceSystem &crs );

    //! \brief GDAL progress callback, used to display warping progress via a QProgressDialog
    static int CPL_STDCALL updateWarpProgress( double dfComplete, const char *pszMessage, void *pProgressArg );

    GDALResampleAlg toGDALResampleAlg( ResamplingMethod method ) const;
};


class QgsImageWarperTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsImageWarperTask.
     *
     * \param input input file name
     * \param output output file name
     * \param georefTransform specifies the warp transformation which should be applied to \a input.
     * \param resampling specifies image resampling algorithm to use.
     * \param useZeroAsTrans specifies whether to mark transparent areas with a value of "zero".
     * \param compression image compression method
     * \param crs output file CRS
     * \param destResX The desired horizontal resolution of the output file, in target georeferenced units. A value of zero means automatic selection.
     * \param destResY The desired vertical resolution of the output file, in target georeferenced units. A value of zero means automatic selection.
     */
    QgsImageWarperTask( const QString &input,
                        const QString &output,
                        const QgsGeorefTransform &georefTransform,
                        QgsImageWarper::ResamplingMethod resampling,
                        bool useZeroAsTrans,
                        const QString &compression,
                        const QgsCoordinateReferenceSystem &crs,
                        double destResX = 0.0, double destResY = 0.0 );

    void cancel() override;

    /**
     * Returns the result of running the task.
     */
    QgsImageWarper::Result result() const { return mResult; }

  protected:

    bool run() override;

  private:

    QString mInput;
    QString mOutput;
    std::unique_ptr< QgsGeorefTransform > mTransform;
    QgsImageWarper::ResamplingMethod mResamplingMethod = QgsImageWarper::ResamplingMethod::Bilinear;
    bool mUseZeroAsTrans = false;
    QString mCompression;
    QgsCoordinateReferenceSystem mDestinationCrs;
    double mDestinationResX = 0;
    double mDestinationResY = 0;

    std::unique_ptr< QgsFeedback > mFeedback;

    QgsImageWarper::Result mResult = QgsImageWarper::Result::Success;
    double mLastProgress = 0;
};


#endif
