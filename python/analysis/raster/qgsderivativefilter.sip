/** Adds the ability to calculate derivatives in x- and y-directions. Needs to be subclassed (e.g. for slope and aspect)*/
class QgsDerivativeFilter : QgsNineCellFilter
{
%TypeHeaderCode
#include <qgsderivativefilter.h>
%End

  public:
    QgsDerivativeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    virtual ~QgsDerivativeFilter();
    //to be implemented by subclasses
    virtual float processNineCellWindow( float* x11, float* x21, float* x31,
                                         float* x12, float* x22, float* x32,
                                         float* x13, float* x23, float* x33 ) = 0;

  protected:
    /** Calculates the first order derivative in x-direction according to Horn (1981)*/
    float calcFirstDerX( float* x11, float* x21, float* x31, float* x12, float* x22, float* x32, float* x13, float* x23, float* x33 );
    /** Calculates the first order derivative in y-direction according to Horn (1981)*/
    float calcFirstDerY( float* x11, float* x21, float* x31, float* x12, float* x22, float* x32, float* x13, float* x23, float* x33 );
};
