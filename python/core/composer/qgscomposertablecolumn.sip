/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposertablecolumn.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerTableColumn: QObject
{
%Docstring
Stores properties of a column in a QgsComposerTable. Some properties of a QgsComposerTableColumn
are applicable only in certain contexts. For instance, the attribute and setAttribute methods only
have an effect for QgsComposerAttributeTables, and have no effect for QgsComposerTextTables.*
%End

%TypeHeaderCode
#include "qgscomposertablecolumn.h"
%End
  public:

    QgsComposerTableColumn( const QString &heading = QString() );
%Docstring
Constructor for QgsComposerTableColumn.

:param heading: column heading
%End

    virtual bool writeXml( QDomElement &columnElem, QDomDocument &doc ) const;
%Docstring
Writes the column's properties to xml for storage.

:param columnElem: an existing QDomElement in which to store the column's properties.
:param doc: QDomDocument for the destination xml.

.. versionadded:: 2.3

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &columnElem );
%Docstring
Reads the column's properties from xml.

:param columnElem: a QDomElement holding the column's desired properties.

.. versionadded:: 2.3

.. seealso:: :py:func:`writeXml`
%End

    double width() const;
%Docstring
Returns the width for a column.

:return: column width in mm, or 0 if column width is automatically calculated.

.. versionadded:: 2.5

.. seealso:: :py:func:`setWidth`
%End

    void setWidth( const double width );
%Docstring
Sets the width for a column.

:param width: column width in mm, or 0 if column width is to be automatically calculated.

.. versionadded:: 2.5

.. seealso:: :py:func:`width`
%End

    QString heading() const;
%Docstring
Returns the heading for a column, which is the value displayed in the columns
header cell.

:return: Heading for column.

.. versionadded:: 2.3

.. seealso:: :py:func:`setHeading`
%End

    void setHeading( const QString &heading );
%Docstring
Sets the heading for a column, which is the value displayed in the columns
header cell.

:param heading: Heading for column.

.. versionadded:: 2.3

.. seealso:: :py:func:`heading`
%End

    Qt::AlignmentFlag hAlignment() const;
%Docstring
Returns the horizontal alignment for a column, which controls the alignment
used for drawing column values within cells.

:return: horizontal alignment.

.. versionadded:: 2.3

.. seealso:: :py:func:`setHAlignment`

.. seealso:: :py:func:`vAlignment`
%End

    void setHAlignment( Qt::AlignmentFlag alignment );
%Docstring
Sets the horizontal alignment for a column, which controls the alignment
used for drawing column values within cells.

:param alignment: horizontal alignment for cell.

.. versionadded:: 2.3

.. seealso:: :py:func:`hAlignment`

.. seealso:: :py:func:`setVAlignment`
%End

    Qt::AlignmentFlag vAlignment() const;
%Docstring
Returns the vertical alignment for a column, which controls the alignment
used for drawing column values within cells.

:return: vertical alignment.

.. versionadded:: 2.12

.. seealso:: :py:func:`setVAlignment`

.. seealso:: :py:func:`hAlignment`
%End

    void setVAlignment( Qt::AlignmentFlag alignment );
%Docstring
Sets the vertical alignment for a column, which controls the alignment
used for drawing column values within cells.

:param alignment: vertical alignment for cell.

.. versionadded:: 2.12

.. seealso:: :py:func:`vAlignment`

.. seealso:: :py:func:`setHAlignment`
%End

    QString attribute() const;
%Docstring
Returns the attribute name or expression used for the column's values. This property
is only used when the column is part of a QgsComposerAttributeTable.

:return: attribute name or expression text for column

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`setAttribute`
%End

    void setAttribute( const QString &attribute );
%Docstring
Sets the attribute name or expression used for the column's values. This property
is only used when the column is part of a QgsComposerAttributeTable.

:param attribute: attribute name or expression text for column

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`attribute`
%End

    Qt::SortOrder sortOrder() const;
%Docstring
Returns the sort order for the column. This property is only used when the column
is part of a QgsComposerAttributeTable and when sortByRank is > 0.

:return: sort order for column

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`setSortOrder`

.. seealso:: :py:func:`sortByRank`
%End

    void setSortOrder( Qt::SortOrder sortOrder );
%Docstring
Sets the sort order for the column. This property is only used when the column
is part of a QgsComposerAttributeTable and when sortByRank is > 0.

:param sortOrder: sort order for column

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`sortOrder`

.. seealso:: :py:func:`setSortByRank`
%End

    int sortByRank() const;
%Docstring
Returns the sort rank for the column. If the sort rank is > 0 then the column
will be sorted in the table. The sort rank specifies the priority given to the
column when the table is sorted by multiple columns, with lower sort ranks
having higher priority. This property is only used when the column
is part of a QgsComposerAttributeTable.

:return: sort rank for column. If sort rank is <= 0 then the column is not being
sorted.

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`setSortByRank`

.. seealso:: :py:func:`sortOrder`
%End

    void setSortByRank( int sortByRank );
%Docstring
Sets the sort rank for the column. If the sort rank is > 0 then the column
will be sorted in the table. The sort rank specifies the priority given to the
column when the table is sorted by multiple columns, with lower sort ranks
having higher priority. This property is only used when the column
is part of a QgsComposerAttributeTable.

:param sortByRank: sort rank for column. If sort rank is <= 0 then the column is not being
sorted.

.. versionadded:: 2.3

.. note::

   only applicable when used in a QgsComposerAttributeTable

.. seealso:: :py:func:`sortByRank`

.. seealso:: :py:func:`setSortOrder`
%End

    QgsComposerTableColumn *clone();
%Docstring
Creates a duplicate column which is a deep copy of this column.

:return: a new QgsComposerTableColumn with same properties as this column.

.. versionadded:: 2.3
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposertablecolumn.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
