/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscontrastenhancementfunction.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsContrastEnhancementFunction
{
%Docstring
A contrast enhancement function is the base class for all raster contrast enhancements.

The purpose of a contrast enhancement is to enhanceContrast or clip a pixel value into
a specified bounding range.
%End

%TypeHeaderCode
#include "qgscontrastenhancementfunction.h"
%End
  public:
    QgsContrastEnhancementFunction( Qgis::DataType, double, double );
    QgsContrastEnhancementFunction( const QgsContrastEnhancementFunction &f );
    virtual ~QgsContrastEnhancementFunction();

    virtual int enhance( double value );
%Docstring
A customizable method that takes in a double ``value`` and returns a int between 0 and 255.
%End

    virtual bool isValueInDisplayableRange( double value );
%Docstring
A customizable method to indicate if a pixel's value is within the displayable range.
%End

    void setMaximumValue( double value );
%Docstring
Sets the maximum ``value``.

.. seealso:: :py:func:`maximumValue`

.. seealso:: :py:func:`setMinimumValue`
%End

    void setMinimumValue( double value );
%Docstring
Sets the minimum ``value``.

.. seealso:: :py:func:`minimumValue`

.. seealso:: :py:func:`setMaximumValue`
%End

    double maximumValue() const;
%Docstring
Returns the maximum value.

.. seealso:: :py:func:`setMaximumValue`

.. seealso:: :py:func:`minimumValue`

.. versionadded:: 3.2
%End

    double minimumValue() const;
%Docstring
Returns the minimum value.

.. seealso:: :py:func:`setMinimumValue`

.. seealso:: :py:func:`maximumValue`

.. versionadded:: 3.2
%End

  protected:



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscontrastenhancementfunction.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
