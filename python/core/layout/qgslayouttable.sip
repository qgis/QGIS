/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayouttable.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QVector< QVariant > QgsLayoutTableRow;

typedef QVector< QVector< QVariant > > QgsLayoutTableContents;


typedef QVector< QgsLayoutTableColumn * > QgsLayoutTableColumns;



class QgsLayoutTableStyle
{
%Docstring
 Styling option for a composer table cell
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayouttable.h"
%End
  public:

    QgsLayoutTableStyle();
%Docstring
Constructor for QgsLayoutTableStyle
%End

    bool enabled;
%Docstring
Whether the styling option is enabled
%End

    QColor cellBackgroundColor;
%Docstring
Cell background color
%End

    bool writeXml( QDomElement &styleElem, QDomDocument &doc ) const;
%Docstring
 Writes the style's properties to XML for storage.
 \param styleElem an existing QDomElement in which to store the style's properties.
 \param doc QDomDocument for the destination XML.
.. seealso:: readXml
 :rtype: bool
%End

    bool readXml( const QDomElement &styleElem );
%Docstring
 Reads the style's properties from XML.
 \param styleElem a QDomElement holding the style's desired properties.
.. seealso:: writeXml
 :rtype: bool
%End

};

class QgsLayoutTable: QgsLayoutMultiFrame
{
%Docstring
 A class to display a table in the print layout, and allow
 the table to span over multiple frames
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayouttable.h"
%End
  public:

    enum HeaderHAlignment
    {
      FollowColumn,
      HeaderLeft,
      HeaderCenter,
      HeaderRight
    };

    enum HeaderMode
    {
      FirstFrame,
      AllFrames,
      NoHeaders
    };

    enum EmptyTableMode
    {
      HeadersOnly,
      HideTable,
      ShowMessage
    };

    enum WrapBehavior
    {
      TruncateText,
      WrapText
    };

    enum CellStyleGroup
    {
      OddColumns,
      EvenColumns,
      OddRows,
      EvenRows,
      FirstColumn,
      LastColumn,
      HeaderRow,
      FirstRow,
      LastRow
    };

    QgsLayoutTable( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutTable, belonging to the specified ``layout``.
%End

    ~QgsLayoutTable();

    void setCellMargin( const double margin );
%Docstring
 Sets the ``margin`` distance in mm between cell borders and their contents.
.. seealso:: cellMargin()
%End

    double cellMargin() const;
%Docstring
 Returns the margin distance between cell borders and their contents in mm.
.. seealso:: setCellMargin()
 :rtype: float
%End

    void setEmptyTableBehavior( const EmptyTableMode mode );
%Docstring
 Sets the behavior ``mode`` for empty tables with no content rows.
.. seealso:: emptyTableBehavior()
%End

    EmptyTableMode emptyTableBehavior() const;
%Docstring
 Returns the behavior mode for empty tables. This property controls
 how the table is drawn if it contains no content rows.
.. seealso:: setEmptyTableBehavior()
 :rtype: EmptyTableMode
%End

    void setEmptyTableMessage( const QString &message );
%Docstring
 Sets the ``message`` for empty tables with no content rows. This message
 is displayed in the table body if the empty table behavior is
 set to ShowMessage.
.. seealso:: emptyTableMessage()
.. seealso:: setEmptyTableBehavior()
%End

    QString emptyTableMessage() const;
%Docstring
 Returns the message for empty tables with no content rows. This message
 is displayed in the table body if the empty table behavior is
 set to ShowMessage.
.. seealso:: setEmptyTableMessage()
.. seealso:: emptyTableBehavior()
 :rtype: str
%End

    void setShowEmptyRows( const bool showEmpty );
%Docstring
 Sets whether empty rows should be drawn. Tables default to hiding empty rows.
 \param showEmpty set to true to show empty rows in the table
.. seealso:: showEmptyRows()
%End

    bool showEmptyRows() const;
%Docstring
 Returns whether empty rows are drawn in the table.
.. seealso:: setShowEmptyRows()
 :rtype: bool
%End

    void setHeaderFont( const QFont &font );
%Docstring
 Sets the ``font`` used to draw header text in the table.
.. seealso:: headerFont()
.. seealso:: setContentFont()
%End

    QFont headerFont() const;
%Docstring
 Returns the font used to draw header text in the table.
.. seealso:: setHeaderFont()
.. seealso:: contentFont()
 :rtype: QFont
%End

    void setHeaderFontColor( const QColor &color );
%Docstring
 Sets the ``color`` used to draw header text in the table.
.. seealso:: headerFontColor()
.. seealso:: setHeaderFont()
.. seealso:: setContentFontColor()
%End

    QColor headerFontColor() const;
%Docstring
 Returns the color used to draw header text in the table.
.. seealso:: setHeaderFontColor()
.. seealso:: headerFont()
.. seealso:: contentFontColor()
 :rtype: QColor
%End

    void setHeaderHAlignment( const HeaderHAlignment alignment );
%Docstring
 Sets the horizontal ``alignment`` for table headers.
.. seealso:: headerHAlignment()
%End

    HeaderHAlignment headerHAlignment() const;
%Docstring
 Returns the horizontal alignment for table headers.
.. seealso:: setHeaderHAlignment()
 :rtype: HeaderHAlignment
%End

    void setHeaderMode( const HeaderMode mode );
%Docstring
 Sets the display ``mode`` for headers in the table. This property controls
 if and where headers are shown in the table.
.. seealso:: headerMode()
%End

    HeaderMode headerMode() const;
%Docstring
 Returns the display mode for headers in the table. This property controls
 if and where headers are shown in the table.
.. seealso:: setHeaderMode()
 :rtype: HeaderMode
%End

    void setContentFont( const QFont &font );
%Docstring
 Sets the ``font`` used to draw text in table body cells.
.. seealso:: contentFont()
.. seealso:: setHeaderFont()
%End

    QFont contentFont() const;
%Docstring
 Returns the font used to draw text in table body cells.
.. seealso:: setContentFont()
.. seealso:: headerFont()
 :rtype: QFont
%End

    void setContentFontColor( const QColor &color );
%Docstring
 Sets the ``color`` used to draw text in table body cells.
.. seealso:: contentFontColor()
.. seealso:: setContentFont()
.. seealso:: setHeaderFontColor()
%End

    QColor contentFontColor() const;
%Docstring
 Returns the color used to draw text in table body cells.
.. seealso:: setContentFontColor()
.. seealso:: contentFont()
.. seealso:: headerFontColor()
 :rtype: QColor
%End

    void setShowGrid( const bool showGrid );
%Docstring
 Sets whether grid lines should be drawn in the table
 \param showGrid set to true to show grid lines
.. seealso:: showGrid()
.. seealso:: setGridStrokeWidth()
.. seealso:: setGridColor()
%End

    bool showGrid() const;
%Docstring
 Returns whether grid lines are drawn in the table
.. seealso:: setShowGrid()
.. seealso:: gridStrokeWidth()
.. seealso:: gridColor()
 :rtype: bool
%End

    void setGridStrokeWidth( const double width );
%Docstring
 Sets the ``width`` in mm for grid lines in the table.
.. seealso:: gridStrokeWidth()
.. seealso:: setShowGrid()
.. seealso:: setGridColor()
%End

    double gridStrokeWidth() const;
%Docstring
 Returns the width of grid lines in the table in mm.
.. seealso:: setGridStrokeWidth()
.. seealso:: showGrid()
.. seealso:: gridColor()
 :rtype: float
%End

    void setGridColor( const QColor &color );
%Docstring
 Sets the ``color`` used for grid lines in the table.
.. seealso:: gridColor()
.. seealso:: setShowGrid()
.. seealso:: setGridStrokeWidth()
%End

    QColor gridColor() const;
%Docstring
 Returns the color used for grid lines in the table.
.. seealso:: setGridColor()
.. seealso:: showGrid()
.. seealso:: gridStrokeWidth()
 :rtype: QColor
%End

    void setHorizontalGrid( const bool horizontalGrid );
%Docstring
 Sets whether the grid's horizontal lines should be drawn in the table
 \param horizontalGrid set to true to draw grid's horizontal lines
.. seealso:: setShowGrid()
.. seealso:: setGridStrokeWidth()
.. seealso:: setGridColor()
.. seealso:: setVerticalGrid()
%End

    bool horizontalGrid() const;
%Docstring
 Returns whether the grid's horizontal lines are drawn in the table.
.. seealso:: setShowGrid()
.. seealso:: setGridStrokeWidth()
.. seealso:: setGridColor()
.. seealso:: setVerticalGrid()
 :rtype: bool
%End

    void setVerticalGrid( const bool verticalGrid );
%Docstring
 Sets whether the grid's vertical lines should be drawn in the table
 \param verticalGrid set to true to draw grid's vertical lines
.. seealso:: setShowGrid()
.. seealso:: setGridStrokeWidth()
.. seealso:: setGridColor()
.. seealso:: setHorizontalGrid()
%End

    bool verticalGrid() const;
%Docstring
 Returns whether the grid's vertical lines are drawn in the table.
.. seealso:: setShowGrid()
.. seealso:: setGridStrokeWidth()
.. seealso:: setGridColor()
.. seealso:: setHorizontalGrid()
 :rtype: bool
%End

    void setBackgroundColor( const QColor &color );
%Docstring
 Sets the ``color`` used for background of table.
.. seealso:: backgroundColor()
.. seealso:: setGridColor()
%End

    QColor backgroundColor() const;
%Docstring
 Returns the color used for the background of the table.
.. seealso:: setBackgroundColor()
.. seealso:: gridColor()
 :rtype: QColor
%End

    void setWrapBehavior( WrapBehavior behavior );
%Docstring
 Sets the wrap ``behavior`` for the table, which controls how text within cells is
 automatically wrapped.
.. seealso:: wrapBehavior()
%End

    WrapBehavior wrapBehavior() const;
%Docstring
 Returns the wrap behavior for the table, which controls how text within cells is
 automatically wrapped.
.. seealso:: setWrapBehavior()
 :rtype: WrapBehavior
%End

    QgsLayoutTableColumns &columns();
%Docstring
 Returns a reference to the list of QgsLayoutTableColumns shown in the table
.. seealso:: setColumns()
 :rtype: QgsLayoutTableColumns
%End

    void setColumns( const QgsLayoutTableColumns &columns );
%Docstring
 Replaces the columns in the table with a specified list of QgsLayoutTableColumns.
 \param columns list of QgsLayoutTableColumns to show in table.
.. seealso:: columns()
%End

    void setCellStyle( CellStyleGroup group, const QgsLayoutTableStyle &style );
%Docstring
 Sets the cell ``style`` for a cell ``group``.
.. seealso:: cellStyle()
%End

    const QgsLayoutTableStyle *cellStyle( CellStyleGroup group ) const;
%Docstring
 Returns the cell style for a cell ``group``.
.. seealso:: setCellStyle()
 :rtype: QgsLayoutTableStyle
%End


    virtual bool getTableContents( QgsLayoutTableContents &contents ) = 0;
%Docstring
 Fetches the contents used for the cells in the table.
 :return: true if table contents were successfully retrieved.
 \param contents QgsLayoutTableContents to store retrieved row data in
 :rtype: bool
%End

    QgsLayoutTableContents &contents();
%Docstring
 Returns the current contents of the table. Excludes header cells.
 :rtype: QgsLayoutTableContents
%End

    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;

    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const;


    virtual bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context );

    virtual QSizeF totalSize() const;

     virtual void render( QgsRenderContext &context, const QRectF &renderExtent, int frameIndex,
                 const QStyleOptionGraphicsItem *itemStyle = 0 );

  public slots:

    virtual void refreshAttributes();
%Docstring
 Refreshes the contents shown in the table by querying for new data.
 This also causes the column widths and size of the table to change to accommodate the
 new data.
.. seealso:: adjustFrameToSize()
%End

    virtual void recalculateFrameSizes();


  protected:























    virtual bool calculateMaxColumnWidths();
%Docstring
 Calculates the maximum width of text shown in columns.
 :rtype: bool
%End

    virtual bool calculateMaxRowHeights();
%Docstring
 Calculates the maximum height of text shown in rows.
 :rtype: bool
%End

    double totalWidth();
%Docstring
 :rtype: float
%End

    double totalHeight();
%Docstring
 :rtype: float
%End

    int rowsVisible( double frameHeight, int firstRow, bool includeHeader, bool includeEmptyRows ) const;
%Docstring
 Calculates how many content rows would be visible within a frame of the specified
 height.
 \param frameHeight height of frame
 \param firstRow index of first row visible in frame (where 0 = first row in table)
 \param includeHeader set to true if frame would include a header row
 \param includeEmptyRows set to true to also include rows which would be empty in the returned count. For instance,
 if the frame would include all table content rows and have space left for extra rows then setting this parameter
 to true would also include a count of these extra blank rows.
 :return: number of visible content rows (excluding header row)
 :rtype: int
%End

    int rowsVisible( int frameIndex, int firstRow, bool includeEmptyRows ) const;
%Docstring
 Calculates how many content rows are visible within a given frame.
 \param frameIndex index number for frame
 \param firstRow index of first row visible in frame (where 0 = first row in table)
 \param includeEmptyRows set to true to also include rows which would be empty in the returned count. For instance,
 if the frame would include all table content rows and have space left for extra rows then setting this parameter
 to true would also include a count of these extra blank rows.
 :return: number of visible content rows (excludes header rows)
 :rtype: int
%End

    QPair<int, int> rowRange( const int frameIndex ) const;
%Docstring
 Calculates a range of rows which should be visible in a given frame.
 \param frameIndex index number for frame
 :return: row range
 :rtype: QPair<int, int>
%End

    void drawHorizontalGridLines( QPainter *painter, int firstRow, int lastRow, bool drawHeaderLines ) const;
%Docstring
 Draws the horizontal grid lines for the table.
 \param painter destination painter for grid lines
 \param firstRow index corresponding to first row shown in frame
 \param lastRow index corresponding to last row shown in frame. If greater than the number of content rows in the
 table, then the default row height will be used for the remaining rows.
 \param drawHeaderLines set to true to include for the table header
.. seealso:: drawVerticalGridLines()
%End


    void recalculateTableSize();
%Docstring
 Recalculates and updates the size of the table and all table frames.
%End

    bool contentsContainsRow( const QgsLayoutTableContents &contents, const QgsLayoutTableRow &row ) const;
%Docstring
 Checks whether a table contents contains a given row
 \param contents table contents to check
 \param row row to check for
 :return: true if contents contains rows
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayouttable.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
