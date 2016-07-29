class QgsTINInterpolator: QgsInterpolator
{
%TypeHeaderCode
#include <qgstininterpolator.h>
%End
  public:
    //describes the type of interpolation
    enum TIN_INTERPOLATION
    {
      Linear,
      CloughTocher
    };
    QgsTINInterpolator( const QList<QgsInterpolator::LayerData>& inputData, TIN_INTERPOLATION interpolation = Linear, bool showProgressDialog = false );
    ~QgsTINInterpolator();

    /** Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    int interpolatePoint( double x, double y, double& result );

    void setExportTriangulationToFile( bool e );
    void setTriangulationFilePath( const QString& filepath );
};
