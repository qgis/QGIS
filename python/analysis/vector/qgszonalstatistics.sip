/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgszonalstatistics.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsZonalStatistics
{
%Docstring
  A class that calculates raster statistics (count, sum, mean) for a polygon or multipolygon layer and appends the results as attributes*
%End

%TypeHeaderCode
#include "qgszonalstatistics.h"
%End
  public:

    enum Statistic
    {
      Count,
      Sum,
      Mean,
      Median,
      StDev,
      Min,
      Max,
      Range,
      Minority,
      Majority,
      Variety,
      Variance,
      All
    };
    typedef QFlags<QgsZonalStatistics::Statistic> Statistics;


    QgsZonalStatistics( QgsVectorLayer *polygonLayer,
                        QgsRasterLayer *rasterLayer,
                        const QString &attributePrefix = QString(),
                        int rasterBand = 1,
                        QgsZonalStatistics::Statistics stats = QgsZonalStatistics::Statistics( QgsZonalStatistics::Count | QgsZonalStatistics::Sum | QgsZonalStatistics::Mean ) );
%Docstring
 Constructor for QgsZonalStatistics.
%End

    int calculateStatistics( QgsFeedback *feedback );
%Docstring
 Starts the calculation
:return: 0 in case of success*
 :rtype: int
%End

      public:
};

QFlags<QgsZonalStatistics::Statistic> operator|(QgsZonalStatistics::Statistic f1, QFlags<QgsZonalStatistics::Statistic> f2);



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgszonalstatistics.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
