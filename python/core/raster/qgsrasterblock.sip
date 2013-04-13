class QgsRasterBlock
{
%TypeHeaderCode
#include <qgis.h>
#include <qgsrasterblock.h>
%End

  public:
    QgsRasterBlock();

    QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    virtual ~QgsRasterBlock();

    bool reset( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    bool isEmpty() const;

    int typeSize( int dataType ) const;

    int dataTypeSize( int bandNo ) const;

    /** Returns true if data type is numeric */
    bool typeIsNumeric( QGis::DataType type ) const;

    /** Returns true if data type is color */
    bool typeIsColor( QGis::DataType type ) const;

    /** Returns data type for the band specified by number */
    virtual QGis::DataType dataType() const;

    /** For given data type returns wider type and sets no data value */
    static QGis::DataType typeWithNoDataValue( QGis::DataType dataType, double *noDataValue );

    bool hasNoDataValue() const;

    double noDataValue( ) const;

    //void setNoDataValue( double noDataValue );

    //static bool isNoDataValue( double value, double noDataValue );

    //bool isNoDataValue( double value ) const;

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

    bool convert( QGis::DataType destDataType );
    QImage image() const;
    bool setImage( const QImage * image );
};

