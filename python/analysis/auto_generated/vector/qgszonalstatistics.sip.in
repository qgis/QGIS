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
A class that calculates raster statistics (count, sum, mean) for a polygon or multipolygon layer and appends the results as attributes.
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
Convenience constructor for QgsZonalStatistics, using an input raster layer.

The raster layer must exist for the lifetime of the zonal statistics calculation.

.. warning::

   Constructing QgsZonalStatistics using this method is not thread safe, and
   the constructor which accepts a QgsRasterInterface should be used instead.
%End

    QgsZonalStatistics( QgsVectorLayer *polygonLayer,
                        QgsRasterInterface *rasterInterface,
                        const QgsCoordinateReferenceSystem &rasterCrs,
                        double rasterUnitsPerPixelX,
                        double rasterUnitsPerPixelY,
                        const QString &attributePrefix = QString(),
                        int rasterBand = 1,
                        QgsZonalStatistics::Statistics stats = QgsZonalStatistics::Statistics( QgsZonalStatistics::Count | QgsZonalStatistics::Sum | QgsZonalStatistics::Mean ) );
%Docstring
Constructor for QgsZonalStatistics, using a QgsRasterInterface.

The ``polygonLayer`` gives the vector layer containing the (multi)polygon features corresponding to the
different zones. This layer will be modified, adding extra attributes for each of the zonal statistics
calculated.

Pixel values for each zone are taken from the raster ``rasterInterface``. The constructor must also
be given various properties relating to the input raster, such as the raster CRS (``rasterCrs``),
and the size (X and Y) in map units for each raster pixel. The source raster band is specified
via ``rasterBand``, where a value of 1 corresponds to the first band.

If the CRS of the ``polygonLayer`` and ``rasterCrs`` differ, the calculation will automatically
reproject the zones to ensure valid results are calculated.

The ``attributePrefix`` argument specifies an optional prefix to use when creating the
new fields for each calculated statistic.

Finally, the calculated statistics can be set via the ``stats`` argument. A new field will be
added to ``polygonLayer`` for each statistic calculated.

.. warning::

   The raster interface must exist for the lifetime of the zonal statistics calculation. For thread
   safe use, always use a cloned raster interface.

.. versionadded:: 3.2
%End

    int calculateStatistics( QgsFeedback *feedback );
%Docstring
Starts the calculation

:return: 0 in case of success
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
