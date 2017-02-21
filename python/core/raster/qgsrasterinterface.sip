
/** \ingroup core
 * Feedback object tailored for raster block reading.
 *
 * @note added in QGIS 3.0
 */
class QgsRasterBlockFeedback : QgsFeedback
{
%TypeHeaderCode
#include <qgsrasterinterface.h>
%End

  public:
    //! Construct a new raster block feedback object
    QgsRasterBlockFeedback( QObject* parent = nullptr );

    //! May be emitted by raster data provider to indicate that some partial data are available
    //! and a new preview image may be produced
    virtual void onNewData();

    //! Whether the raster provider should return only data that are already available
    //! without waiting for full result. By default this flag is not enabled.
    //! @see setPreviewOnly()
    bool isPreviewOnly() const;
    //! set flag whether the block request is for preview purposes only
    //! @see isPreviewOnly()
    void setPreviewOnly( bool preview );

    //! Whether our painter is drawing to a temporary image used just by this layer
    //! @see setRenderPartialOutput()
    bool renderPartialOutput() const;
    //! Set whether our painter is drawing to a temporary image used just by this layer
    //! @see renderPartialOutput()
    void setRenderPartialOutput( bool enable );

};


/** Base class for processing modules.
 *
 */

class QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterinterface.h>

// QgsRasterInterface subclasses
#include <qgsbrightnesscontrastfilter.h>
#include <qgshuesaturationfilter.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasternuller.h>
#include <qgsrasterprojector.h>
#include <qgsrasterrenderer.h>
#include <qgsrasterresamplefilter.h>

// QgsRasterRenderer subclasses
#include <qgsmultibandcolorrenderer.h>
#include <qgspalettedrasterrenderer.h>
#include <qgssinglebandcolordatarenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
%End

%ConvertToSubClassCode
  if (     dynamic_cast<QgsBrightnessContrastFilter*>(sipCpp))
    sipType = sipType_QgsBrightnessContrastFilter;
  else if (dynamic_cast<QgsHueSaturationFilter*>(sipCpp))
    sipType = sipType_QgsHueSaturationFilter;
  else if (dynamic_cast<QgsRasterDataProvider*>(sipCpp))
  {
    sipType = sipType_QgsRasterDataProvider;
    // use static cast because QgsRasterDataProvider has multiple inheritance
    // and we would end up with bad pointer otherwise!
    *sipCppRet = static_cast<QgsRasterDataProvider*>(sipCpp);
  }
  else if (dynamic_cast<QgsRasterNuller*>(sipCpp))
    sipType = sipType_QgsRasterNuller;
  else if (dynamic_cast<QgsRasterProjector*>(sipCpp))
    sipType = sipType_QgsRasterProjector;
  else if (dynamic_cast<QgsRasterRenderer*>(sipCpp))
  {
    if (     dynamic_cast<QgsMultiBandColorRenderer*>(sipCpp))
      sipType = sipType_QgsMultiBandColorRenderer;
    else if (dynamic_cast<QgsPalettedRasterRenderer*>(sipCpp))
      sipType = sipType_QgsPalettedRasterRenderer;
    else if (dynamic_cast<QgsSingleBandColorDataRenderer*>(sipCpp))
      sipType = sipType_QgsSingleBandColorDataRenderer;
    else if (dynamic_cast<QgsSingleBandGrayRenderer*>(sipCpp))
      sipType = sipType_QgsSingleBandGrayRenderer;
    else if (dynamic_cast<QgsSingleBandPseudoColorRenderer*>(sipCpp))
      sipType = sipType_QgsSingleBandPseudoColorRenderer;
    else
      sipType = sipType_QgsRasterRenderer;
  }
  else if (dynamic_cast<QgsRasterResampleFilter*>(sipCpp))
    sipType = sipType_QgsRasterResampleFilter;
  else
    sipType = 0;
%End

  public:
    //! If you add to this, please also add to capabilitiesString()
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

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    /** Clone itself, create deep copy */
    virtual QgsRasterInterface *clone() const = 0 /Factory/;

    /** Returns a bitmask containing the supported capabilities */
    virtual int capabilities() const;

    /**
     *  Returns the above in friendly format.
     */
    QString capabilitiesString() const;

    /** Returns data type for the band specified by number */
    virtual Qgis::DataType dataType( int bandNo ) const = 0;

    /** Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType */
    virtual Qgis::DataType sourceDataType( int bandNo ) const;

    /**
     * Get the extent of the interface.
     * @return QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const;

    int dataTypeSize( int bandNo );

    /** Get number of bands */
    virtual int bandCount() const = 0;

    /** Get block size */
    virtual int xBlockSize() const;
    virtual int yBlockSize() const;

    /** Get raster size */
    virtual int xSize() const;
    virtual int ySize() const;

    /** \brief helper function to create zero padded band names */
    virtual QString generateBandName( int bandNumber ) const;

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     * @param bandNo band number
     * @param extent extent of block
     * @param width pixel width of block
     * @param height pixel height of block
     * @param feedback optional raster feedback object for cancelation/preview. Added in QGIS 3.0.
     */
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) = 0 /Factory/;

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input );

    /** Current input */
    virtual QgsRasterInterface * input() const;

    /** Is on/off */
    virtual bool on() const;

    /** Set on/off */
    virtual void setOn( bool on );

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface *sourceInput();

    /** \brief Get band statistics.
     * @param bandNo The band (number).
     * @param stats Requested statistics
     * @param extent Extent used to calc statistics, if empty, whole raster extent is used.
     * @param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @return Band statistics.
     */
    virtual QgsRasterBandStats bandStatistics( int bandNo,
        int stats = QgsRasterBandStats::All,
        const QgsRectangle & extent = QgsRectangle(),
        int sampleSize = 0 );

    /** \brief Returns true if histogram is available (cached, already calculated).     *   The parameters are the same as in bandStatistics()
     * @return true if statistics are available (ready to use)
     */
    virtual bool hasStatistics( int bandNo,
                                int stats = QgsRasterBandStats::All,
                                const QgsRectangle & extent = QgsRectangle(),
                                int sampleSize = 0 );

    /** \brief Get histogram. Histograms are cached in providers.
     * @param bandNo The band (number).
     * @param binCount Number of bins (intervals,buckets). If 0, the number of bins is decided automatically according to data type, raster size etc.
     * @param minimum Minimum value, if NaN, raster minimum value will be used.
     * @param maximum Maximum value, if NaN, raster minimum value will be used.
     * @param extent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @param includeOutOfRange include out of range values
     * @return Vector of non NULL cell counts for each bin.
     * @note binCount, minimum and maximum not optional in python bindings
     */
    virtual QgsRasterHistogram histogram( int bandNo,
                                          int binCount,
                                          double minimum,
                                          double maximum,
                                          const QgsRectangle & extent,
                                          int sampleSize,
                                          bool includeOutOfRange );

    /** \brief Returns true if histogram is available (cached, already calculated), the parameters are the same as in histogram()
     * @note binCount, minimum and maximum not optional in python bindings
     */
    virtual bool hasHistogram( int bandNo,
                               int binCount,
                               double minimum,
                               double maximum,
                               const QgsRectangle & extent,
                               int sampleSize,
                               bool includeOutOfRange );

    /** \brief Find values for cumulative pixel count cut.
     * @param bandNo The band (number).
     * @param lowerCount The lower count as fraction of 1, e.g. 0.02 = 2%
     * @param upperCount The upper count as fraction of 1, e.g. 0.98 = 98%
     * @param lowerValue Location into which the lower value will be set.
     * @param upperValue  Location into which the upper value will be set.
     * @param extent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param sampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     */
    virtual void cumulativeCut( int bandNo,
                                double lowerCount,
                                double upperCount,
                                double &lowerValue,
                                double &upperValue,
                                const QgsRectangle & extent = QgsRectangle(),
                                int sampleSize = 0 );

    /** Write base class members to xml. */
    virtual void writeXml( QDomDocument& doc, QDomElement& parentElem ) const;
    /** Sets base class members from xml. Usually called from create() methods of subclasses */
    virtual void readXml( const QDomElement& filterElem );

  protected:

    /** Fill in histogram defaults if not specified
     * @note binCount, minimum and maximum not optional in python bindings
     */
    void initHistogram( QgsRasterHistogram &histogram, int bandNo,
                        int binCount /* = 0 */,
                        double minimum /*= std::numeric_limits<double>::quiet_NaN()*/,
                        double maximum /*= std::numeric_limits<double>::quiet_NaN()*/,
                        const QgsRectangle & boundingBox = QgsRectangle(),
                        int sampleSize = 0,
                        bool includeOutOfRange = false );

    //! Fill in statistics defaults if not specified
    void initStatistics(QgsRasterBandStats &statistics, int bandNo,
                         int stats = QgsRasterBandStats::All,
                         const QgsRectangle & boundingBox = QgsRectangle(),
                         int binCount = 0 );

  private:
    QgsRasterInterface(const QgsRasterInterface &);
    QgsRasterInterface &operator=(const QgsRasterInterface &);

};

