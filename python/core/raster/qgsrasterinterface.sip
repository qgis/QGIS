
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
    sipClass = sipClass_QgsBrightnessContrastFilter;
  else if (dynamic_cast<QgsHueSaturationFilter*>(sipCpp))
    sipClass = sipClass_QgsHueSaturationFilter;
  else if (dynamic_cast<QgsRasterDataProvider*>(sipCpp))
  {
    sipClass = sipClass_QgsRasterDataProvider;
    // use static cast because QgsRasterDataProvider has multiple inheritance
    // and we would end up with bad pointer otherwise!
    *sipCppRet = static_cast<QgsRasterDataProvider*>(sipCpp);
  }
  else if (dynamic_cast<QgsRasterNuller*>(sipCpp))
    sipClass = sipClass_QgsRasterNuller;
  else if (dynamic_cast<QgsRasterProjector*>(sipCpp))
    sipClass = sipClass_QgsRasterProjector;
  else if (dynamic_cast<QgsRasterRenderer*>(sipCpp))
  {
    if (     dynamic_cast<QgsMultiBandColorRenderer*>(sipCpp))
      sipClass = sipClass_QgsMultiBandColorRenderer;
    else if (dynamic_cast<QgsPalettedRasterRenderer*>(sipCpp))
      sipClass = sipClass_QgsPalettedRasterRenderer;
    else if (dynamic_cast<QgsSingleBandColorDataRenderer*>(sipCpp))
      sipClass = sipClass_QgsSingleBandColorDataRenderer;
    else if (dynamic_cast<QgsSingleBandGrayRenderer*>(sipCpp))
      sipClass = sipClass_QgsSingleBandGrayRenderer;
    else if (dynamic_cast<QgsSingleBandPseudoColorRenderer*>(sipCpp))
      sipClass = sipClass_QgsSingleBandPseudoColorRenderer;
    else
      sipClass = sipClass_QgsRasterRenderer;
  }
  else if (dynamic_cast<QgsRasterResampleFilter*>(sipCpp))
    sipClass = sipClass_QgsRasterResampleFilter;
  else
    sipClass = 0;
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
    virtual QGis::DataType dataType( int bandNo ) const = 0;

    /** Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType */
    virtual QGis::DataType srcDataType( int bandNo ) const;

    /**
     * Get the extent of the interface.
     * @return QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent();

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
    virtual QString generateBandName( int theBandNumber ) const;

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     * @param bandNo band number
     * @param extent extent of block
     * @param width pixel width of block
     * @param height pixel height of block
     */
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) = 0 /Factory/;

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
    virtual QgsRasterInterface *srcInput();

    /** \brief Get band statistics.
     * @param theBandNo The band (number).
     * @param theStats Requested statistics
     * @param theExtent Extent used to calc statistics, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @return Band statistics.
     */
    virtual QgsRasterBandStats bandStatistics( int theBandNo,
        int theStats = QgsRasterBandStats::All,
        const QgsRectangle & theExtent = QgsRectangle(),
        int theSampleSize = 0 );

    /** \brief Returns true if histogram is available (cached, already calculated).     *   The parameters are the same as in bandStatistics()
     * @return true if statistics are available (ready to use)
     */
    virtual bool hasStatistics( int theBandNo,
                                int theStats = QgsRasterBandStats::All,
                                const QgsRectangle & theExtent = QgsRectangle(),
                                int theSampleSize = 0 );

    /** \brief Get histogram. Histograms are cached in providers.
     * @param theBandNo The band (number).
     * @param theBinCount Number of bins (intervals,buckets). If 0, the number of bins is decided automaticaly according to data type, raster size etc.
     * @param theMinimum Minimum value, if NaN, raster minimum value will be used.
     * @param theMaximum Maximum value, if NaN, raster minimum value will be used.
     * @param theExtent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @param theIncludeOutOfRange include out of range values
     * @return Vector of non NULL cell counts for each bin.
     * @note theBinCount, theMinimun and theMaximum not optional in python bindings
     */
    virtual QgsRasterHistogram histogram( int theBandNo,
                                          int theBinCount,
                                          double theMinimum,
                                          double theMaximum,
                                          const QgsRectangle & theExtent,
                                          int theSampleSize,
                                          bool theIncludeOutOfRange );

    /** \brief Returns true if histogram is available (cached, already calculated), the parameters are the same as in histogram()
     * @note theBinCount, theMinimun and theMaximum not optional in python bindings
     */
    virtual bool hasHistogram( int theBandNo,
                               int theBinCount,
                               double theMinimum,
                               double theMaximum,
                               const QgsRectangle & theExtent,
                               int theSampleSize,
                               bool theIncludeOutOfRange );

    /** \brief Find values for cumulative pixel count cut.
     * @param theBandNo The band (number).
     * @param theLowerCount The lower count as fraction of 1, e.g. 0.02 = 2%
     * @param theUpperCount The upper count as fraction of 1, e.g. 0.98 = 98%
     * @param theLowerValue Location into which the lower value will be set.
     * @param theUpperValue  Location into which the upper value will be set.
     * @param theExtent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     */
    virtual void cumulativeCut( int theBandNo,
                                double theLowerCount,
                                double theUpperCount,
                                double &theLowerValue,
                                double &theUpperValue,
                                const QgsRectangle & theExtent = QgsRectangle(),
                                int theSampleSize = 0 );

    /** Write base class members to xml. */
    virtual void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;
    /** Sets base class members from xml. Usually called from create() methods of subclasses */
    virtual void readXML( const QDomElement& filterElem );

};

