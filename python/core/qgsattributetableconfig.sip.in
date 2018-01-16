/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributetableconfig.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAttributeTableConfig
{
%Docstring
This is a container for configuration of the attribute table.
The configuration is specific for one vector layer.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsattributetableconfig.h"
%End
  public:

    enum Type
    {
      Field,
      Action
    };

    struct ColumnConfig
    {
      ColumnConfig();
%Docstring
Constructor for ColumnConfig
%End


      QgsAttributeTableConfig::Type type;
      QString name; //!< The name of the attribute if this column represents a field
      bool hidden;
      int width;
    };

    enum ActionWidgetStyle
    {
      ButtonList,
      DropDown
    };

    QgsAttributeTableConfig();
%Docstring
Constructor for QgsAttributeTableConfig.
%End

    QVector<QgsAttributeTableConfig::ColumnConfig> columns() const;
%Docstring
Get the list with all columns and their configuration.
The list order defines the order of appearance.
%End

    bool isEmpty() const;
%Docstring
Returns true if the configuration is empty, ie it contains no columns.
%End

    int mapVisibleColumnToIndex( int visibleColumn ) const;
%Docstring
Maps a visible column index to its original column index.

:param visibleColumn: index of visible column

:return: corresponding index when hidden columns are considered
%End

    void setColumns( const QVector<QgsAttributeTableConfig::ColumnConfig> &columns );
%Docstring
Set the list of columns visible in the attribute table.
The list order defines the order of appearance.
%End

    void update( const QgsFields &fields );
%Docstring
Update the configuration with the given fields.
Any field which is present in the configuration but not present in the
parameter fields will be removed. Any field which is in the parameter
fields but not in the configuration will be appended.
%End

    bool actionWidgetVisible() const;
%Docstring
Returns true if the action widget is visible
%End

    void setActionWidgetVisible( bool visible );
%Docstring
Set if the action widget is visible
%End

    ActionWidgetStyle actionWidgetStyle() const;
%Docstring
Get the style of the action widget
%End

    void setActionWidgetStyle( ActionWidgetStyle actionWidgetStyle );
%Docstring
Set the style of the action widget
%End

    void writeXml( QDomNode &node ) const;
%Docstring
Serialize to XML on layer save
%End

    void readXml( const QDomNode &node );
%Docstring
Deserialize to XML on layer load
%End

    QString sortExpression() const;
%Docstring
Get the expression used for sorting.
%End

    void setSortExpression( const QString &sortExpression );
%Docstring
Set the sort expression used for sorting.
%End

    int columnWidth( int column ) const;
%Docstring
Returns the width of a column, or -1 if column should use default width.

:param column: column index

.. seealso:: :py:func:`setColumnWidth`
%End

    void setColumnWidth( int column, int width );
%Docstring
Sets the width of a column.

:param column: column index
:param width: column width in pixels, or -1 if column should use default width

.. seealso:: :py:func:`columnWidth`
%End

    bool columnHidden( int column ) const;
%Docstring
Returns true if the specified column is hidden.

:param column: column index

.. seealso:: :py:func:`setColumnHidden`
%End

    void setColumnHidden( int column, bool hidden );
%Docstring
Sets whether the specified column should be hidden.

:param column: column index
:param hidden: set to true to hide column

.. seealso:: :py:func:`columnHidden`
%End

    Qt::SortOrder sortOrder() const;
%Docstring
Get the sort order

.. versionadded:: 2.16
%End

    void setSortOrder( Qt::SortOrder sortOrder );
%Docstring
Set the sort order

.. versionadded:: 2.16
%End

    bool operator!= ( const QgsAttributeTableConfig &other ) const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributetableconfig.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
