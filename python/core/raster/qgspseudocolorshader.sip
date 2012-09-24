class QgsPseudoColorShader : QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgspseudocolorshader.h>
%End

  public:
    QgsPseudoColorShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    /** \brief generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief generates and new RGB value based on original RGB value */
    bool shade( double, double, double, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief Set the maximum value */
    void setMaximumValue( double );

    /** \brief Return the minimum value */
    void setMinimumValue( double );

    double classBreakMin1() const;
    double classBreakMax1() const;
    double classBreakMin2() const;
    double classBreakMax2() const;
    double classBreakMin3() const;
};
