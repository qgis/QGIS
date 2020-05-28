/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastertransparency.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRasterTransparency
{
%Docstring
Defines the list of pixel values to be considered as transparent or semi
transparent when rendering rasters.
%End

%TypeHeaderCode
#include "qgsrastertransparency.h"
%End
  public:

    QgsRasterTransparency();
%Docstring
Constructor for QgsRasterTransparency.
%End

    struct TransparentThreeValuePixel
    {
      double red;
      double green;
      double blue;
      double percentTransparent;
    };

    struct TransparentSingleValuePixel
    {
      double min;
      double max;
      double percentTransparent;
    };


    QList<QgsRasterTransparency::TransparentSingleValuePixel> transparentSingleValuePixelList() const;
%Docstring
Returns the transparent single value pixel list.

.. seealso:: :py:func:`setTransparentSingleValuePixelList`
%End

    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;
%Docstring
Returns the transparent three value pixel list.

.. seealso:: :py:func:`setTransparentThreeValuePixelList`
%End

    void initializeTransparentPixelList( double value );
%Docstring
Resets the transparency list to a single ``value``.
%End

    void initializeTransparentPixelList( double redValue, double greenValue, double blueValue );
%Docstring
Resets the transparency list to single red, green, and blue values.
%End

    void setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel> &newList );
%Docstring
Sets the transparent single value pixel list, replacing the whole existing list.

.. seealso:: :py:func:`transparentSingleValuePixelList`
%End

    void setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel> &newList );
%Docstring
Sets the transparent three value pixel list, replacing the whole existing list.

.. seealso:: :py:func:`transparentThreeValuePixelList`
%End

    int alphaValue( double value, int globalTransparency = 255 ) const;
%Docstring
Returns the transparency value for a single ``value`` pixel.

Searches through the transparency list, and if a match is found, the global transparency value is scaled
by the stored transparency value.

:param value: the needle to search for in the transparency hay stack
:param globalTransparency: the overall transparency level for the layer
%End


    int alphaValue( double redValue, double greenValue, double blueValue, int globalTransparency = 255 ) const;
%Docstring
Returns the transparency value for a RGB pixel.

Searches through the transparency list, if a match is found, the global transparency value is scaled
by the stored transparency value.

:param redValue: the red portion of the needle to search for in the transparency hay stack
:param greenValue: the green portion of the needle to search for in the transparency hay stack
:param blueValue: the green portion of the needle to search for in the transparency hay stack
:param globalTransparency: the overall transparency level for the layer
%End

    bool isEmpty() const;
%Docstring
True if there are no entries in the pixel lists except the nodata value
%End

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;
%Docstring
Writes the transparency information to an XML document.
%End

    void readXml( const QDomElement &elem );
%Docstring
Reads the transparency information from an XML document.
%End

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrastertransparency.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
