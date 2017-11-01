/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastershaderfunction.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterShaderFunction
{
%Docstring
 The raster shade function applies a shader to a pixel at render time -
 typically used to render grayscale images as false color.
%End

%TypeHeaderCode
#include "qgsrastershaderfunction.h"
#include <qgscolorrampshader.h>
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsColorRampShader *>( sipCpp ) != NULL )
      sipType = sipType_QgsColorRampShader;
    else
      sipType = 0;
%End
  public:
    QgsRasterShaderFunction( double minimumValue = 0.0, double maximumValue = 255.0 );
    virtual ~QgsRasterShaderFunction();

    virtual void setMaximumValue( double );
%Docstring
 Set the maximum value
%End

    virtual void setMinimumValue( double );
%Docstring
 Return the minimum value
%End

    virtual bool shade( double value,
                        int *returnRedValue /Out/,
                        int *returnGreenValue /Out/,
                        int *returnBlueValue /Out/,
                        int *returnAlpha /Out/ );
%Docstring
 generates and new RGBA value based on one input value
 :rtype: bool
%End

    virtual bool shade( double redValue,
                        double greenValue,
                        double blueValue,
                        double alphaValue,
                        int *returnRedValue /Out/,
                        int *returnGreenValue /Out/,
                        int *returnBlueValue /Out/,
                        int *returnAlpha /Out/ );
%Docstring
 generates and new RGBA value based on original RGBA value
 :rtype: bool
%End

    double minimumMaximumRange() const;
%Docstring
 :rtype: float
%End

    double minimumValue() const;
%Docstring
 :rtype: float
%End
    double maximumValue() const;
%Docstring
 :rtype: float
%End

    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;

  protected:


};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastershaderfunction.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
