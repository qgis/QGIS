/***************************************************************************
                         qgslayoutitemmanualtable.h
                         ---------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSLAYOUTITEMMANUALTABLE_H
#define QGSLAYOUTITEMMANUALTABLE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayouttable.h"
#include "qgstablecell.h"

/**
 * \ingroup core
 * \brief A layout table subclass that displays manually entered (and formatted) content.
 * \since QGIS 3.12
*/
class CORE_EXPORT QgsLayoutItemManualTable: public QgsLayoutTable
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemManualTable, attached to the specified \a layout.
     *
     * Ownership is transferred to the layout.
     */
    QgsLayoutItemManualTable( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutItemManualTable() override;

    int type() const override;
    QIcon icon() const override;
    QString displayName() const override;

    /**
     * Returns a new QgsLayoutItemManualTable for the specified parent \a layout.
     */
    static QgsLayoutItemManualTable *create( QgsLayout *layout ) SIP_FACTORY;
    bool getTableContents( QgsLayoutTableContents &contents ) override SIP_SKIP;
    QgsConditionalStyle conditionalCellStyle( int row, int column ) const override;

    /**
     * Sets the \a contents of the table.
     *
     * \see tableContents()
     */
    void setTableContents( const QgsTableContents &contents );

    /**
     * Returns the contents of the table.
     *
     * \see contents()
     */
    QgsTableContents tableContents() const;

    /**
     * Returns the list of row heights (in millimeters) to use when rendering the table.
     *
     * A height of 0 indicates that the row height should be automatically calculated.
     *
     * \see setRowHeights()
     * \see columnWidths()
     */
    QList< double > rowHeights() const { return mRowHeights; }

    /**
     * Sets the list of row \a heights (in millimeters) to use when rendering the table.
     *
     * A height of 0 indicates that the row height should be automatically calculated.
     *
     * \see rowHeights()
     * \see setColumnWidths()
     */
    void setRowHeights( const QList< double > &heights );

    /**
     * Returns the list of column widths (in millimeters) to use when rendering the table.
     *
     * A width of 0 indicates that the column width should be automatically calculated.
     *
     * \see setColumnWidths()
     * \see rowHeights()
     */
    QList< double > columnWidths() const { return mColumnWidths; }

    /**
     * Sets the list of column \a widths (in millimeters) to use when rendering the table.
     *
     * A width of 0 indicates that the column width should be automatically calculated.
     *
     * \see columnWidths()
     * \see setColumnWidths()
     */
    void setColumnWidths( const QList< double > &widths );

    /**
     * Returns TRUE if the table includes a header row.
     *
     * \see setIncludeTableHeader()
     */
    bool includeTableHeader() const;

    /**
     * Sets whether the table includes a header row.
     *
     * \see includeTableHeader()
     */
    void setIncludeTableHeader( bool included );

    /**
     * Returns a reference to the list of headers shown in the table
     * \see setHeaders()
     */
    QgsLayoutTableColumns &headers();

    /**
     * Replaces the headers in the table with a specified list of QgsLayoutTableColumns.
     * \see headers()
     */
    void setHeaders( const QgsLayoutTableColumns &headers );

  protected:

    bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool calculateMaxRowHeights() override;
    QgsTextFormat textFormatForHeader( int column ) const override;
    QgsTextFormat textFormatForCell( int row, int column ) const override;
    Qt::Alignment horizontalAlignmentForCell( int row, int column ) const override;
    Qt::Alignment verticalAlignmentForCell( int row, int column ) const override;
    int rowSpan( int row, int column ) const override;
    int columnSpan( int row, int column ) const override;

  private:

    QgsTableContents mContents;
    QgsLayoutTableColumns mHeaders;

    QList< double > mRowHeights;
    QList< double > mColumnWidths;
    bool mIncludeHeader = false;

    void refreshColumns();

};

#endif // QGSLAYOUTITEMMANUALTABLE_H
