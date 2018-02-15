/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscolorrampshader.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsColorRampShader : QgsRasterShaderFunction
{
%Docstring
A ramp shader will color a raster pixel based on a list of values ranges in a ramp.
%End

%TypeHeaderCode
#include "qgscolorrampshader.h"
%End
  public:

    enum Type
    {
      Interpolated,
      Discrete,
      Exact
    };

    enum ClassificationMode
    {
      Continuous,
      EqualInterval,
      Quantile
    };

    QgsColorRampShader( double minimumValue = 0.0, double maximumValue = 255.0, QgsColorRamp *colorRamp = 0, Type type = Interpolated, ClassificationMode classificationMode = Continuous );
%Docstring
Creates a new color ramp shader.

:param minimumValue: minimum value for the raster shader
:param maximumValue: maximum value for the raster shader
:param type: interpolation type used
:param classificationMode: method used to classify the color ramp shader
:param colorRamp: vector color ramp used to classify the color ramp shader

:return: new QgsColorRampShader
%End

    QgsColorRampShader( const QgsColorRampShader &other );
%Docstring
Copy constructor
%End


    struct ColorRampItem
    {
      ColorRampItem();
%Docstring
default constructor
%End
      ColorRampItem( double val, const QColor &col, const QString &lbl = QString() );
%Docstring
convenience constructor
%End

      QString label;
      double value;
      QColor color;

      bool operator<( const QgsColorRampShader::ColorRampItem &other ) const;
    };

    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const;
%Docstring
 Get the custom colormap
%End

    Type colorRampType() const;
%Docstring
 Get the color ramp type
%End

    QString colorRampTypeAsQString();
%Docstring
 Get the color ramp type as a string
%End

    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem> &list ); //TODO: sort on set

    void setColorRampType( QgsColorRampShader::Type colorRampType );
%Docstring
 Set the color ramp type
%End

    QgsColorRamp *sourceColorRamp() const /Factory/;
%Docstring
Get the source color ramp

.. versionadded:: 3.0

.. seealso:: :py:func:`setSourceColorRamp`
%End

    void setSourceColorRamp( QgsColorRamp *colorramp /Transfer/ );
%Docstring
Set the source color ramp. Ownership is transferred to the renderer.

.. versionadded:: 3.0

.. seealso:: :py:func:`sourceColorRamp`
%End

    void setColorRampType( const QString &type );
%Docstring
 Set the color ramp type
%End

    void classifyColorRamp( const int classes = 0, const int band = -1, const QgsRectangle &extent = QgsRectangle(), QgsRasterInterface *input = 0 );
%Docstring
Classify color ramp shader

:param classes: number of classes
:param band: raster band used in classification (only used in quantile mode)
:param extent: extent used in classification (only used in quantile mode)
:param input: raster input used in classification (only used in quantile mode)
%End

    void classifyColorRamp( const int band = -1, const QgsRectangle &extent = QgsRectangle(), QgsRasterInterface *input = 0 ) /PyName=classifyColorRampV2/;
%Docstring
Classify color ramp shader

:param band: raster band used in classification (quantile mode only)
:param extent: extent used in classification (quantile mode only)
:param input: raster input used in classification (quantile mode only)
%End

    virtual bool shade( double value, int *returnRedValue /Out/, int *returnGreenValue /Out/, int *returnBlueValue /Out/, int *returnAlphaValue /Out/ );

%Docstring
 Generates and new RGB value based on one input value
%End

     virtual bool shade( double redValue, double greenValue,
                double blueValue, double alphaValue,
                int *returnRedValue /Out/, int *returnGreenValue /Out/,
                int *returnBlueValue /Out/, int *returnAlphaValue /Out/ );
%Docstring
 Generates and new RGB value based on original RGB value
%End

    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;

%Docstring
 Get symbology items if provided by renderer
%End

    void setClassificationMode( ClassificationMode classificationMode );
%Docstring
Sets classification mode
%End

    ClassificationMode classificationMode() const;
%Docstring
Returns the classification mode
%End

    void setClip( bool clip );
%Docstring
Sets whether the shader should not render values out of range.

:param clip: set to true to clip values which are out of range.

.. seealso:: :py:func:`clip`
%End

    bool clip() const;
%Docstring
Returns whether the shader will clip values which are out of range.

.. seealso:: :py:func:`setClip`
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscolorrampshader.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
