class QgsRasterBlock
{
%TypeHeaderCode
#include <qgsrasterblock.h>
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
          QImage::Format_ARGB32_Premultiplied */  ARGB32_Premultiplied = 13
    };

    struct Range
    {
      double min;
      double max;
      bool operator==( const QgsRasterBlock::Range &o ) const;
    };

    QgsRasterBlock();

    QgsRasterBlock( DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    virtual ~QgsRasterBlock();

    bool reset( DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    bool isEmpty() const;

    int typeSize( int dataType ) const;

    int dataTypeSize( int bandNo ) const;

    /** Returns true if data type is numeric */
    bool typeIsNumeric( QgsRasterBlock::DataType type ) const;

    /** Returns true if data type is color */
    bool typeIsColor( QgsRasterBlock::DataType type ) const;

    /** Returns data type for the band specified by number */
    virtual QgsRasterBlock::DataType dataType() const;

    /** For given data type returns wider type and sets no data value */
    static QgsRasterBlock::DataType typeWithNoDataValue( DataType dataType, double *noDataValue );

    double noDataValue( ) const;

    void setNoDataValue( double noDataValue );

    static bool isNoDataValue( double value, double noDataValue );

    bool isNoDataValue( double value ) const;

    double value( int row, int column ) const;
    double value( size_t index) const;
    QRgb color( int row, int column ) const;
    QRgb color( size_t index) const;
    bool isNoData( int row, int column );
    bool isNoData( size_t index );
    bool setValue( int row, int column, double value );
    bool setValue( size_t index, double value );
    bool setColor( int row, int column, QRgb color );
    bool setColor( size_t index, QRgb color );
    // Not desired to give direct access to data in Python, could cause crash
    //char * bits( int row, int column );
    //char * bits( size_t index );
    static QString printValue( double value );

    bool convert( QgsRasterBlock::DataType destDataType );
    QImage image() const;
    bool setImage( const QImage * image );
  
    static bool valueInRange( double value, const QList<QgsRasterBlock::Range> &rangeList );

};

