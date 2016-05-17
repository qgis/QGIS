/** Stores properties of a column in a QgsComposerTable. Some properties of a QgsComposerTableColumn
are applicable only in certain contexts. For instance, the attribute and setAttribute methods only
have an effect for QgsComposerAttributeTables, and have no effect for QgsComposerTextTables.*/
class QgsComposerTableColumn: QObject
{
%TypeHeaderCode
#include <qgscomposertablecolumn.h>
%End
  public:

    /** Constructor for QgsComposerTableColumn.
     * @param heading column heading
     */
    QgsComposerTableColumn( const QString& heading = QString() );

    virtual ~QgsComposerTableColumn();

    /** Writes the column's properties to xml for storage.
     * @param columnElem an existing QDomElement in which to store the column's properties.
     * @param doc QDomDocument for the destination xml.
     * @note added in 2.3
     * @see readXML
     */
    virtual bool writeXML( QDomElement& columnElem, QDomDocument & doc ) const;

    /** Reads the column's properties from xml.
     * @param columnElem a QDomElement holding the column's desired properties.
     * @note added in 2.3
     * @see writeXML
     */
    virtual bool readXML( const QDomElement& columnElem );

    /** Returns the width for a column.
     * @returns column width in mm, or 0 if column width is automatically calculated.
     * @note added in 2.5
     * @see setWidth
     */
    double width() const;

    /** Sets the width for a column.
     * @param width column width in mm, or 0 if column width is to be automatically calculated.
     * @note added in 2.5
     * @see width
     */
    void setWidth( const double width );

    /** Returns the heading for a column, which is the value displayed in the columns
     * header cell.
     * @returns Heading for column.
     * @note added in 2.3
     * @see setHeading
     */
    QString heading() const;

    /** Sets the heading for a column, which is the value displayed in the columns
     * header cell.
     * @param heading Heading for column.
     * @note added in 2.3
     * @see heading
     */
    void setHeading( const QString& heading );

    /** Returns the horizontal alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @returns horizontal alignment.
     * @note added in 2.3
     * @see setHAlignment
     * @see vAlignment
     */
    Qt::AlignmentFlag hAlignment() const;

    /** Sets the horizontal alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @param alignment horizontal alignment for cell.
     * @note added in 2.3
     * @see hAlignment
     * @see setVAlignment
     */
    void setHAlignment( Qt::AlignmentFlag alignment );

    /** Returns the vertical alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @returns vertical alignment.
     * @note added in 2.12
     * @see setVAlignment
     * @see hAlignment
     */
    Qt::AlignmentFlag vAlignment() const;

    /** Sets the vertical alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @param alignment vertical alignment for cell.
     * @note added in 2.12
     * @see vAlignment
     * @see setHAlignment
     */
    void setVAlignment( Qt::AlignmentFlag alignment );

    /** Returns the attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsComposerAttributeTable.
     * @returns attribute name or expression text for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setAttribute
     */
    QString attribute() const;

    /** Sets the attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsComposerAttributeTable.
     * @param attribute attribute name or expression text for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see attribute
     */
    void setAttribute( const QString& attribute );

    /** Returns the sort order for the column. This property is only used when the column
     * is part of a QgsComposerAttributeTable and when sortByRank is > 0.
     * @returns sort order for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setSortOrder
     * @see sortByRank
     */
    Qt::SortOrder sortOrder() const;

    /** Sets the sort order for the column. This property is only used when the column
     * is part of a QgsComposerAttributeTable and when sortByRank is > 0.
     * @param sortOrder sort order for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see sortOrder
     * @see setSortByRank
     */
    void setSortOrder( Qt::SortOrder sortOrder );

    /** Returns the sort rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsComposerAttributeTable.
     * @returns sort rank for column. If sort rank is <= 0 then the column is not being
     * sorted.
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setSortByRank
     * @see sortOrder
     */
    int sortByRank() const;

    /** Sets the sort rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsComposerAttributeTable.
     * @param sortByRank sort rank for column. If sort rank is <= 0 then the column is not being
     * sorted.
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see sortByRank
     * @see setSortOrder
     */
    void setSortByRank( int sortByRank );

    /** Creates a duplicate column which is a deep copy of this column.
     * @returns a new QgsComposerTableColumn with same properties as this column.
     * @note added in 2.3
     */
    QgsComposerTableColumn* clone();
};
