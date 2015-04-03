/***************************************************************************
                         qgscomposertable.h
                         ------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERTABLE_H
#define QGSCOMPOSERTABLE_H

#include "qgscomposeritem.h"
#include "qgscomposition.h"
#include "qgsfeature.h"
#include <QSet>
#include <QObject>

class QgsComposerTableColumn;

/**A class to display feature attributes in the print composer*/
class CORE_EXPORT QgsComposerTable: public QgsComposerItem
{
    Q_OBJECT

  public:

    /*! Controls how headers are horizontally aligned in a table
     */
    enum HeaderHAlignment
    {
      FollowColumn, /*!< header uses the same alignment as the column */
      HeaderLeft, /*!< align headers left */
      HeaderCenter, /*!< align headers to center */
      HeaderRight /*!< align headers right */
    };

    QgsComposerTable( QgsComposition* composition );
    virtual ~QgsComposerTable();

    /** return correct graphics item type. */
    virtual int type() const override { return ComposerTable; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const override = 0;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override = 0;

    /**Sets the margin distance between cell borders and their contents.
     * @param d margin for cell contents
     * @see lineTextDistance
     */
    void setLineTextDistance( double d );

    /**Returns the margin distance between cell borders and their contents.
     * @returns margin for cell contents
     * @see setLineTextDistance
     */
    double lineTextDistance() const { return mLineTextDistance; }

    /**Sets the font used to draw header text in the table.
     * @param f font for header cells
     * @see headerFont
     * @see setContentFont
     */
    void setHeaderFont( const QFont& f );

    /**Returns the font used to draw header text in the table.
     * @returns font for header cells
     * @see setHeaderFont
     * @see contentFont
     */
    QFont headerFont() const { return mHeaderFont; }

    /**Sets the color used to draw header text in the table.
     * @param color header text color
     * @see headerFontColor
     * @see setHeaderFont
     * @see setContentFontColor
     * @note added in 2.5
     */
    void setHeaderFontColor( const QColor& color );

    /**Returns the color used to draw header text in the table.
     * @returns color for header text
     * @see setHeaderFontColor
     * @see headerFont
     * @see contentFontColor
     * @note added in 2.5
     */
    QColor headerFontColor() const { return mHeaderFontColor; }

    /**Sets the horizontal alignment for table headers
     * @param alignment Horizontal alignment for table header cells
     * @note added in 2.3
     * @see headerHAlignment
     */
    void setHeaderHAlignment( const HeaderHAlignment alignment );

    /**Returns the horizontal alignment for table headers
     * @returns Horizontal alignment for table header cells
     * @note added in 2.3
     * @see setHeaderHAlignment
     */
    HeaderHAlignment headerHAlignment() const { return mHeaderHAlignment; }

    /**Sets the font used to draw text in table body cells.
     * @param f font for table cells
     * @see contentFont
     * @see setHeaderFont
     */
    void setContentFont( const QFont& f );

    /**Returns the font used to draw text in table body cells.
     * @returns font for table cells
     * @see setContentFont
     * @see headerFont
     */
    QFont contentFont() const { return mContentFont; }

    /**Sets the color used to draw text in table body cells.
     * @param color table cell text color
     * @see contentFontColor
     * @see setContentFont
     * @see setHeaderFontColor
     * @note added in 2.5
     */
    void setContentFontColor( const QColor& color );

    /**Returns the color used to draw text in table body cells.
     * @returns text color for table cells
     * @see setContentFontColor
     * @see contentFont
     * @see headerFontColor
     * @note added in 2.5
     */
    QColor contentFontColor() const { return mContentFontColor; }

    /**Sets whether grid lines should be drawn in the table
     * @param show set to true to show grid lines
     * @see showGrid
     * @see setGridStrokeWidth
     * @see setGridColor
     */
    void setShowGrid( bool show );

    /**Returns whether grid lines are drawn in the table
     * @returns true if grid lines are shown
     * @see setShowGrid
     * @see gridStrokeWidth
     * @see gridColor
     */
    bool showGrid() const { return mShowGrid; }

    /**Sets the width for grid lines in the table.
     * @param w grid line width
     * @see gridStrokeWidth
     * @see setShowGrid
     * @see setGridColor
     */
    void setGridStrokeWidth( double w );

    /**Returns the width of grid lines in the table.
     * @returns grid line width
     * @see setGridStrokeWidth
     * @see showGrid
     * @see gridColor
     */
    double gridStrokeWidth() const { return mGridStrokeWidth; }

    /**Sets color used for grid lines in the table.
     * @param c grid line color
     * @see gridColor
     * @see setShowGrid
     * @see setGridStrokeWidth
     */
    void setGridColor( const QColor& c ) { mGridColor = c; }

    /**Returns the color used for grid lines in the table.
     * @returns grid line color
     * @see setGridColor
     * @see showGrid
     * @see gridStrokeWidth
     */
    QColor gridColor() const { return mGridColor; }

    /**Returns the text used in the column headers for the table.
     * @returns QMap of int to QString, where the int is the column index (starting at 0),
     * and the string is the text to use for the column's header
     * @note added in 2.3
     * @note not available in python bindings
     */
    virtual QMap<int, QString> headerLabels() const;

    //TODO - make this more generic for next API break, eg rename as getRowValues, use QStringList rather than
    //QgsAttributeMap

    /**Fetches the text used for the rows of the table.
     * @returns true if attribute text was successfully retrieved.
     * @param attributeMaps QList of QgsAttributeMap to store retrieved row data in
     * @note not available in python bindings
     */
    virtual bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps ) { Q_UNUSED( attributeMaps ); return false; }

    /**Returns a pointer to the list of QgsComposerTableColumns shown in the table
     * @returns pointer to list of columns in table
     * @note added in 2.3
     * @see setColumns
     */
    QList<QgsComposerTableColumn*>* columns() { return &mColumns; }

    /**Replaces the columns in the table with a specified list of QgsComposerTableColumns.
     * @param columns list of QgsComposerTableColumns to show in table
     * @note added in 2.3
     * @see columns
     */
    void setColumns( QList<QgsComposerTableColumn*> columns );

  public slots:

    /**Refreshes the attributes shown in the table by querying the vector layer for new data.
     * This also causes the column widths and size of the table to change to accommodate the
     * new data.
     * @note added in 2.3
     * @see adjustFrameToSize
    */
    virtual void refreshAttributes();

    /**Adapts the size of the frame to match the content. First, the optimal width of the columns
     * is recalculated by checking the maximum width of attributes shown in the table. Then, the
     * table is resized to fit its contents. This slot utilises the table's attribute cache so
     * that a re-query of the vector layer is not required.
     * @note added in 2.3
     * @see refreshAttributes
    */
    virtual void adjustFrameToSize();

  protected:
    /**Distance between table lines and text*/
    double mLineTextDistance;

    QFont mHeaderFont;
    QColor mHeaderFontColor;
    QFont mContentFont;
    QColor mContentFontColor;

    HeaderHAlignment mHeaderHAlignment;

    bool mShowGrid;
    double mGridStrokeWidth;
    QColor mGridColor;

    QList<QgsAttributeMap> mAttributeMaps;
    QMap<int, double> mMaxColumnWidthMap;

    QList<QgsComposerTableColumn*> mColumns;

    /**Calculates the maximum width of text shown in columns.
     * @param maxWidthMap QMap of int to double in which to store the maximum widths. The int will be filled
     * with the column number and the double with the maximum width of text present in the column.
     * @param attributeMaps list of attribute values for each row shown in the table
     * @note not available in python bindings
     * @see adaptItemFrame
     */
    virtual bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps ) const;

    /**Adapts the size of the item frame to match the table's content.
     * @param maxWidthMap QMap of int to double, where the int contains the column number and the double is the
     * maximum width of text present in the column.
     * @param attributeMaps list of attribute values for each row shown in the table
     * @note not available in python bindings
     * @see calculateMaxColumnWidths
     */
    void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps );

    /**Draws the horizontal grid lines for the table.
     * @param p destination painter for grid lines
     * @param nAttributes number of attribute rows shown in table
     * @see drawVerticalGridLines
     */
    void drawHorizontalGridLines( QPainter* p, int nAttributes );

    /**Draws the vertical grid lines for the table.
     * @param p destination painter for grid lines
     * @param maxWidthMap QMap of int to double, where the int contains the column number and the double is the
     * maximum width of text present in the column.
     * @note not available in python bindings
     * @see drawVerticalGridLines
     * @see calculateMaxColumnWidths
     */
    void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );

    /**Writes common table properties to xml for storage.
     * @param itemElem an existing QDomElement in which to store the table's properties.
     * @param doc QDomDocument for the destination xml.
     * @see tableReadXML
     * @see writeXML
     */
    bool tableWriteXML( QDomElement& itemElem, QDomDocument& doc ) const;

    /**Reads the table's common properties from xml.
     * @param itemElem a QDomElement holding the table's desired properties.
     * @param doc QDomDocument for the source xml.
     * @see tableWriteXML
     * @see readXML
     */
    bool tableReadXML( const QDomElement& itemElem, const QDomDocument& doc );
};

#endif // QGSCOMPOSERTABLE_H
