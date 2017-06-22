/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgstininterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsTINInterpolator: QgsInterpolator
{
%Docstring
  Interpolation in a triangular irregular network*
%End

%TypeHeaderCode
#include "qgstininterpolator.h"
%End
  public:
    enum TINInterpolation
    {
      Linear,
      CloughTocher
    };
    QgsTINInterpolator( const QList<QgsInterpolator::LayerData> &inputData, TINInterpolation interpolation = Linear, bool showProgressDialog = false );
    ~QgsTINInterpolator();

    virtual int interpolatePoint( double x, double y, double &result );

%Docstring
 Calculates interpolation value for map coordinates x, y
\param x x-coordinate (in map units)
\param y y-coordinate (in map units)
\param result out: interpolation result
:return: 0 in case of success*
 :rtype: int
%End

    void setExportTriangulationToFile( bool e );
    void setTriangulationFilePath( const QString &filepath );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/qgstininterpolator.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
