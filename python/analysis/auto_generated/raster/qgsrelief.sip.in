/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrelief.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRelief
{
%Docstring
Produces colored relief rasters from DEM*
%End

%TypeHeaderCode
#include "qgsrelief.h"
%End
  public:
    struct ReliefColor
    {
      ReliefColor( const QColor &c, double min, double max );
      QColor color;
      double minElevation;
      double maxElevation;
    };

    QgsRelief( const QString &inputFile, const QString &outputFile, const QString &outputFormat );
    ~QgsRelief();


    int processRaster( QgsFeedback *feedback = 0 );
%Docstring
Starts the calculation, reads from mInputFile and stores the result in mOutputFile

:param feedback: feedback object that receives update and that is checked for cancellation.

:return: 0 in case of success*
%End

    double zFactor() const;
    void setZFactor( double factor );

    void clearReliefColors();
    void addReliefColorClass( const QgsRelief::ReliefColor &color );
    QList< QgsRelief::ReliefColor > reliefColors() const;
    void setReliefColors( const QList< QgsRelief::ReliefColor > &c );

    QList< QgsRelief::ReliefColor > calculateOptimizedReliefClasses();
%Docstring
Calculates class breaks according with the method of Buenzli (2011) using an iterative algorithm for segmented regression

:return: true in case of success*
%End

    bool exportFrequencyDistributionToCsv( const QString &file );
%Docstring
Write frequency of elevation values to file for manual inspection
%End

  private:
    QgsRelief( const QgsRelief &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrelief.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
