class QgsHillshadeFilter: QgsDerivativeFilter
{
%TypeHeaderCode
#include <qgshillshadefilter.h>
%End

  public:
    QgsHillshadeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat, double lightAzimuth = 300,
                        double lightAngle = 40 );
    ~QgsHillshadeFilter();

    /** Calculates output value from nine input values. The input values and the output value can be equal to the
    nodata value if not present or outside of the border. Must be implemented by subclasses*/
    float processNineCellWindow( float* x11, float* x21, float* x31,
                                 float* x12, float* x22, float* x32,
                                 float* x13, float* x23, float* x33 );

    float lightAzimuth() const;
    void setLightAzimuth( float azimuth );
    float lightAngle() const;
    void setLightAngle( float angle );
};
