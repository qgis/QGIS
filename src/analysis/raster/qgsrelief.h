/***************************************************************************
                          qgsrelief.h  -  description
                          ---------------------------
    begin                : November 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELIEF_H
#define QGSRELIEF_H

#include <gdal.h>

#include "qgis_analysis.h"
#include "qgsogrutils.h"
#include "qgsrasterlayerutils.h"

#include <QColor>
#include <QMap>
#include <QPair>
#include <QString>

class QgsAspectFilter;
class QgsSlopeFilter;
class QgsHillshadeFilter;
class QgsFeedback;

/**
 * \ingroup analysis
 * \brief Produces colored relief rasters from DEM.
*/
class ANALYSIS_EXPORT QgsRelief
{
  public:
    QgsRelief( const QString &inputFile, const QString &outputFile, const QString &outputFormat );
    ~QgsRelief();

    QgsRelief( const QgsRelief &rh ) = delete;
    QgsRelief &operator=( const QgsRelief &rh ) = delete;

    /**
     * Starts the calculation, reads from mInputFile and stores the result in mOutputFile
     * \param feedback feedback object that receives update and that is checked for cancellation.
     * \returns 0 in case of success
    */
    int processRaster( QgsFeedback *feedback = nullptr );

    double zFactor() const { return mZFactor; }
    void setZFactor( double factor ) { mZFactor = factor; }

    void clearReliefColors();
    void addReliefColorClass( const QgsRasterReliefColor &color );
    QList<QgsRasterReliefColor> reliefColors() const { return mReliefColors; }
    void setReliefColors( const QList<QgsRasterReliefColor> &c ) { mReliefColors = c; }

    /**
     * Calculates class breaks according with the method of Buenzli (2011) using an iterative algorithm for segmented regression
     * \returns TRUE in case of success
    */
    QList<QgsRasterReliefColor> calculateOptimizedReliefClasses();

    //! Write frequency of elevation values to file for manual inspection
    bool exportFrequencyDistributionToCsv( const QString &file );

  private:
#ifdef SIP_RUN
    QgsRelief( const QgsRelief &rh );
#endif

    static constexpr int RET_OUTPUT_CREATION_FAILED = 3;
    static constexpr int RET_CANCELED = 7;

    QString mInputFile;
    QString mOutputFile;
    QString mOutputFormat;

    double mCellSizeX = 0.0;
    double mCellSizeY = 0.0;
    //! The nodata value of the input layer
    float mInputNodataValue = -1;
    //! The nodata value of the output layer
    float mOutputNodataValue = -1;

    double mZFactor = 1;

    std::unique_ptr<QgsSlopeFilter> mSlopeFilter;
    std::unique_ptr<QgsAspectFilter> mAspectFilter;
    std::unique_ptr<QgsHillshadeFilter> mHillshadeFilter285;
    std::unique_ptr<QgsHillshadeFilter> mHillshadeFilter300;
    std::unique_ptr<QgsHillshadeFilter> mHillshadeFilter315;

    //relief colors and corresponding elevations
    QList<QgsRasterReliefColor> mReliefColors;

    bool processNineCellWindow( float *x1, float *x2, float *x3, float *x4, float *x5, float *x6, float *x7, float *x8, float *x9, unsigned char *red, unsigned char *green, unsigned char *blue );

    /**
     * Retrieves the color corresponding to the specified \a elevation.
     */
    bool getElevationColor( double elevation, int *red, int *green, int *blue ) const;

    //! Sets relief colors
    void setDefaultReliefColors();

    /**
     * Returns class (0-255) for an elevation value
     * \returns elevation class or -1 in case of error
    */
    static int frequencyClassForElevation( double elevation, double minElevation, double elevationClassRange );
};

#endif // QGSRELIEF_H
