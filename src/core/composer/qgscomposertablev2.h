/***************************************************************************
                         qgscomposertablev2.h
                         ------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson, Marco Hugentobler
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

#ifndef QGSCOMPOSERTABLEV2_H
#define QGSCOMPOSERTABLEV2_H

#include "qgscomposermultiframe.h"
#include <QFont>
#include <QColor>
#include <QPair>

class QgsComposerTableColumn;

/** List of QVariants, representing a the contents of a single row in
 * a QgsComposerTable
 * \note Added in version 2.5
*/
typedef QList< QVariant > QgsComposerTableRow;

/** List of QgsComposerTableRows, representing rows and column cell contents
 * for a QgsComposerTable
 * \note Added in version 2.5
*/
typedef QList< QgsComposerTableRow > QgsComposerTableContents;

/** List of column definitions for a QgsComposerTable
 * \note Added in version 2.5
*/
typedef QList<QgsComposerTableColumn*> QgsComposerTableColumns;

/** A class to display a table in the print composer, and allow
 * the table to span over multiple frames
 * @note added in QGIS 2.5
 */
class CORE_EXPORT QgsComposerTableV2: public QgsComposerMultiFrame
{
    Q_OBJECT

  public:

    /** Controls how headers are horizontally aligned in a table
     */
    enum HeaderHAlignment
    {
      FollowColumn, /*!< header uses the same alignment as the column */
      HeaderLeft, /*!< align headers left */
      HeaderCenter, /*!< align headers to center */
      HeaderRight /*!< align headers right */
    };

    /** Controls where headers are shown in the table
     */
    enum HeaderMode
    {
      FirstFrame = 0, /*!< header shown on first frame only */
      AllFrames, /*!< headers shown on all frames */
      NoHeaders /*!< no headers shown for table */
    };

    /** Controls how empty tables are displayed
     */
    enum EmptyTableMode
    {
      HeadersOnly = 0, /*!< show header rows only */
      HideTable, /*!< hides entire table if empty */
      ShowMessage /*!< shows preset message instead of table contents*/
    };

    QgsComposerTableV2( QgsComposition* composition, bool createUndoCommands );
    QgsComposerTableV2();

    virtual ~QgsComposerTableV2();

    /** Sets the margin distance between cell borders and their contents.
     * @param margin margin for cell contents
     * @see cellMargin
     */
    void setCellMargin( const double margin );

    /** Returns the margin distance between cell borders and their contents.
     * @returns margin for cell contents
     * @see setCellMargin
     */
    double cellMargin() const { return mCellMargin; }

    /** Sets the behaviour for empty tables with no content rows.
     * @param mode behaviour mode for empty tables
     * @see emptyTableBehaviour
     */
    void setEmptyTableBehaviour( const EmptyTableMode mode );

    /** Returns the behaviour mode for empty tables. This property controls
     * how the table is drawn if it contains no content rows.
     * @returns behaviour mode for empty tables
     * @see setEmptyTableBehaviour
     */
    EmptyTableMode emptyTableBehaviour() const { return mEmptyTableMode; }

    /** Sets the message for empty tables with no content rows. This message
     * is displayed in the table body if the empty table behaviour is
     * set to ShowMessage
     * @param message message to show for empty tables
     * @see emptyTableMessage
     * @see setEmptyTableBehaviour
     */
    void setEmptyTableMessage( const QString message );

    /** Returns the message for empty tables with no content rows. This message
     * is displayed in the table body if the empty table behaviour is
     * set to ShowMessage
     * @returns message to show for empty tables
     * @see setEmptyTableMessage
     * @see emptyTableBehaviour
     */
    QString emptyTableMessage() const { return mEmptyTableMessage; }

    /** Sets whether empty rows should be drawn. Tables default to hiding empty rows.
     * @param showEmpty set to true to show empty rows in the table
     * @see showEmptyRows
     */
    void setShowEmptyRows( const bool showEmpty );

    /** Returns whether empty rows are drawn in the table
     * @returns true if empty rows are drawn
     * @see setShowEmptyRows
     */
    bool showEmptyRows() const { return mShowEmptyRows; }

    /** Sets the font used to draw header text in the table.
     * @param font font for header cells
     * @see headerFont
     * @see setContentFont
     */
    void setHeaderFont( const QFont& font );

    /** Returns the font used to draw header text in the table.
     * @returns font for header cells
     * @see setHeaderFont
     * @see contentFont
     */
    QFont headerFont() const { return mHeaderFont; }

    /** Sets the color used to draw header text in the table.
     * @param color header text color
     * @see headerFontColor
     * @see setHeaderFont
     * @see setContentFontColor
     */
    void setHeaderFontColor( const QColor& color );

    /** Returns the color used to draw header text in the table.
     * @returns color for header text
     * @see setHeaderFontColor
     * @see headerFont
     * @see contentFontColor
     */
    QColor headerFontColor() const { return mHeaderFontColor; }

    /** Sets the horizontal alignment for table headers
     * @param alignment Horizontal alignment for table header cells
     * @see headerHAlignment
     */
    void setHeaderHAlignment( const HeaderHAlignment alignment );

    /** Returns the horizontal alignment for table headers
     * @returns Horizontal alignment for table header cells
     * @see setHeaderHAlignment
     */
    HeaderHAlignment headerHAlignment() const { return mHeaderHAlignment; }

    /** Sets the display mode for headers in the table. This property controls
     * if and where headers are shown in the table.
     * @param mode display mode for headers
     * @see headerMode
     */
    void setHeaderMode( const HeaderMode mode );

    /** Returns the display mode for headers in the table. This property controls
     * if and where headers are shown in the table.
     * @returns display mode for headers
     * @see setHeaderMode
     */
    HeaderMode headerMode() const { return mHeaderMode; }

    /** Sets the font used to draw text in table body cells.
     * @param font font for table cells
     * @see contentFont
     * @see setHeaderFont
     */
    void setContentFont( const QFont& font );

    /** Returns the font used to draw text in table body cells.
     * @returns font for table cells
     * @see setContentFont
     * @see headerFont
     */
    QFont contentFont() const { return mContentFont; }

    /** Sets the color used to draw text in table body cells.
     * @param color table cell text color
     * @see contentFontColor
     * @see setContentFont
     * @see setHeaderFontColor
     */
    void setContentFontColor( const QColor& color );

    /** Returns the color used to draw text in table body cells.
     * @returns text color for table cells
     * @see setContentFontColor
     * @see contentFont
     * @see headerFontColor
     */
    QColor contentFontColor() const { return mContentFontColor; }

    /** Sets whether grid lines should be drawn in the table
     * @param showGrid set to true to show grid lines
     * @see showGrid
     * @see setGridStrokeWidth
     * @see setGridColor
     */
    void setShowGrid( const bool showGrid );

    /** Returns whether grid lines are drawn in the table
     * @returns true if grid lines are shown
     * @see setShowGrid
     * @see gridStrokeWidth
     * @see gridColor
     */
    bool showGrid() const { return mShowGrid; }

    /** Sets the width for grid lines in the table.
     * @param width grid line width
     * @see gridStrokeWidth
     * @see setShowGrid
     * @see setGridColor
     */
    void setGridStrokeWidth( const double width );

    /** Returns the width of grid lines in the table.
     * @returns grid line width
     * @see setGridStrokeWidth
     * @see showGrid
     * @see gridColor
     */
    double gridStrokeWidth() const { return mGridStrokeWidth; }

    /** Sets color used for grid lines in the table.
     * @param color grid line color
     * @see gridColor
     * @see setShowGrid
     * @see setGridStrokeWidth
     */
    void setGridColor( const QColor& color );

    /** Returns the color used for grid lines in the table.
     * @returns grid line color
     * @see setGridColor
     * @see showGrid
     * @see gridStrokeWidth
     */
    QColor gridColor() const { return mGridColor; }

    /** Sets color used for background of table.
     * @param color table background color
     * @see backgroundColor
     * @see setGridColor
     */
    void setBackgroundColor( const QColor& color );

    /** Returns the color used for the background of the table.
     * @returns table background color
     * @see setBackgroundColor
     * @see gridColor
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /** Returns a pointer to the list of QgsComposerTableColumns shown in the table
     * @returns pointer to list of columns in table
     * @see setColumns
     */
    QgsComposerTableColumns* columns() { return &mColumns; }

    /** Replaces the columns in the table with a specified list of QgsComposerTableColumns.
     * @param columns list of QgsComposerTableColumns to show in table
     * @see columns
     */
    void setColumns( QgsComposerTableColumns columns );

    /** Returns the text used in the column headers for the table.
     * @returns QMap of int to QString, where the int is the column index (starting at 0),
     * and the string is the text to use for the column's header
     * @note not available in python bindings
     */
    virtual QMap<int, QString> headerLabels() const;

    /** Fetches the contents used for the cells in the table.
     * @returns true if table contents were successfully retrieved.
     * @param contents QgsComposerTableContents to store retrieved row data in
     * @note not available in python bindings
     */
    virtual bool getTableContents( QgsComposerTableContents &contents ) = 0;

    /** Returns the current contents of the table. Excludes header cells.
     * @returns table contents
     */
    QgsComposerTableContents* contents() { return &mTableContents; }

    //reimplemented to return fixed table width
    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const override;

    //reimplemented to return min frame height
    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const override;

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const override;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false ) override;
    virtual QSizeF totalSize() const override;
    virtual void render( QPainter* p, const QRectF& renderExtent, const int frameIndex ) override;

  public slots:

    /** Refreshes the contents shown in the table by querying for new data.
     * This also causes the column widths and size of the table to change to accommodate the
     * new data.
     * @see adjustFrameToSize
    */
    virtual void refreshAttributes();

    void recalculateFrameSizes() override;

  protected:
    /** Margin between cell borders and cell text*/
    double mCellMargin;

    /** Behaviour for empty tables*/
    EmptyTableMode mEmptyTableMode;

    /** String to show in empty tables*/
    QString mEmptyTableMessage;

    /** True if empty rows should be shown in the table*/
    bool mShowEmptyRows;

    /** Header font*/
    QFont mHeaderFont;

    /** Header font color*/
    QColor mHeaderFontColor;

    /** Alignment for table headers*/
    HeaderHAlignment mHeaderHAlignment;

    /** Header display mode*/
    HeaderMode mHeaderMode;

    /** Table contents font*/
    QFont mContentFont;

    /** Table contents font color*/
    QColor mContentFontColor;

    /** True if grid should be shown*/
    bool mShowGrid;

    /** Width of grid lines*/
    double mGridStrokeWidth;

    /** Color for grid lines*/
    QColor mGridColor;

    /** Color for table background*/
    QColor mBackgroundColor;

    /** Columns to show in table*/
    QgsComposerTableColumns mColumns;

    /** Contents to show in table*/
    QgsComposerTableContents mTableContents;

    /** Map of maximum width for each column*/
    QMap<int, double> mMaxColumnWidthMap;

    QSizeF mTableSize;

    /** Calculates the maximum width of text shown in columns.
     */
    virtual bool calculateMaxColumnWidths();

    /** Returns total width of table contents.
     * @returns table width
     * @see totalHeight
     */
    //not const, as needs to call calculateMaxColumnWidths()
    double totalWidth();

    /** Returns total height of table contents.
     * @returns total height
     * @see totalWidth
     */
    double totalHeight() const;

    /** Calculates how many content rows are visible within a given frame
     * @param frameIndex index number for frame
     * @returns number of visible content rows (excludes header rows)
     */
    int rowsVisible( const int frameIndex ) const;

    /** Calculates how many content rows would be visible within a specified
     * height.
     * @param frameHeight height of frame
     * @param includeHeader set to true if frame would include a header row
     * @returns number of visible content rows (excluding header row)
     */
    int rowsVisible( const double frameHeight, const bool includeHeader ) const;

    /** Calculates a range of rows which should be visible in a given
     * frame extent.
     * @param extent visible extent
     * @param frameIndex index number for frame
     * @returns row range
     */
    QPair<int, int> rowRange( const QRectF &extent, const int frameIndex ) const;

    /** Draws the horizontal grid lines for the table.
     * @param painter destination painter for grid lines
     * @param rows number of rows shown in table
     * @param drawHeaderLines set to true to include for the table header
     * @see drawVerticalGridLines
     */
    void drawHorizontalGridLines( QPainter* painter, const int rows, const bool drawHeaderLines ) const;

    /** Draws the vertical grid lines for the table.
     * @param painter destination painter for grid lines
     * @param maxWidthMap QMap of int to double, where the int contains the column number and the double is the
     * maximum width of text present in the column.
     * @param numberRows number of rows of content in table frame
     * @param hasHeader set to true if table frame includes header cells
     * @param mergeCells set to true to merge table content cells
     * @note not available in python bindings
     * @see drawVerticalGridLines
     * @see calculateMaxColumnWidths
     * @note not available in python bindings
     */
    void drawVerticalGridLines( QPainter* painter, const QMap<int, double>& maxWidthMap, const int numberRows, const bool hasHeader, const bool mergeCells = false ) const;

    /** Recalculates and updates the size of the table and all table frames.
     */
    void recalculateTableSize();

    /** Checks whether a table contents contains a given row
     * @param contents table contents to check
     * @param row row to check for
     * @returns true if contents contains rows
     */
    bool contentsContainsRow( const QgsComposerTableContents &contents, const QgsComposerTableRow &row ) const;

    friend class TestQgsComposerTableV2;
};

#endif // QGSCOMPOSERTABLEV2_H
