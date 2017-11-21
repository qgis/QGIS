/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayouttablecolumn.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutTableColumn
{
%Docstring
 Stores properties of a column for a QgsLayoutTable. Some properties of aQgsLayoutTableColumn
 are applicable only in certain contexts. For instance, the attribute and setAttribute methods only
 have an effect for QgsLayoutItemAttributeTables, and have no effect for QgsLayoutItemTextTables.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayouttablecolumn.h"
%End
  public:

    QgsLayoutTableColumn( const QString &heading = QString() );
%Docstring
 Constructor for QgsLayoutTableColumn.
 \param heading column heading
%End

    bool writeXml( QDomElement &columnElem, QDomDocument &doc ) const;
%Docstring
 Writes the column's properties to xml for storage.
 \param columnElem an existing QDomElement in which to store the column's properties.
 \param doc QDomDocument for the destination xml.
.. seealso:: readXml()
 :rtype: bool
%End

    bool readXml( const QDomElement &columnElem );
%Docstring
 Reads the column's properties from xml.
 \param columnElem a QDomElement holding the column's desired properties.
.. seealso:: writeXml()
 :rtype: bool
%End

    double width() const;
%Docstring
 Returns the width for the column in mm,
 or 0 if column width is automatically calculated.
.. seealso:: setWidth()
 :rtype: float
%End

    void setWidth( const double width );
%Docstring
 Sets the width for a column in mm. Set the ``width`` to 0 if the column width is to be automatically calculated.
.. seealso:: width()
%End

    QString heading() const;
%Docstring
 Returns the heading for a column, which is the value displayed in the column's
 header cell.
.. seealso:: setHeading()
 :rtype: str
%End

    void setHeading( const QString &heading );
%Docstring
 Sets the ``heading`` for a column, which is the value displayed in the column's
 header cell.
.. seealso:: heading()
%End

    Qt::AlignmentFlag hAlignment() const;
%Docstring
 Returns the horizontal alignment for a column, which controls the alignment
 used for drawing column values within cells.
.. seealso:: setHAlignment()
.. seealso:: vAlignment()
 :rtype: Qt.AlignmentFlag
%End

    void setHAlignment( Qt::AlignmentFlag alignment );
%Docstring
 Sets the horizontal ``alignment`` for a column, which controls the alignment
 used for drawing column values within cells.
.. seealso:: hAlignment()
.. seealso:: setVAlignment()
%End

    Qt::AlignmentFlag vAlignment() const;
%Docstring
 Returns the vertical alignment for a column, which controls the alignment
 used for drawing column values within cells.
.. seealso:: setVAlignment()
.. seealso:: hAlignment()
 :rtype: Qt.AlignmentFlag
%End

    void setVAlignment( Qt::AlignmentFlag alignment );
%Docstring
 Sets the vertical ``alignment`` for a column, which controls the alignment
 used for drawing column values within cells.
.. seealso:: vAlignment()
.. seealso:: setHAlignment()
%End

    QString attribute() const;
%Docstring
 Returns the attribute name or expression used for the column's values. This property
 is only used when the column is part of a QgsLayoutItemAttributeTable.
.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: setAttribute()
 :rtype: str
%End

    void setAttribute( const QString &attribute );
%Docstring
 Sets the ``attribute`` name or expression used for the column's values. This property
 is only used when the column is part of a QgsLayoutItemAttributeTable.
.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: attribute()
%End

    Qt::SortOrder sortOrder() const;
%Docstring
 Returns the sort order for the column. This property is only used when the column
 is part of a QgsLayoutItemAttributeTable and when sortByRank is > 0.
.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: setSortOrder()
.. seealso:: sortByRank()
 :rtype: Qt.SortOrder
%End

    void setSortOrder( Qt::SortOrder order );
%Docstring
 Sets the sort ``order`` for the column. This property is only used when the column
 is part of a QgsLayoutItemAttributeTable and when sortByRank() is > 0.
.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: sortOrder()
.. seealso:: setSortByRank()
%End

    int sortByRank() const;
%Docstring
 Returns the sort rank for the column. If the sort rank is > 0 then the column
 will be sorted in the table. The sort rank specifies the priority given to the
 column when the table is sorted by multiple columns, with lower sort ranks
 having higher priority. This property is only used when the column
 is part of a QgsLayoutItemAttributeTable.

 If sort rank is <= 0 then the column is not being sorted.

.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: setSortByRank()
.. seealso:: sortOrder()
 :rtype: int
%End

    void setSortByRank( int rank );
%Docstring
 Sets the sort ``rank`` for the column. If the sort rank is > 0 then the column
 will be sorted in the table. The sort rank specifies the priority given to the
 column when the table is sorted by multiple columns, with lower sort ranks
 having higher priority. This property is only used when the column
 is part of a QgsComposerAttributeTable.
 If the sort ``rank`` is <= 0 then the column is not being sorted.

.. note::

   only applicable when used in a QgsLayoutItemAttributeTable
.. seealso:: sortByRank()
.. seealso:: setSortOrder()
%End

    QgsLayoutTableColumn *clone() /Factory/;
%Docstring
 Creates a duplicate column which is a deep copy of this column.
 :return: a new QgsLayoutTableColumn with same properties as this column.
 :rtype: QgsLayoutTableColumn
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayouttablecolumn.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
