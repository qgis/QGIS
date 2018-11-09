/***************************************************************************
                         qgslayouttable.h
                         ----------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTTABLE_H
#define QGSLAYOUTTABLE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgslayoutmultiframe.h"
#include <QFont>
#include <QColor>
#include <QPair>

class QgsLayoutTableColumn;

/**
 * \ingroup core
 * List of QVariants, representing a the contents of a single row in
 * a QgsComposerTable
 * \since QGIS 3.0
*/
typedef QVector< QVariant > QgsLayoutTableRow;

/**
 * \ingroup core
 * List of QgsLayoutTableRows, representing rows and column cell contents
 * for a QgsLayoutTable
 * \since QGIS 3.0
*/
#ifndef SIP_RUN
typedef QVector< QgsLayoutTableRow > QgsLayoutTableContents;
#else
typedef QVector< QVector< QVariant > > QgsLayoutTableContents;
#endif


/**
 * \ingroup core
 * List of column definitions for a QgsLayoutTable
 * \since QGIS 3.0
*/
typedef QVector< QgsLayoutTableColumn * > QgsLayoutTableColumns;


/**
 * \ingroup core
 *  \class QgsLayoutTableStyle
 *  \brief Styling option for a composer table cell
 *  \since QGIS 3.0
 */

class CORE_EXPORT QgsLayoutTableStyle
{
  public:

    //! Constructor for QgsLayoutTableStyle
    QgsLayoutTableStyle() = default;

    //! Whether the styling option is enabled
    bool enabled = false;

    //! Cell background color
    QColor cellBackgroundColor = QColor( 255, 255, 255, 255 );

    /**
     * Writes the style's properties to XML for storage.
     * \param styleElem an existing QDomElement in which to store the style's properties.
     * \param doc QDomDocument for the destination XML.
     * \see readXml
     */
    bool writeXml( QDomElement &styleElem, QDomDocument &doc ) const;

    /**
     * Reads the style's properties from XML.
     * \param styleElem a QDomElement holding the style's desired properties.
     * \see writeXml
     */
    bool readXml( const QDomElement &styleElem );

};

/**
 * A class to display a table in the print layout, and allow
 * the table to span over multiple frames
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutTable: public QgsLayoutMultiFrame
{
    Q_OBJECT

  public:

    /**
     * Controls how headers are horizontally aligned in a table
     */
    enum HeaderHAlignment
    {
      FollowColumn, //!< Header uses the same alignment as the column
      HeaderLeft, //!< Align headers left
      HeaderCenter, //!< Align headers to center
      HeaderRight //!< Align headers right
    };

    /**
     * Controls where headers are shown in the table
     */
    enum HeaderMode
    {
      FirstFrame = 0, //!< Header shown on first frame only
      AllFrames, //!< Headers shown on all frames
      NoHeaders //!< No headers shown for table
    };

    /**
     * Controls how empty tables are displayed
     */
    enum EmptyTableMode
    {
      HeadersOnly = 0, //!< Show header rows only
      HideTable, //!< Hides entire table if empty
      ShowMessage //!< Shows preset message instead of table contents
    };

    /**
     * Controls how long strings in the table are handled
     */
    enum WrapBehavior
    {
      TruncateText = 0, //!< Text which doesn't fit inside the cell is truncated
      WrapText //!< Text which doesn't fit inside the cell is wrapped. Note that this only applies to text in columns with a fixed width.
    };

    /**
     * Row or column groups for cell styling
     */
    enum CellStyleGroup
    {
      OddColumns, //!< Style odd numbered columns
      EvenColumns, //!< Style even numbered columns
      OddRows, //!< Style odd numbered rows
      EvenRows, //!< Style even numbered rows
      FirstColumn, //!< Style first column only
      LastColumn, //!< Style last column only
      HeaderRow, //!< Style header row
      FirstRow, //!< Style first row only
      LastRow //!< Style last row only
    };

    /**
     * Constructor for QgsLayoutTable, belonging to the specified \a layout.
     */
    QgsLayoutTable( QgsLayout *layout );

    ~QgsLayoutTable() override;

    /**
     * Sets the \a margin distance in mm between cell borders and their contents.
     * \see cellMargin()
     */
    void setCellMargin( double margin );

    /**
     * Returns the margin distance between cell borders and their contents in mm.
     * \see setCellMargin()
     */
    double cellMargin() const { return mCellMargin; }

    /**
     * Sets the behavior \a mode for empty tables with no content rows.
     * \see emptyTableBehavior()
     */
    void setEmptyTableBehavior( EmptyTableMode mode );

    /**
     * Returns the behavior mode for empty tables. This property controls
     * how the table is drawn if it contains no content rows.
     * \see setEmptyTableBehavior()
     */
    EmptyTableMode emptyTableBehavior() const { return mEmptyTableMode; }

    /**
     * Sets the \a message for empty tables with no content rows. This message
     * is displayed in the table body if the empty table behavior is
     * set to ShowMessage.
     * \see emptyTableMessage()
     * \see setEmptyTableBehavior()
     */
    void setEmptyTableMessage( const QString &message );

    /**
     * Returns the message for empty tables with no content rows. This message
     * is displayed in the table body if the empty table behavior is
     * set to ShowMessage.
     * \see setEmptyTableMessage()
     * \see emptyTableBehavior()
     */
    QString emptyTableMessage() const { return mEmptyTableMessage; }

    /**
     * Sets whether empty rows should be drawn. Tables default to hiding empty rows.
     * \param showEmpty set to true to show empty rows in the table
     * \see showEmptyRows()
     */
    void setShowEmptyRows( bool showEmpty );

    /**
     * Returns whether empty rows are drawn in the table.
     * \see setShowEmptyRows()
     */
    bool showEmptyRows() const { return mShowEmptyRows; }

    /**
     * Sets the \a font used to draw header text in the table.
     * \see headerFont()
     * \see setContentFont()
     */
    void setHeaderFont( const QFont &font );

    /**
     * Returns the font used to draw header text in the table.
     * \see setHeaderFont()
     * \see contentFont()
     */
    QFont headerFont() const { return mHeaderFont; }

    /**
     * Sets the \a color used to draw header text in the table.
     * \see headerFontColor()
     * \see setHeaderFont()
     * \see setContentFontColor()
     */
    void setHeaderFontColor( const QColor &color );

    /**
     * Returns the color used to draw header text in the table.
     * \see setHeaderFontColor()
     * \see headerFont()
     * \see contentFontColor()
     */
    QColor headerFontColor() const { return mHeaderFontColor; }

    /**
     * Sets the horizontal \a alignment for table headers.
     * \see headerHAlignment()
     */
    void setHeaderHAlignment( HeaderHAlignment alignment );

    /**
     * Returns the horizontal alignment for table headers.
     * \see setHeaderHAlignment()
     */
    HeaderHAlignment headerHAlignment() const { return mHeaderHAlignment; }

    /**
     * Sets the display \a mode for headers in the table. This property controls
     * if and where headers are shown in the table.
     * \see headerMode()
     */
    void setHeaderMode( HeaderMode mode );

    /**
     * Returns the display mode for headers in the table. This property controls
     * if and where headers are shown in the table.
     * \see setHeaderMode()
     */
    HeaderMode headerMode() const { return mHeaderMode; }

    /**
     * Sets the \a font used to draw text in table body cells.
     * \see contentFont()
     * \see setHeaderFont()
     */
    void setContentFont( const QFont &font );

    /**
     * Returns the font used to draw text in table body cells.
     * \see setContentFont()
     * \see headerFont()
     */
    QFont contentFont() const { return mContentFont; }

    /**
     * Sets the \a color used to draw text in table body cells.
     * \see contentFontColor()
     * \see setContentFont()
     * \see setHeaderFontColor()
     */
    void setContentFontColor( const QColor &color );

    /**
     * Returns the color used to draw text in table body cells.
     * \see setContentFontColor()
     * \see contentFont()
     * \see headerFontColor()
     */
    QColor contentFontColor() const { return mContentFontColor; }

    /**
     * Sets whether grid lines should be drawn in the table
     * \param showGrid set to true to show grid lines
     * \see showGrid()
     * \see setGridStrokeWidth()
     * \see setGridColor()
     */
    void setShowGrid( bool showGrid );

    /**
     * Returns whether grid lines are drawn in the table
     * \see setShowGrid()
     * \see gridStrokeWidth()
     * \see gridColor()
     */
    bool showGrid() const { return mShowGrid; }

    /**
     * Sets the \a width in mm for grid lines in the table.
     * \see gridStrokeWidth()
     * \see setShowGrid()
     * \see setGridColor()
     */
    void setGridStrokeWidth( double width );

    /**
     * Returns the width of grid lines in the table in mm.
     * \see setGridStrokeWidth()
     * \see showGrid()
     * \see gridColor()
     */
    double gridStrokeWidth() const { return mGridStrokeWidth; }

    /**
     * Sets the \a color used for grid lines in the table.
     * \see gridColor()
     * \see setShowGrid()
     * \see setGridStrokeWidth()
     */
    void setGridColor( const QColor &color );

    /**
     * Returns the color used for grid lines in the table.
     * \see setGridColor()
     * \see showGrid()
     * \see gridStrokeWidth()
     */
    QColor gridColor() const { return mGridColor; }

    /**
     * Sets whether the grid's horizontal lines should be drawn in the table
     * \param horizontalGrid set to true to draw grid's horizontal lines
     * \see setShowGrid()
     * \see setGridStrokeWidth()
     * \see setGridColor()
     * \see setVerticalGrid()
     */
    void setHorizontalGrid( bool horizontalGrid );

    /**
     * Returns whether the grid's horizontal lines are drawn in the table.
     * \see setShowGrid()
     * \see setGridStrokeWidth()
     * \see setGridColor()
     * \see setVerticalGrid()
     */
    bool horizontalGrid() const { return mHorizontalGrid; }

    /**
     * Sets whether the grid's vertical lines should be drawn in the table
     * \param verticalGrid set to true to draw grid's vertical lines
     * \see setShowGrid()
     * \see setGridStrokeWidth()
     * \see setGridColor()
     * \see setHorizontalGrid()
     */
    void setVerticalGrid( bool verticalGrid );

    /**
     * Returns whether the grid's vertical lines are drawn in the table.
     * \see setShowGrid()
     * \see setGridStrokeWidth()
     * \see setGridColor()
     * \see setHorizontalGrid()
     */
    bool verticalGrid() const { return mVerticalGrid; }

    /**
     * Sets the \a color used for background of table.
     * \see backgroundColor()
     * \see setGridColor()
     */
    void setBackgroundColor( const QColor &color );

    /**
     * Returns the color used for the background of the table.
     * \see setBackgroundColor()
     * \see gridColor()
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the wrap \a behavior for the table, which controls how text within cells is
     * automatically wrapped.
     * \see wrapBehavior()
     */
    void setWrapBehavior( WrapBehavior behavior );

    /**
     * Returns the wrap behavior for the table, which controls how text within cells is
     * automatically wrapped.
     * \see setWrapBehavior()
     */
    WrapBehavior wrapBehavior() const { return mWrapBehavior; }

    /**
     * Returns a reference to the list of QgsLayoutTableColumns shown in the table
     * \see setColumns()
     */
    QgsLayoutTableColumns &columns() { return mColumns; }

    /**
     * Replaces the columns in the table with a specified list of QgsLayoutTableColumns.
     * \param columns list of QgsLayoutTableColumns to show in table.
     * \see columns()
     */
    void setColumns( const QgsLayoutTableColumns &columns );

    /**
     * Sets the cell \a style for a cell \a group.
     * \see cellStyle()
     */
    void setCellStyle( CellStyleGroup group, const QgsLayoutTableStyle &style );

    /**
     * Returns the cell style for a cell \a group.
     * \see setCellStyle()
     */
    const QgsLayoutTableStyle *cellStyle( CellStyleGroup group ) const;

    /**
     * Returns the text used in the column headers for the table.
     * \returns QMap of int to QString, where the int is the column index (starting at 0),
     * and the string is the text to use for the column's header
     * \note not available in Python bindings
     */
    virtual QMap<int, QString> headerLabels() const SIP_SKIP;

    /**
     * Fetches the contents used for the cells in the table.
     * \returns true if table contents were successfully retrieved.
     * \param contents QgsLayoutTableContents to store retrieved row data in
     */
    virtual bool getTableContents( QgsLayoutTableContents &contents ) = 0;

    /**
     * Returns the current contents of the table. Excludes header cells.
     */
    QgsLayoutTableContents &contents() { return mTableContents; }

    QSizeF fixedFrameSize( int frameIndex = -1 ) const override;
    QSizeF minFrameSize( int frameIndex = -1 ) const override;

    bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QSizeF totalSize() const override;
    void render( QgsLayoutItemRenderContext &context, const QRectF &renderExtent, int frameIndex ) override;

  public slots:

    void refresh() override;

    /**
     * Refreshes the contents shown in the table by querying for new data.
     * This also causes the column widths and size of the table to change to accommodate the
     * new data.
     */
    virtual void refreshAttributes();

    void recalculateFrameSizes() override;

  protected:
    //! Margin between cell borders and cell text
    double mCellMargin = 1.0;

    //! Behavior for empty tables
    EmptyTableMode mEmptyTableMode = HeadersOnly;

    //! String to show in empty tables
    QString mEmptyTableMessage;

    //! True if empty rows should be shown in the table
    bool mShowEmptyRows = false;

    //! Header font
    QFont mHeaderFont;

    //! Header font color
    QColor mHeaderFontColor = Qt::black;

    //! Alignment for table headers
    HeaderHAlignment mHeaderHAlignment = FollowColumn;

    //! Header display mode
    HeaderMode mHeaderMode = FirstFrame;

    //! Table contents font
    QFont mContentFont;

    //! Table contents font color
    QColor mContentFontColor = Qt::black;

    //! True if grid should be shown
    bool mShowGrid = true;

    //! Width of grid lines
    double mGridStrokeWidth = 0.5;

    //! Color for grid lines
    QColor mGridColor = Qt::black;

    //! True if grid should be shown
    bool mHorizontalGrid = true;

    //! True if grid should be shown
    bool mVerticalGrid = true;

    //! Color for table background
    QColor mBackgroundColor = Qt::white;

    //! Columns to show in table
    QgsLayoutTableColumns mColumns;

    //! Contents to show in table
    QgsLayoutTableContents mTableContents;

    //! Map of maximum width for each column
    QMap<int, double> mMaxColumnWidthMap;

    //! Map of maximum height for each row
    QMap<int, double> mMaxRowHeightMap;

    QSizeF mTableSize;

    WrapBehavior mWrapBehavior = TruncateText;

    QMap< CellStyleGroup, QgsLayoutTableStyle * > mCellStyles;

    /**
     * Calculates the maximum width of text shown in columns.
     */
    virtual bool calculateMaxColumnWidths();

    /**
     * Calculates the maximum height of text shown in rows.
     */
    virtual bool calculateMaxRowHeights();

    /**
     * Returns total width of table contents.
     * \returns table width
     * \see totalHeight
     */
    //not const, as needs to call calculateMaxColumnWidths()
    double totalWidth();

    /**
     * Returns total height of table contents.
     * \see totalWidth()
     */
    //not const, as needs to call calculateMaxRowHeights()
    double totalHeight();

    /**
     * Calculates how many content rows would be visible within a frame of the specified
     * height.
     * \param frameHeight height of frame
     * \param firstRow index of first row visible in frame (where 0 = first row in table)
     * \param includeHeader set to true if frame would include a header row
     * \param includeEmptyRows set to true to also include rows which would be empty in the returned count. For instance,
     * if the frame would include all table content rows and have space left for extra rows then setting this parameter
     * to true would also include a count of these extra blank rows.
     * \returns number of visible content rows (excluding header row)
     */
    int rowsVisible( double frameHeight, int firstRow, bool includeHeader, bool includeEmptyRows ) const;

    /**
     * Calculates how many content rows are visible within a given frame.
     * \param frameIndex index number for frame
     * \param firstRow index of first row visible in frame (where 0 = first row in table)
     * \param includeEmptyRows set to true to also include rows which would be empty in the returned count. For instance,
     * if the frame would include all table content rows and have space left for extra rows then setting this parameter
     * to true would also include a count of these extra blank rows.
     * \returns number of visible content rows (excludes header rows)
     */
    int rowsVisible( int frameIndex, int firstRow, bool includeEmptyRows ) const;

    /**
     * Calculates a range of rows which should be visible in a given frame.
     * \param frameIndex index number for frame
     * \returns row range
     */
    QPair<int, int> rowRange( int frameIndex ) const;

    /**
     * Draws the horizontal grid lines for the table.
     * \param painter destination painter for grid lines
     * \param firstRow index corresponding to first row shown in frame
     * \param lastRow index corresponding to last row shown in frame. If greater than the number of content rows in the
     * table, then the default row height will be used for the remaining rows.
     * \param drawHeaderLines set to true to include for the table header
     * \see drawVerticalGridLines()
     */
    void drawHorizontalGridLines( QPainter *painter, int firstRow, int lastRow, bool drawHeaderLines ) const;

    /**
     * Draws the vertical grid lines for the table.
     * \param painter destination painter for grid lines
     * \param maxWidthMap QMap of int to double, where the int contains the column number and the double is the
     * maximum width of text present in the column.
     * \param firstRow index corresponding to first row shown in frame
     * \param lastRow index corresponding to last row shown in frame. If greater than the number of content rows in the
     * table, then the default row height will be used for the remaining rows.
     * \param hasHeader set to true if table frame includes header cells
     * \param mergeCells set to true to merge table content cells
     * \note not available in Python bindings
     * \see drawVerticalGridLines()
     * \see calculateMaxColumnWidths()
     * \note not available in Python bindings
     */
    void drawVerticalGridLines( QPainter *painter, const QMap<int, double> &maxWidthMap, int firstRow, int lastRow, bool hasHeader, bool mergeCells = false ) const SIP_SKIP;

    /**
     * Recalculates and updates the size of the table and all table frames.
     */
    void recalculateTableSize();

    /**
     * Checks whether a table contents contains a given row
     * \param contents table contents to check
     * \param row row to check for
     * \returns true if contents contains rows
     */
    bool contentsContainsRow( const QgsLayoutTableContents &contents, const QgsLayoutTableRow &row ) const;

  private:

    QMap< CellStyleGroup, QString > mCellStyleNames;

    //! Initializes cell style map
    void initStyles();

    bool textRequiresWrapping( const QString &text, double columnWidth, const QFont &font ) const;

    QString wrappedText( const QString &value, double columnWidth, const QFont &font ) const;

    /**
     * Returns the calculated background color for a row and column combination.
     * \param row row number, where -1 is the header row, and 0 is the first body row
     * \param column column number, where 0 is the first column
     * \returns background color, or invalid QColor if no background should be drawn
     */
    QColor backgroundColor( int row, int column ) const;

    friend class TestQgsLayoutTable;
    friend class QgsCompositionConverter;
};

#endif // QGSLAYOUTTABLE_H
