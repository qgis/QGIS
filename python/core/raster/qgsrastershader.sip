
class QgsRasterShader
{
%TypeHeaderCode
#include <qgsrastershader.h>
#include <qgsrastershaderfunction.h>
%End

  public:
    QgsRasterShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );
    ~QgsRasterShader();

    /*
     *
     * Non-Static Inline methods
     *
     */
    /** \brief Return the maximum value for the raster shader */
    double maximumValue();

    /** \brief Return the minimum value for the raster shader */
    double minimumValue();

    QgsRasterShaderFunction* rasterShaderFunction();

    /*
     *
     * Non-Static methods
     *
     */
    /** \brief generates and new RGB value based on one input value */
    bool shade( double, int* /Out/, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief generates and new RGB value based on original RGB value */
    bool shade( double, double, double, double, int* /Out/, int* /Out/, int* /Out/, int* /Out/ );

    /** \brief A public method that allows the user to set their own shader function
      \note Raster shader takes ownership of the shader function instance */
    void setRasterShaderFunction( QgsRasterShaderFunction* /Transfer/ );

    /** \brief Set the maximum value */
    void setMaximumValue( double );

    /** \brief Return the minimum value */
    void setMinimumValue( double );

    void writeXML( QDomDocument& doc, QDomElement& parent ) const;

    void readXML( const QDomElement& elem );
};
