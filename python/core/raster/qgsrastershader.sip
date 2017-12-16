/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastershader.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterShader
{
%Docstring
Interface for all raster shaders.
%End

%TypeHeaderCode
#include "qgsrastershader.h"
%End
  public:
    QgsRasterShader( double minimumValue = 0.0, double maximumValue = 255.0 );


    double maximumValue();
%Docstring
 Return the maximum value for the raster shader
%End

    double minimumValue();
%Docstring
 Return the minimum value for the raster shader
%End

    QgsRasterShaderFunction *rasterShaderFunction();

    bool shade( double value,
                int *returnRedValue /Out/,
                int *returnGreenValue /Out/,
                int *returnBlueValue /Out/,
                int *returnAlpha /Out/ );
%Docstring
 generates and new RGBA value based on one input value
%End

    bool shade( double redValue,
                double greenValue,
                double blueValue,
                double alphaValue,
                int *returnRedValue /Out/,
                int *returnGreenValue /Out/,
                int *returnBlueValue /Out/,
                int *returnAlpha /Out/ );
%Docstring
 generates and new RGBA value based on original RGBA value
%End

    void setRasterShaderFunction( QgsRasterShaderFunction *function /Transfer/ );
%Docstring
 A public method that allows the user to set their own shader function

.. note::

   Raster shader takes ownership of the shader function instance *
%End

    void setMaximumValue( double );
%Docstring
 Set the maximum value
%End

    void setMinimumValue( double );
%Docstring
 Return the minimum value
%End

    void writeXml( QDomDocument &doc, QDomElement &parent ) const;
%Docstring
Writes shader state to an XML element.
%End

    void readXml( const QDomElement &elem );
%Docstring
Reads shader state from an XML element.
%End

  private:
    QgsRasterShader( const QgsRasterShader &rh );
};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastershader.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
