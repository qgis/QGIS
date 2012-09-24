
/**  Base class for processing modules.
 *
 */

class QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterinterface.h>
#include <qgsrasterresamplefilter.h>
//#include <qgsrasternuller.h>
#include <qgsrasterrenderer.h>
#include <qgsrasterprojector.h>
#include <qgsrasterdataprovider.h>

// QgsRasterRenderer subclass headers must be here because ConvertToSubClassCode
// from QgsRasterRenderer is probably included
#include <qgspalettedrasterrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandcolordatarenderer.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsRasterResampleFilter*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterResampleFilter;
//  if (dynamic_cast<QgsRasterNuller*>(sipCpp) != NULL)
//    sipClass = sipClass_QgsRasterNuller;
  if (dynamic_cast<QgsRasterRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterRenderer;
  if (dynamic_cast<QgsRasterProjector*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterProjector;
  if (dynamic_cast<QgsRasterDataProvider*>(sipCpp) != NULL)
  {
    sipClass = sipClass_QgsRasterDataProvider;
    *sipCppRet = static_cast<QgsRasterDataProvider*>(sipCpp);
  }
  else
    sipClass = 0;
%End

  public:

    /** Data types.
     *  This is modified and extended copy of GDALDataType.
     */
    enum DataType
    {
      /*! Unknown or unspecified type */          UnknownDataType = 0,
      /*! Eight bit unsigned integer */           Byte = 1,
      /*! Sixteen bit unsigned integer */         UInt16 = 2,
      /*! Sixteen bit signed integer */           Int16 = 3,
      /*! Thirty two bit unsigned integer */      UInt32 = 4,
      /*! Thirty two bit signed integer */        Int32 = 5,
      /*! Thirty two bit floating point */        Float32 = 6,
      /*! Sixty four bit floating point */        Float64 = 7,
      /*! Complex Int16 */                        CInt16 = 8,
      /*! Complex Int32 */                        CInt32 = 9,
      /*! Complex Float32 */                      CFloat32 = 10,
      /*! Complex Float64 */                      CFloat64 = 11,
      /*! Color, alpha, red, green, blue, 4 bytes the same as
          QImage::Format_ARGB32 */                ARGB32 = 12,
      /*! Color, alpha, red, green, blue, 4 bytes  the same as
          QImage::Format_ARGB32_Premultiplied */  ARGB32_Premultiplied = 13,

      TypeCount = 14          /* maximum type # + 1 */
    };

    struct Range
    {
      double min;
      double max;
      bool operator==( const QgsRasterInterface::Range &o ) const;
    };

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    int typeSize( int dataType ) const;

    /** Clone itself, create deep copy */
    virtual QgsRasterInterface *clone() const = 0 /Factory/;

    int dataTypeSize( int bandNo ) const;

    /** Returns true if data type is numeric */
    bool typeIsNumeric( QgsRasterInterface::DataType type ) const;

    /** Returns true if data type is color */
    bool typeIsColor( QgsRasterInterface::DataType type ) const;

    /** Returns data type for the band specified by number */
    virtual QgsRasterInterface::DataType dataType( int bandNo ) const;

    /** For given data type returns wider type and sets no data value */
    static QgsRasterInterface::DataType typeWithNoDataValue( DataType dataType, double *noDataValue );

    /** Get number of bands */
    virtual int bandCount() const;

    /** Return no data value for specific band. Each band/provider must have
     * no data value, if there is no one set in original data, provider decides one
     * possibly using wider data type.
     * @param bandNo band number
     * @return No data value */
    virtual double noDataValue( int bandNo ) const;

    /** Test if value is nodata for specific band
     * @param bandNo band number
     * @param value tested value
     * @return true if value is nodata */
    virtual bool isNoDataValue( int bandNo, double value ) const;

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    void *block( int bandNo, const QgsRectangle &extent, int width, int height );

    /** Read block of data using given extent and size.
     *  Method to be implemented by subclasses.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    virtual void *readBlock( int bandNo, const QgsRectangle &extent, int width, int height );

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input );

    /** Current input */
    virtual QgsRasterInterface * input() const;

    /** Is on/off */
    virtual bool on();

    /** Set on/off */
    virtual void setOn( bool on );

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface *srcInput();

    /** Create a new image with extraneous data, such data may be used
     *  after the image is destroyed. The memory is not initialized.
     */
    QImage * createImage( int width, int height, QImage::Format format ) /Factory/;

    /** Switch on (and clear old statistics) or off collection of statistics */
    void setStatsOn( bool on );

    /** Last total time (for allbands) consumed by this interface for call to block()
     * If cumulative is true, the result includes also time spent in all preceding
     * interfaces. If cumulative is false, only time consumed by this interface is
     * returned. */
    double time( bool cumulative = false );

    static double readValue( void *data, QgsRasterInterface::DataType type, int index );

    static void writeValue( void *data, QgsRasterInterface::DataType type, int index, double value );

    /** \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  @param value the value to be printed
     *  @return string representing the value*/
    static QString printValue( double value );

    /** \brief Convert block of data from one type to another. Original block memory
     *         is not release.
     *  @param srcData source data
     *  @param srcDataType source data type
     *  @param destDataType dest data type
     *  @param size block size (width * height)
     *  @return block of data in destDataType */
    static void * convert( void *srcData, QgsRasterInterface::DataType srcDataType, QgsRasterInterface::DataType destDataType, int size ) /Factory/;

  protected:
    /** \brief Test if value is within the list of ranges
     *  @param value value
     *  @param rangeList list of ranges
     *  @return true if value is in at least one of ranges
     */
    static bool valueInRange( double value, QList<QgsRasterInterface::Range> rangeList );
};

