/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterinterface.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterBlockFeedback : QgsFeedback
{
%Docstring
Feedback object tailored for raster block reading.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsrasterinterface.h"
%End
  public:
    QgsRasterBlockFeedback( QObject *parent = 0 );
%Docstring
Construct a new raster block feedback object
%End

    virtual void onNewData();
%Docstring
May be emitted by raster data provider to indicate that some partial data are available
and a new preview image may be produced
%End

    bool isPreviewOnly() const;
%Docstring
Whether the raster provider should return only data that are already available
without waiting for full result. By default this flag is not enabled.

.. seealso:: :py:func:`setPreviewOnly`
%End

    void setPreviewOnly( bool preview );
%Docstring
set flag whether the block request is for preview purposes only

.. seealso:: :py:func:`isPreviewOnly`
%End

    bool renderPartialOutput() const;
%Docstring
Whether our painter is drawing to a temporary image used just by this layer

.. seealso:: :py:func:`setRenderPartialOutput`
%End

    void setRenderPartialOutput( bool enable );
%Docstring
Set whether our painter is drawing to a temporary image used just by this layer

.. seealso:: :py:func:`renderPartialOutput`
%End

};


class QgsRasterInterface
{
%Docstring
Base class for processing filters like renderers, reprojector, resampler etc.
%End

%TypeHeaderCode
#include "qgsrasterinterface.h"
// QgsRasterInterface subclasses
#include <qgsbrightnesscontrastfilter.h>
#include <qgshuesaturationfilter.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasternuller.h>
#include <qgsrasterprojector.h>
#include <qgsrasterrenderer.h>
#include <qgsrasterresamplefilter.h>

// QgsRasterRenderer subclasses
#include <qgshillshaderenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgspalettedrasterrenderer.h>
#include <qgssinglebandcolordatarenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsBrightnessContrastFilter *>( sipCpp ) )
      sipType = sipType_QgsBrightnessContrastFilter;
    else if ( dynamic_cast<QgsHueSaturationFilter *>( sipCpp ) )
      sipType = sipType_QgsHueSaturationFilter;
    else if ( dynamic_cast<QgsRasterDataProvider *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterDataProvider;
      // use static cast because QgsRasterDataProvider has multiple inheritance
      // and we would end up with bad pointer otherwise!
      *sipCppRet = static_cast<QgsRasterDataProvider *>( sipCpp );
    }
    else if ( dynamic_cast<QgsRasterNuller *>( sipCpp ) )
      sipType = sipType_QgsRasterNuller;
    else if ( dynamic_cast<QgsRasterProjector *>( sipCpp ) )
      sipType = sipType_QgsRasterProjector;
    else if ( dynamic_cast<QgsRasterRenderer *>( sipCpp ) )
    {
      if ( dynamic_cast<QgsHillshadeRenderer *>( sipCpp ) )
        sipType = sipType_QgsHillshadeRenderer;
      else if ( dynamic_cast<QgsMultiBandColorRenderer *>( sipCpp ) )
        sipType = sipType_QgsMultiBandColorRenderer;
      else if ( dynamic_cast<QgsPalettedRasterRenderer *>( sipCpp ) )
        sipType = sipType_QgsPalettedRasterRenderer;
      else if ( dynamic_cast<QgsSingleBandColorDataRenderer *>( sipCpp ) )
        sipType = sipType_QgsSingleBandColorDataRenderer;
      else if ( dynamic_cast<QgsSingleBandGrayRenderer *>( sipCpp ) )
        sipType = sipType_QgsSingleBandGrayRenderer;
      else if ( dynamic_cast<QgsSingleBandPseudoColorRenderer *>( sipCpp ) )
        sipType = sipType_QgsSingleBandPseudoColorRenderer;
      else
        sipType = sipType_QgsRasterRenderer;
    }
    else if ( dynamic_cast<QgsRasterResampleFilter *>( sipCpp ) )
      sipType = sipType_QgsRasterResampleFilter;
    else
      sipType = 0;
%End
  public:
    enum Capability
    {
      NoCapabilities,
      Size,
      Create,
      Remove,
      BuildPyramids,
      Identify,
      IdentifyValue,
      IdentifyText,
      IdentifyHtml,
      IdentifyFeature,
    };

    QgsRasterInterface( QgsRasterInterface *input = 0 );

    virtual ~QgsRasterInterface();

    virtual QgsRasterInterface *clone() const = 0 /Factory/;
%Docstring
Clone itself, create deep copy
%End

    virtual int capabilities() const;
%Docstring
Returns a bitmask containing the supported capabilities
%End

    QString capabilitiesString() const;
%Docstring
Returns the above in friendly format.
%End

    virtual Qgis::DataType dataType( int bandNo ) const = 0;
%Docstring
Returns data type for the band specified by number
%End

    virtual Qgis::DataType sourceDataType( int bandNo ) const;
%Docstring
Returns source data type for the band specified by number,
source data type may be shorter than dataType *
%End

    virtual QgsRectangle extent() const;
%Docstring
Get the extent of the interface.

:return: QgsRectangle containing the extent of the layer
%End

    int dataTypeSize( int bandNo );

    virtual int bandCount() const = 0;
%Docstring
Get number of bands
%End

    virtual int xBlockSize() const;
%Docstring
Get block size
%End
    virtual int yBlockSize() const;

    virtual int xSize() const;
%Docstring
Get raster size
%End
    virtual int ySize() const;

    virtual QString generateBandName( int bandNumber ) const;
%Docstring
 helper function to create zero padded band names
%End

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) = 0 /Factory/;
%Docstring
Read block of data using given extent and size.
Returns pointer to data.
Caller is responsible to free the memory returned.

:param bandNo: band number
:param extent: extent of block
:param width: pixel width of block
:param height: pixel height of block
:param feedback: optional raster feedback object for cancelation/preview. Added in QGIS 3.0.
%End

    virtual bool setInput( QgsRasterInterface *input );
%Docstring
Set input.
Returns true if set correctly, false if cannot use that input *
%End

    virtual QgsRasterInterface *input() const;
%Docstring
Current input
%End

    virtual bool on() const;
%Docstring
Is on/off
%End

    virtual void setOn( bool on );
%Docstring
Set on/off
%End


    virtual QgsRasterInterface *sourceInput();
%Docstring
Get source / raw input, the first in pipe, usually provider.
It may be used to get info about original data, e.g. resolution to decide
resampling etc.
%End

    virtual QgsRasterBandStats bandStatistics( int bandNo,
        int stats = QgsRasterBandStats::All,
        const QgsRectangle &extent = QgsRectangle(),
        int sampleSize = 0, QgsRasterBlockFeedback *feedback = 0 );
%Docstring
 Get band statistics.

:param bandNo: The band (number).
:param stats: Requested statistics
:param extent: Extent used to calc statistics, if empty, whole raster extent is used.
:param sampleSize: Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
:param feedback: optional feedback object

:return: Band statistics.
%End

    virtual bool hasStatistics( int bandNo,
                                int stats = QgsRasterBandStats::All,
                                const QgsRectangle &extent = QgsRectangle(),
                                int sampleSize = 0 );
%Docstring
 Returns true if histogram is available (cached, already calculated).     *   The parameters are the same as in bandStatistics()

:return: true if statistics are available (ready to use)
%End


    virtual QgsRasterHistogram histogram( int bandNo,
                                          int binCount = 0,
                                          SIP_PYOBJECT minimum = Py_None,
                                          SIP_PYOBJECT maximum = Py_None,
                                          const QgsRectangle &extent = QgsRectangle(),
                                          int sampleSize = 0,
                                          bool includeOutOfRange = false,
                                          QgsRasterBlockFeedback *feedback = 0 )
    [QgsRasterHistogram( int bandNo,
                         int binCount = 0,
                         double minimum = 0.0,
                         double maximum = 0.0,
                         const QgsRectangle &extent = QgsRectangle(),
                         int sampleSize = 0,
                         bool includeOutOfRange = false,
                         QgsRasterBlockFeedback *feedback = nullptr )];
%Docstring
 Get histogram. Histograms are cached in providers.

:param bandNo: The band (number).
:param binCount: Number of bins (intervals,buckets). If 0, the number of bins is decided automatically according to data type, raster size etc.
:param minimum: Minimum value, if NaN (None for Python), raster minimum value will be used.
:param maximum: Maximum value, if NaN (None for Python), raster maximum value will be used.
:param extent: Extent used to calc histogram, if empty, whole raster extent is used.
:param sampleSize: Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
:param includeOutOfRange: include out of range values
:param feedback: optional feedback object

:return: Vector of non NULL cell counts for each bin.

.. note::

   binCount, minimum and maximum not optional in Python bindings
%End
%MethodCode
    double minimum;
    double maximum;
    if ( a2 == Py_None )
    {
      minimum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      minimum = PyFloat_AsDouble( a2 );
    }

    if ( a3 == Py_None )
    {
      maximum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      maximum = PyFloat_AsDouble( a3 );
    }

    QgsRasterHistogram h = sipCpp->histogram( a0, a1, minimum, maximum, *a4, a5, a6, a7 );
    sipRes = &h;
%End


    virtual bool hasHistogram( int bandNo,
                               int binCount,
                               SIP_PYOBJECT minimum = Py_None,
                               SIP_PYOBJECT maximum = Py_None,
                               const QgsRectangle &extent = QgsRectangle(),
                               int sampleSize = 0,
                               bool includeOutOfRange = false )
    [bool( int bandNo,
           int binCount,
           double minimum = 0.0,
           double maximum = 0.0,
           const QgsRectangle &extent = QgsRectangle(),
           int sampleSize = 0,
           bool includeOutOfRange = false )];
%Docstring
 Returns true if histogram is available (cached, already calculated)

.. note::

   the parameters are the same as in :py:func:`histogram`
%End
%MethodCode
    double minimum;
    double maximum;
    if ( a2 == Py_None )
    {
      minimum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      minimum = PyFloat_AsDouble( a2 );
    }

    if ( a3 == Py_None )
    {
      maximum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      maximum = PyFloat_AsDouble( a3 );
    }

    sipRes = sipCpp->hasHistogram( a0, a1, minimum, maximum, *a4, a5, a6 );
%End


    virtual void cumulativeCut( int bandNo,
                                double lowerCount,
                                double upperCount,
                                double &lowerValue,
                                double &upperValue,
                                const QgsRectangle &extent = QgsRectangle(),
                                int sampleSize = 0 );
%Docstring
 Find values for cumulative pixel count cut.

:param bandNo: The band (number).
:param lowerCount: The lower count as fraction of 1, e.g. 0.02 = 2%
:param upperCount: The upper count as fraction of 1, e.g. 0.98 = 98%
:param lowerValue: Location into which the lower value will be set.
:param upperValue:  Location into which the upper value will be set.
:param extent: Extent used to calc histogram, if empty, whole raster extent is used.
:param sampleSize: Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;
%Docstring
Write base class members to xml.
%End
    virtual void readXml( const QDomElement &filterElem );
%Docstring
Sets base class members from xml. Usually called from create() methods of subclasses
%End

  protected:




    void initHistogram( QgsRasterHistogram &histogram,
                        int bandNo,
                        int binCount,
                        SIP_PYOBJECT minimum = Py_None,
                        SIP_PYOBJECT maximum = Py_None,
                        const QgsRectangle &boundingBox = QgsRectangle(),
                        int sampleSize = 0,
                        bool includeOutOfRange = false )
    [void ( QgsRasterHistogram & histogram,
            int bandNo,
            int binCount,
            double minimum = 0.0,
            double maximum = 0.0,
            const QgsRectangle &boundingBox = QgsRectangle(),
            int sampleSize = 0,
            bool includeOutOfRange = false )];
%Docstring
Fill in histogram defaults if not specified

.. note::

   the parameters are the same as in :py:func:`histogram`
%End
%MethodCode
    double minimum;
    double maximum;
    if ( a3 == Py_None )
    {
      minimum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      minimum = PyFloat_AsDouble( a3 );
    }

    if ( a4 == Py_None )
    {
      maximum = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      maximum = PyFloat_AsDouble( a4 );
    }

#if defined(SIP_PROTECTED_IS_PUBLIC)
    sipCpp->initHistogram( *a0, a1, a2, minimum, maximum, *a5, a6, a7 );
#else
    sipCpp->sipProtect_initHistogram( *a0, a1, a2, minimum, maximum, *a5, a6, a7 );
#endif
%End



    void initStatistics( QgsRasterBandStats &statistics, int bandNo,
                         int stats = QgsRasterBandStats::All,
                         const QgsRectangle &boundingBox = QgsRectangle(),
                         int binCount = 0 );
%Docstring
Fill in statistics defaults if not specified
%End

  private:
    QgsRasterInterface( const QgsRasterInterface & );
    QgsRasterInterface &operator=( const QgsRasterInterface & );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterinterface.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
