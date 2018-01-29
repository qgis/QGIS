/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterblock.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterBlock
{
%Docstring
Raster data container.
%End

%TypeHeaderCode
#include "qgsrasterblock.h"
%End
  public:
    QgsRasterBlock();

    QgsRasterBlock( Qgis::DataType dataType, int width, int height );
%Docstring
 Constructor which allocates data block in memory

:param dataType: raster data type
:param width: width of data matrix
:param height: height of data matrix
%End

    virtual ~QgsRasterBlock();

    bool reset( Qgis::DataType dataType, int width, int height );
%Docstring
 Reset block

:param dataType: raster data type
:param width: width of data matrix
:param height: height of data matrix

:return: true on success
%End


    bool isValid() const;
%Docstring
 Returns true if the block is valid (correctly filled with data).
An empty block may still be valid (if zero size block was requested).
If the block is not valid, error may be retrieved by error() method.
%End

    void setValid( bool valid );
%Docstring
 Mark block as valid or invalid
%End

    bool isEmpty() const;
%Docstring
Returns true if block is empty, i.e. its size is 0 (zero rows or cols).
This method does not return true if size is not zero and all values are
'no data' (null).
%End

    static int typeSize( int dataType );

    int dataTypeSize() const;

    static bool typeIsNumeric( Qgis::DataType type );
%Docstring
Returns true if data type is numeric
%End

    static bool typeIsColor( Qgis::DataType type );
%Docstring
Returns true if data type is color
%End

    Qgis::DataType dataType() const;
%Docstring
Returns data type
%End

    static Qgis::DataType typeWithNoDataValue( Qgis::DataType dataType, double *noDataValue );
%Docstring
For given data type returns wider type and sets no data value
%End

    bool hasNoDataValue() const;
%Docstring
True if the block has no data value.

:return: true if the block has no data value

.. seealso:: :py:func:`noDataValue`
%End

    bool hasNoData() const;
%Docstring
Returns true if the block may contain no data. It does not guarantee
that it really contains any no data. It can be used to speed up processing.
Not the difference between this method and hasNoDataValue().

:return: true if the block may contain no data *
%End

    void setNoDataValue( double noDataValue );
%Docstring
Sets cell value that will be considered as "no data".

.. versionadded:: 3.0

.. seealso:: :py:func:`noDataValue`
%End

    void resetNoDataValue();
%Docstring
Reset no data value: if there was a no data value previously set,
it will be discarded.

.. versionadded:: 3.0

.. seealso:: :py:func:`noDataValue`
%End

    double noDataValue() const;
%Docstring
Return no data value. If the block does not have a no data value the
returned value is undefined.

:return: No data value

.. seealso:: :py:func:`hasNoDataValue`
%End

    static QByteArray valueBytes( Qgis::DataType dataType, double value );
%Docstring
Get byte array representing a value.

:param dataType: data type
:param value: value

:return: byte array representing the value *
%End

    double value( int row, int column ) const;
%Docstring
 Read a single value if type of block is numeric. If type is color,
returned value is undefined.

:param row: row index
:param column: column index

:return: value *
%End

    double value( qgssize index ) const;
%Docstring
 Read a single value if type of block is numeric. If type is color,
returned value is undefined.

:param index: data matrix index (long type in Python)

:return: value *
%End

    QRgb color( int row, int column ) const;
%Docstring
 Read a single color

:param row: row index
:param column: column index

:return: color *
%End

    QRgb color( qgssize index ) const;
%Docstring
 Read a single value

:param index: data matrix index (long type in Python)

:return: color *
%End

    bool isNoData( int row, int column );
%Docstring
 Check if value at position is no data

:param row: row index
:param column: column index

:return: true if value is no data *
%End

    bool isNoData( qgssize index );
%Docstring
 Check if value at position is no data

:param index: data matrix index (long type in Python)

:return: true if value is no data *
%End

    bool setValue( int row, int column, double value );
%Docstring
 Set value on position

:param row: row index
:param column: column index
:param value: the value to be set

:return: true on success *
%End

    bool setValue( qgssize index, double value );
%Docstring
 Set value on index (indexed line by line)

:param index: data matrix index (long type in Python)
:param value: the value to be set

:return: true on success *
%End

    bool setColor( int row, int column, QRgb color );
%Docstring
 Set color on position

:param row: row index
:param column: column index
:param color: the color to be set, QRgb value

:return: true on success *
%End

    bool setColor( qgssize index, QRgb color );
%Docstring
 Set color on index (indexed line by line)

:param index: data matrix index (long type in Python)
:param color: the color to be set, QRgb value

:return: true on success *
%End

    bool setIsNoData( int row, int column );
%Docstring
 Set no data on pixel

:param row: row index
:param column: column index

:return: true on success *
%End

    bool setIsNoData( qgssize index );
%Docstring
 Set no data on pixel

:param index: data matrix index (long type in Python)

:return: true on success *
%End

    bool setIsNoData();
%Docstring
 Set the whole block to no data

:return: true on success *
%End

    bool setIsNoDataExcept( QRect exceptRect );
%Docstring
 Set the whole block to no data except specified rectangle

:return: true on success *
%End

    void setIsData( int row, int column );
%Docstring
 Remove no data flag on pixel. If the raster block does not have an explicit
no data value set then an internal map of no data pixels is maintained for the block.
In this case it is possible to reset a pixel to flag it as having valid data using this
method. This method has no effect for raster blocks with an explicit no data value set.

:param row: row index
:param column: column index

.. versionadded:: 2.10
%End

    void setIsData( qgssize index );
%Docstring
 Remove no data flag on pixel. If the raster block does not have an explicit
no data value set then an internal map of no data pixels is maintained for the block.
In this case it is possible to reset a pixel to flag it as having valid data using this
method. This method has no effect for raster blocks with an explicit no data value set.

:param index: data matrix index (long type in Python)

.. versionadded:: 2.10
%End

    QByteArray data() const;
%Docstring
Get access to raw data.
The returned QByteArray instance is not a copy of the data: it only refers to the array
owned by the QgsRasterBlock, therefore it is only valid while the QgsRasterBlock object
still exists. Writing to the returned QByteArray will not affect the original data:
a deep copy of the data will be made and only the local copy will be modified.

.. note::

   in Python the method returns ordinary bytes object as the

.. versionadded:: 3.0
%End

    void setData( const QByteArray &data, int offset = 0 );
%Docstring
Rewrite raw pixel data.
If the data array is shorter than the internal array within the raster block object,
pixels at the end will stay untouched. If the data array is longer than the internal
array, only the initial data from the input array will be used.
Optionally it is possible to set non-zero offset (in bytes) if the input data should
overwrite data somewhere in the middle of the internal buffer.

.. versionadded:: 3.0
%End




    static QString printValue( double value );
%Docstring
 Print double value with all necessary significant digits.
It is ensured that conversion back to double gives the same number.

:param value: the value to be printed

:return: string representing the value*
%End


    bool convert( Qgis::DataType destDataType );
%Docstring
 Convert data to different type.

:param destDataType: dest data type

:return: true on success *
%End

    QImage image() const;
%Docstring
 Get image if type is color.

:return: image *
%End

    bool setImage( const QImage *image );
%Docstring
 set image.

:param image: image

:return: true on success *
%End



    void applyNoDataValues( const QgsRasterRangeList &rangeList );

    void applyScaleOffset( double scale, double offset );
%Docstring
Apply band scale and offset to raster block values

.. versionadded:: 2.3
%End

    QgsError error() const;
%Docstring
 Get error
%End

    void setError( const QgsError &error );
%Docstring
 Set error
%End

    QString toString() const;

    static QRect subRect( const QgsRectangle &extent, int width, int height, const QgsRectangle &subExtent );
%Docstring
 For extent and width, height find rectangle covered by subextent.
The output rect has x oriented from left to right and y from top to bottom
(upper-left to lower-right orientation).

:param extent: extent, usually the larger
:param width: numbers of columns in theExtent
:param height: numbers of rows in theExtent
:param subExtent: extent, usually smaller than theExtent

:return: the rectangle covered by sub extent
%End

    int width() const;
%Docstring
Returns the width (number of columns) of the raster block.

.. seealso:: :py:func:`height`

.. versionadded:: 2.10
%End

    int height() const;
%Docstring
Returns the height (number of rows) of the raster block.

.. seealso:: :py:func:`width`

.. versionadded:: 2.10
%End

};







/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterblock.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
