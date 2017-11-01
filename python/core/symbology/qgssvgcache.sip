/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssvgcache.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsSvgCache : QObject
{
%Docstring
 A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"

 QgsSvgCache is not usually directly created, but rather accessed through
 QgsApplication.svgCache().
%End

%TypeHeaderCode
#include "qgssvgcache.h"
%End
  public:

    QgsSvgCache( QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsSvgCache.
%End

    ~QgsSvgCache();

    QImage svgAsImage( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                       double widthScaleFactor, bool &fitsInCache, double fixedAspectRatio = 0 );
%Docstring
 Get SVG as QImage.
 \param path Absolute path to SVG file.
 \param size size of cached image
 \param fill color of fill
 \param stroke color of stroke
 \param strokeWidth width of stroke
 \param widthScaleFactor width scale factor
 \param fitsInCache
 \param fixedAspectRatio fixed aspect ratio (optional)
 :rtype: QImage
%End

    QPicture svgAsPicture( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, bool forceVectorOutput = false, double fixedAspectRatio = 0 );
%Docstring
 Get SVG  as QPicture&.
 \param path Absolute path to SVG file.
 \param size size of cached image
 \param fill color of fill
 \param stroke color of stroke
 \param strokeWidth width of stroke
 \param widthScaleFactor width scale factor
 \param forceVectorOutput
 \param fixedAspectRatio fixed aspect ratio (optional)
 :rtype: QPicture
%End

    QSizeF svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0 );
%Docstring
 Calculates the viewbox size of a (possibly cached) SVG file.
 \param path Absolute path to SVG file.
 \param size size of cached image
 \param fill color of fill
 \param stroke color of stroke
 \param strokeWidth width of stroke
 \param widthScaleFactor width scale factor
 \param fixedAspectRatio fixed aspect ratio (optional)
 :return: viewbox size set in SVG file
.. versionadded:: 2.14
 :rtype: QSizeF
%End

    void containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor, bool &hasStrokeWidthParam,
                         double &defaultStrokeWidth ) const;
%Docstring
 Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
default values in the svg file, only the first one is considered*
%End

    void containsParams( const QString &path, bool &hasFillParam, bool &hasDefaultFillParam, QColor &defaultFillColor,
                         bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                         bool &hasStrokeParam, bool &hasDefaultStrokeColor, QColor &defaultStrokeColor,
                         bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                         bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const /PyName=containsParamsV3/;
%Docstring
 Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
 default values in the svg file, only the first one is considered.
 \param path path to SVG file
 \param hasFillParam will be true if fill param present in SVG
 \param hasDefaultFillParam will be true if fill param has a default value specified
 \param defaultFillColor will be set to default fill color specified in SVG, if present
 \param hasFillOpacityParam will be true if fill opacity param present in SVG
 \param hasDefaultFillOpacity will be true if fill opacity param has a default value specified
 \param defaultFillOpacity will be set to default fill opacity specified in SVG, if present
 \param hasStrokeParam will be true if stroke param present in SVG
 \param hasDefaultStrokeColor will be true if stroke param has a default value specified
 \param defaultStrokeColor will be set to default stroke color specified in SVG, if present
 \param hasStrokeWidthParam will be true if stroke width param present in SVG
 \param hasDefaultStrokeWidth will be true if stroke width param has a default value specified
 \param defaultStrokeWidth will be set to default stroke width specified in SVG, if present
 \param hasStrokeOpacityParam will be true if stroke opacity param present in SVG
 \param hasDefaultStrokeOpacity will be true if stroke opacity param has a default value specified
 \param defaultStrokeOpacity will be set to default stroke opacity specified in SVG, if present
.. note::

   available in Python bindings as containsParamsV3
.. versionadded:: 2.14
%End

    QByteArray getImageData( const QString &path ) const;
%Docstring
Get image data
 :rtype: QByteArray
%End

    QByteArray svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0 );
%Docstring
Get SVG content
 :rtype: QByteArray
%End

  signals:
    void statusChanged( const QString  &statusQString );
%Docstring
Emit a signal to be caught by qgisapp and display a msg on status bar
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssvgcache.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
