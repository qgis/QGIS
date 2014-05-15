class QgsRasterBlock
{
%TypeHeaderCode
#include <qgis.h>
#include <qgsrasterblock.h>
%End

  public:
    QgsRasterBlock();

    /** \brief Constructor which allocates data block in memory
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @note not available in python bindings (use variant with theNoDataValue)
     */
    // QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight );

    /** \brief Constructor which allocates data block in memory
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     */
    QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    virtual ~QgsRasterBlock();

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @return true on success
     *  @note not available in python bindings (use variant with theNoDataValue)
     */
    // bool reset( QGis::DataType theDataType, int theWidth, int theHeight );

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     *  @return true on success
     */
    bool reset( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    // TODO: consider if use isValid() at all, isEmpty() should be sufficient
    // and works also if block is valid but empty - difference between valid and empty?
    /** \brief Returns true if the block is valid (correctly filled with data).
     *  An empty block may still be valid (if zero size block was requested).
     *  If the block is not valid, error may be retrieved by error() method.
     */
    bool isValid() const;

    /** \brief Mark block as valid or invalid */
    void setValid( bool valid );

    /** Returns true if block is empty, i.e. its size is 0 (zero rows or cols).
     *  This method does not return true if size is not zero and all values are
     *  'no data' (null).
     */
    bool isEmpty() const;

    // Return data type size in bytes
    static int typeSize( int dataType );

    // Data type in bytes
    int dataTypeSize() const;

    /** Returns true if data type is numeric */
    static bool typeIsNumeric( QGis::DataType type );

    /** Returns true if data type is color */
    static bool typeIsColor( QGis::DataType type );

    /** Returns data type */
    QGis::DataType dataType() const;

    /** For given data type returns wider type and sets no data value */
    static QGis::DataType typeWithNoDataValue( QGis::DataType dataType, double *noDataValue );

    /** True if the block has no data value.
     * @return true if the block has no data value */
    bool hasNoDataValue() const;

    /** Returns true if the block may contain no data. It does not guarantee
     * that it really contains any no data. It can be used to speed up processing.
     * Not the difference between this method and hasNoDataValue().
     * @return true if the block may contain no data */
    bool hasNoData() const;

    /** Return no data value. If the block does not have a no data value the
     *  returned value is undefined.
     * @return No data value */
    double noDataValue() const;

    /** Get byte array representing a value.
     * @param theDataType data type
     * @param theValue value
     * @return byte array representing the value */
    static QByteArray valueBytes( QGis::DataType theDataType, double theValue );

    /** \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  @param row row index
     *  @param column column index
     *  @return value */
    double value( int row, int column ) const;

    /** \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  @param index data matrix index (long type in Python)
     *  @return value */
    double value( qgssize index ) const;

    /** \brief Read a single color
     *  @param row row index
     *  @param column column index
     *  @return color */
    QRgb color( int row, int column ) const;

    /** \brief Read a single value
     *  @param index data matrix index (long type in Python)
     *  @return color */
    QRgb color( qgssize index ) const;

    /** \brief Check if value at position is no data
     *  @param row row index
     *  @param column column index
     *  @return true if value is no data */
    bool isNoData( int row, int column );

    /** \brief Check if value at position is no data
     *  @param index data matrix index (long type in Python)
     *  @return true if value is no data */
    bool isNoData( qgssize index );

    /** \brief Set value on position
     *  @param row row index
     *  @param column column index
     *  @param value the value to be set
     *  @return true on success */
    bool setValue( int row, int column, double value );

    /** \brief Set value on index (indexed line by line)
     *  @param index data matrix index (long type in Python)
     *  @param value the value to be set
     *  @return true on success */
    bool setValue( qgssize index, double value );

    /** \brief Set color on position
     *  @param row row index
     *  @param column column index
     *  @param color the color to be set, QRgb value
     *  @return true on success */
    bool setColor( int row, int column, QRgb color );

    /** \brief Set color on index (indexed line by line)
     *  @param index data matrix index (long type in Python)
     *  @param color the color to be set, QRgb value
     *  @return true on success */
    bool setColor( qgssize index, QRgb color );

    /** \brief Set no data on pixel
     *  @param row row index
     *  @param column column index
     *  @return true on success */
    bool setIsNoData( int row, int column );

    /** \brief Set no data on pixel
     *  @param index data matrix index (long type in Python)
     *  @return true on success */
    bool setIsNoData( qgssize index );

    /** \brief Set the whole block to no data
     *  @return true on success */
    bool setIsNoData();

    /** \brief Set the whole block to no data except specified rectangle
     *  @return true on success */
    bool setIsNoDataExcept( const QRect & theExceptRect );

    /** \brief Get pointer to data
     *  @param row row index
     *  @param column column index
     *  @return pointer to data
     *  @note not available in python bindings
     */
    // char * bits( int row, int column );

    /** \brief Get pointer to data
     *  @param index data matrix index
     *  @return pointer to data
     *  @note not available in python bindings
     */
    // char * bits( qgssize index );

    /** \brief Get pointer to data
     *  @return pointer to data
     *  @note not available in python bindings
     */
    // char * bits();

    /** \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  @param value the value to be printed
     *  @return string representing the value*/
    static QString printValue( double value );

    /** \brief Convert data to different type.
     *  @param destDataType dest data type
     *  @return true on success */
    bool convert( QGis::DataType destDataType );

    /** \brief Get image if type is color.
    *   @return image */
    QImage image() const;

    /** \brief set image.
     *  @param image image
     *  @return true on success */
    bool setImage( const QImage * image );

    // @note not available in python bindings
    // inline static double readValue( void *data, QGis::DataType type, qgssize index );

    // @note not available in python bindings
    // inline static void writeValue( void *data, QGis::DataType type, qgssize index, double value );

    void applyNoDataValues( const QgsRasterRangeList & rangeList );

    /** apply band scale and offset to raster block values
     * @@note added in 2.3 */
    void applyScaleOffset( double scale, double offset );

    /** \brief Get error */
    QgsError error() const;

    /** \brief Set error */
    void setError( const QgsError & theError );

    /** \brief For theExtent and theWidht, theHeight find rectangle covered by subextent.
     * The output rect has x oriented from left to right and y from top to bottom
     * (upper-left to lower-right orientation).
     * @param theExtent extent, usually the larger
     * @param theWidth numbers of columns in theExtent
     * @param theHeight numbers of rows in theExtent
     * @param theSubExtent extent, usually smaller than theExtent
     * @return the rectangle covered by sub extent
     */
    static QRect subRect( const QgsRectangle &theExtent, int theWidth, int theHeight, const QgsRectangle &theSubExtent );
};

