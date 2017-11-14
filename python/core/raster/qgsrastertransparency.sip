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
 Accessor for transparentSingleValuePixelList
 :rtype: list of QgsRasterTransparency.TransparentSingleValuePixel
%End

    QList<QgsRasterTransparency::TransparentThreeValuePixel> transparentThreeValuePixelList() const;
%Docstring
 Accessor for transparentThreeValuePixelList
 :rtype: list of QgsRasterTransparency.TransparentThreeValuePixel
%End

    void initializeTransparentPixelList( double );
%Docstring
 Reset to the transparency list to a single value
%End

    void initializeTransparentPixelList( double, double, double );
%Docstring
 Reset to the transparency list to a single value
%End

    void setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel> &newList /Transfer/ );
%Docstring
 Mutator for transparentSingleValuePixelList
%End

    void setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel> &newList /Transfer/ );
%Docstring
 Mutator for transparentThreeValuePixelList
%End

    int alphaValue( double, int globalTransparency = 255 ) const;
%Docstring
 Returns the transparency value for a single value Pixel
 :rtype: int
%End

    int alphaValue( double, double, double, int globalTransparency = 255 ) const;
%Docstring
 Return the transparency value for a RGB Pixel
 :rtype: int
%End

    bool isEmpty() const;
%Docstring
True if there are no entries in the pixel lists except the nodata value
 :rtype: bool
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
