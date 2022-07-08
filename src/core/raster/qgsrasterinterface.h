/***************************************************************************
    qgsrasterface.h - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERINTERFACE_H
#define QGSRASTERINTERFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <limits>

#include <QCoreApplication> // for tr()
#include <QImage>

#include "qgsfeedback.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterblock.h"
#include "qgsrasterhistogram.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"

/**
 * \ingroup core
 * \brief Feedback object tailored for raster block reading.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRasterBlockFeedback : public QgsFeedback
{
    Q_OBJECT

  public:
    //! Construct a new raster block feedback object
    QgsRasterBlockFeedback( QObject *parent = nullptr ) : QgsFeedback( parent ) {}

    /**
     * May be emitted by raster data provider to indicate that some partial data are available
     * and a new preview image may be produced
     */
    virtual void onNewData() {}

    /**
     * Whether the raster provider should return only data that are already available
     * without waiting for full result. By default this flag is not enabled.
     * \see setPreviewOnly()
     */
    bool isPreviewOnly() const { return mPreviewOnly; }

    /**
     * set flag whether the block request is for preview purposes only
     * \see isPreviewOnly()
     */
    void setPreviewOnly( bool preview ) { mPreviewOnly = preview; }

    /**
     * Whether our painter is drawing to a temporary image used just by this layer
     * \see setRenderPartialOutput()
     */
    bool renderPartialOutput() const { return mRenderPartialOutput; }

    /**
     * Set whether our painter is drawing to a temporary image used just by this layer
     * \see renderPartialOutput()
     */
    void setRenderPartialOutput( bool enable ) { mRenderPartialOutput = enable; }

    /**
     * Appends an error message to the stored list of errors. Should be called
     * whenever an error is encountered while retrieving a raster block.
     *
     * \see errors()
     * \since QGIS 3.8.0
     */
    void appendError( const QString &error ) { mErrors.append( error ); }

    /**
     * Returns a list of any errors encountered while retrieving the raster block.
     *
     * \see appendError()
     * \since QGIS 3.8.0
     */
    QStringList errors() const { return mErrors; }

    /**
     * Returns the render context of the associated block reading
     *
     * \see setRenderContext()
     * \since QGIS 3.24.0
     */
    QgsRenderContext renderContext() const;

    /**
     * Sets the render context of the associated block reading
     *
     * \see renderContext()
     * \since QGIS 3.24.0
     */
    void setRenderContext( const QgsRenderContext &renderContext );

  private:

    /**
     * Whether the raster provider should return only data that are already available
     * without waiting for full result
     */
    bool mPreviewOnly = false;

    //! Whether our painter is drawing to a temporary image used just by this layer
    bool mRenderPartialOutput = false;

    //! List of errors encountered while retrieving block
    QStringList mErrors;

    QgsRenderContext mRenderContext;
};


/**
 * \ingroup core
 * \brief Base class for processing filters like renderers, reprojector, resampler etc.
 */
class CORE_EXPORT QgsRasterInterface
{
#ifdef SIP_RUN
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
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
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
    SIP_END
#endif

    Q_DECLARE_TR_FUNCTIONS( QgsRasterInterface )

  public:
    //! If you add to this, please also add to capabilitiesString()
    enum Capability
    {
      NoCapabilities   = 0,
      Size             = 1 << 1, //!< Original data source size (and thus resolution) is known, it is not always available, for example for WMS
      Create           = 1 << 2, //!< Create new datasets
      Remove           = 1 << 3, //!< Delete datasets
      BuildPyramids    = 1 << 4, //!< Supports building of pyramids (overviews)
      Identify         = 1 << 5, //!< At least one identify format supported
      IdentifyValue    = 1 << 6, //!< Numerical values
      IdentifyText     = 1 << 7, //!< WMS text
      IdentifyHtml     = 1 << 8, //!< WMS HTML
      IdentifyFeature  = 1 << 9, //!< WMS GML -> feature
      Prefetch         = 1 << 10, //!< Allow prefetching of out-of-view images
    };

    QgsRasterInterface( QgsRasterInterface *input = nullptr );

    virtual ~QgsRasterInterface() = default;

    //! Clone itself, create deep copy
    virtual QgsRasterInterface *clone() const = 0 SIP_FACTORY;

    //! Returns a bitmask containing the supported capabilities
    virtual int capabilities() const
    {
      return QgsRasterInterface::NoCapabilities;
    }

    /**
     *  Returns the raster interface capabilities in friendly format.
     */
    QString capabilitiesString() const;

    //! Returns data type for the band specified by number
    virtual Qgis::DataType dataType( int bandNo ) const = 0;

    /**
     * Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType
    */
    virtual Qgis::DataType sourceDataType( int bandNo ) const { return mInput ? mInput->sourceDataType( bandNo ) : Qgis::DataType::UnknownDataType; }

    /**
     * Gets the extent of the interface.
     * \returns QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const { return mInput ? mInput->extent() : QgsRectangle(); }

    /**
     * Returns the size (in bytes) for the data type for the specified band.
     */
    int dataTypeSize( int bandNo ) const { return QgsRasterBlock::typeSize( dataType( bandNo ) ); }

    //! Gets number of bands
    virtual int bandCount() const = 0;

    //! Gets block size
    virtual int xBlockSize() const { return mInput ? mInput->xBlockSize() : 0; }
    virtual int yBlockSize() const { return mInput ? mInput->yBlockSize() : 0; }

    //! Gets raster size
    virtual int xSize() const { return mInput ? mInput->xSize() : 0; }
    virtual int ySize() const { return mInput ? mInput->ySize() : 0; }

    //! \brief helper function to create zero padded band names
    virtual QString generateBandName( int bandNumber ) const;

    /**
     * Returns the name of the color interpretation for the specified \a bandNumber.
     *
     * \since QGIS 3.18
     */
    virtual QString colorInterpretationName( int bandNumber ) const;

    /**
     * Generates a friendly, descriptive name for the specified \a bandNumber.
     *
     * \since QGIS 3.18
     */
    QString displayBandName( int bandNumber ) const;

    /**
     * Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     * \param bandNo band number
     * \param extent extent of block
     * \param width pixel width of block
     * \param height pixel height of block
     * \param feedback optional raster feedback object for cancellation/preview. Added in QGIS 3.0.
     */
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) = 0 SIP_FACTORY;

    /**
     * Set input.
      * Returns TRUE if set correctly, FALSE if cannot use that input
    */
    virtual bool setInput( QgsRasterInterface *input ) { mInput = input; return true; }

    //! Current input
    virtual QgsRasterInterface *input() const { return mInput; }

    //! Returns whether the interface is on or off
    virtual bool on() const { return mOn; }

    //! Sets whether the interface is on or off
    virtual void setOn( bool on ) { mOn = on; }

    /**
     * Gets source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     * \note not available in Python bindings.
     */
    virtual const QgsRasterInterface *sourceInput() const SIP_SKIP
    {
      QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
      return mInput ? mInput->sourceInput() : this;
    }

    /**
     * Gets source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface *sourceInput()
    {
      QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
      return mInput ? mInput->sourceInput() : this;
    }

    /**
     * Returns the band statistics.
     * \param bandNo The band (number).
     * \param stats Requested statistics
     * \param extent Extent used to calc statistics, if empty, whole raster extent is used.
     * \param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * \param feedback optional feedback object
     */
    virtual QgsRasterBandStats bandStatistics( int bandNo,
        int stats = QgsRasterBandStats::All,
        const QgsRectangle &extent = QgsRectangle(),
        int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * \brief Returns TRUE if histogram is available (cached, already calculated).     *   The parameters are the same as in bandStatistics()
     * \returns TRUE if statistics are available (ready to use)
     */
    virtual bool hasStatistics( int bandNo,
                                int stats = QgsRasterBandStats::All,
                                const QgsRectangle &extent = QgsRectangle(),
                                int sampleSize = 0 );


    /**
     * Returns a band histogram. Histograms are cached in providers.
     * \param bandNo The band (number).
     * \param binCount Number of bins (intervals,buckets). If 0, the number of bins is decided automatically according to data type, raster size etc.
     * \param minimum Minimum value, if NaN (None for Python), raster minimum value will be used.
     * \param maximum Maximum value, if NaN (None for Python), raster maximum value will be used.
     * \param extent Extent used to calc histogram, if empty, whole raster extent is used.
     * \param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * \param includeOutOfRange include out of range values
     * \param feedback optional feedback object
     * \returns Vector of non NULL cell counts for each bin.
     * \note binCount, minimum and maximum not optional in Python bindings
     */
#ifndef SIP_RUN
    virtual QgsRasterHistogram histogram( int bandNo,
                                          int binCount = 0,
                                          double minimum = std::numeric_limits<double>::quiet_NaN(),
                                          double maximum = std::numeric_limits<double>::quiet_NaN(),
                                          const QgsRectangle &extent = QgsRectangle(),
                                          int sampleSize = 0,
                                          bool includeOutOfRange = false,
                                          QgsRasterBlockFeedback *feedback = nullptr );
#else
    virtual QgsRasterHistogram histogram( int bandNo,
                                          int binCount = 0,
                                          SIP_PYOBJECT minimum = Py_None,
                                          SIP_PYOBJECT maximum = Py_None,
                                          const QgsRectangle &extent = QgsRectangle(),
                                          int sampleSize = 0,
                                          bool includeOutOfRange = false,
                                          QgsRasterBlockFeedback *feedback = nullptr )
    [QgsRasterHistogram( int bandNo,
                         int binCount = 0,
                         double minimum = 0.0,
                         double maximum = 0.0,
                         const QgsRectangle &extent = QgsRectangle(),
                         int sampleSize = 0,
                         bool includeOutOfRange = false,
                         QgsRasterBlockFeedback *feedback = nullptr )];
    % MethodCode
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

    QgsRasterHistogram *h = new QgsRasterHistogram( sipCpp->histogram( a0, a1, minimum, maximum, *a4, a5, a6, a7 ) );
    return sipConvertFromType( h, sipType_QgsRasterHistogram, Py_None );
    % End
#endif


    /**
     * \brief Returns TRUE if histogram is available (cached, already calculated)
     * \note the parameters are the same as in \see histogram()
     */
#ifndef SIP_RUN
    virtual bool hasHistogram( int bandNo,
                               int binCount,
                               double minimum = std::numeric_limits<double>::quiet_NaN(),
                               double maximum = std::numeric_limits<double>::quiet_NaN(),
                               const QgsRectangle &extent = QgsRectangle(),
                               int sampleSize = 0,
                               bool includeOutOfRange = false );
#else
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
    % MethodCode
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
    % End
#endif


    /**
     * \brief Find values for cumulative pixel count cut.
     * \param bandNo The band (number).
     * \param lowerCount The lower count as fraction of 1, e.g. 0.02 = 2%
     * \param upperCount The upper count as fraction of 1, e.g. 0.98 = 98%
     * \param lowerValue Location into which the lower value will be set.
     * \param upperValue  Location into which the upper value will be set.
     * \param extent Extent used to calc histogram, if empty, whole raster extent is used.
     * \param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     */
    virtual void cumulativeCut( int bandNo,
                                double lowerCount,
                                double upperCount,
                                double &lowerValue,
                                double &upperValue,
                                const QgsRectangle &extent = QgsRectangle(),
                                int sampleSize = 0 );

    //! Write base class members to xml.
    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const { Q_UNUSED( doc ) Q_UNUSED( parentElem ); }
    //! Sets base class members from xml. Usually called from create() methods of subclasses
    virtual void readXml( const QDomElement &filterElem ) { Q_UNUSED( filterElem ) }

  protected:
    // QgsRasterInterface used as input
    QgsRasterInterface *mInput = nullptr;

    //! \brief List  of cached statistics, all bands mixed
    QList<QgsRasterBandStats> mStatistics;

    //! \brief List  of cached histograms, all bands mixed
    QList<QgsRasterHistogram> mHistograms;

    // On/off state, if off, it does not do anything, replicates input
    bool mOn = true;

    /**
     * Fill in histogram defaults if not specified
     * \note the parameters are the same as in \see histogram()
     */
#ifndef SIP_RUN
    void initHistogram( QgsRasterHistogram &histogram,
                        int bandNo,
                        int binCount,
                        double minimum = std::numeric_limits<double>::quiet_NaN(),
                        double maximum = std::numeric_limits<double>::quiet_NaN(),
                        const QgsRectangle &boundingBox = QgsRectangle(),
                        int sampleSize = 0,
                        bool includeOutOfRange = false );
#else
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
    % MethodCode
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

#if defined(SIP_PROTECTED_IS_PUBLIC) || (SIP_VERSION >= 0x050000 && !defined(_MSC_VER))
    sipCpp->initHistogram( *a0, a1, a2, minimum, maximum, *a5, a6, a7 );
#else
    sipCpp->sipProtect_initHistogram( *a0, a1, a2, minimum, maximum, *a5, a6, a7 );
#endif
    % End
#endif

    //! Fill in statistics defaults if not specified
    void initStatistics( QgsRasterBandStats &statistics, int bandNo,
                         int stats = QgsRasterBandStats::All,
                         const QgsRectangle &boundingBox = QgsRectangle(),
                         int binCount = 0 ) const;

  private:
#ifdef SIP_RUN
    QgsRasterInterface( const QgsRasterInterface & );
    QgsRasterInterface &operator=( const QgsRasterInterface & );
#endif

    Q_DISABLE_COPY( QgsRasterInterface )   // there is clone() for copying
};

#endif
