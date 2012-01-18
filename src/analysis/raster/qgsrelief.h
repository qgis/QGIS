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

#include <QColor>
#include <QMap>
#include <QPair>
#include <QString>
#include "gdal.h"

class QgsAspectFilter;
class QgsSlopeFilter;
class QgsHillshadeFilter;
class QProgressDialog;

/**Produces coloured relief rasters from DEM*/
class ANALYSIS_EXPORT QgsRelief
{
  public:
    struct ReliefColor
    {
      ReliefColor( const QColor& c, double min, double max ): color( c ), minElevation( min ), maxElevation( max ) { }
      QColor color;
      double minElevation;
      double maxElevation;
    };

    QgsRelief( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    ~QgsRelief();

    /**Starts the calculation, reads from mInputFile and stores the result in mOutputFile
      @param p progress dialog that receives update and that is checked for abort. 0 if no progress bar is needed.
      @return 0 in case of success*/
    int processRaster( QProgressDialog* p );

    double zFactor() const { return mZFactor; }
    void setZFactor( double factor ) { mZFactor = factor; }

    void clearReliefColors();
    void addReliefColorClass( const ReliefColor& color );
    const QList< ReliefColor >& reliefColors() const { return mReliefColors; }
    void setReliefColors( const QList< ReliefColor >& c ) { mReliefColors = c; }

    /**Calculates class breaks according with the method of Buenzli (2011) using an iterative algorithm for segmented regression
      @return true in case of success*/
    QList< ReliefColor > calculateOptimizedReliefClasses();

    /**Write frequency of elevation values to file for manual inspection*/
    bool exportFrequencyDistributionToCsv( const QString& file );

  private:

    QString mInputFile;
    QString mOutputFile;
    QString mOutputFormat;

    double mCellSizeX;
    double mCellSizeY;
    /**The nodata value of the input layer*/
    float mInputNodataValue;
    /**The nodata value of the output layer*/
    float mOutputNodataValue;

    double mZFactor;

    QgsSlopeFilter* mSlopeFilter;
    QgsAspectFilter* mAspectFilter;
    QgsHillshadeFilter* mHillshadeFilter285;
    QgsHillshadeFilter* mHillshadeFilter300;
    QgsHillshadeFilter* mHillshadeFilter315;

    //relief colors and corresponding elevations
    QList< ReliefColor > mReliefColors;

    bool processNineCellWindow( float* x1, float* x2, float* x3, float* x4, float* x5, float* x6, float* x7, float* x8, float* x9,
                                char* red, char* green, char* blue );

    /**Opens the input file and returns the dataset handle and the number of pixels in x-/y- direction*/
    GDALDatasetH openInputFile( int& nCellsX, int& nCellsY );
    /**Opens the output driver and tests if it supports the creation of a new dataset
      @return NULL on error and the driver handle on success*/
    GDALDriverH openOutputDriver();
    /**Opens the output file and sets the same geotransform and CRS as the input data
      @return the output dataset or NULL in case of error*/
    GDALDatasetH openOutputFile( GDALDatasetH inputDataset, GDALDriverH outputDriver );

    /**Set elevation color*/
    bool setElevationColor( double elevation, int* red, int* green, int* blue );

    /**Sets relief colors*/
    void setDefaultReliefColors();
    /**Returns class (0-255) for an elevation value
      @return elevation class or -1 in case of error*/
    int frequencyClassForElevation( double elevation, double minElevation, double elevationClassRange );
    /**Do one iteration of class break optimisation (algorithm from Garcia and Rodriguez)*/
    void optimiseClassBreaks( QList<int>& breaks, double* frequencies );
    /**Calculates coefficients a (slope) and b (y value for x=0)
      @param input data points ( elevation class / frequency )*/
    bool calculateRegression( const QList< QPair < int, double > >& input, double& a, double& b );
};

#endif // QGSRELIEF_H
