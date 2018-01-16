/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstreewidgetitem.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsTreeWidgetItem : QTreeWidgetItem
{
%Docstring
QTreeWidgetItem subclass with custom handling for item sorting.

QgsTreeWidgetItem allows for items to be sorted using a specified user role, and
also correctly handles sorting numeric or mixed text and numeric values.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstreewidgetitem.h"
%End
  public:

    explicit QgsTreeWidgetItem( QTreeWidget *view /TransferThis/, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param view: parent QTreeWidget view
:param type: item type
%End

    explicit QgsTreeWidgetItem( int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param type: item type
%End

    QgsTreeWidgetItem( const QStringList &strings, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param strings: list of strings containing text for each column in the item
:param type: item type
%End

    QgsTreeWidgetItem( QTreeWidget *view /TransferThis/, const QStringList &strings, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param view: parent QTreeWidget view
:param strings: list of strings containing text for each column in the item
:param type: item type
%End

    QgsTreeWidgetItem( QTreeWidget *view /TransferThis/, QTreeWidgetItem *after, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param view: parent QTreeWidget view
:param after: QTreeWidgetItem to place insert item after in the view
:param type: item type
%End

    explicit QgsTreeWidgetItem( QTreeWidgetItem *parent /TransferThis/, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param parent: QTreeWidgetItem item
:param type: item type
%End

    QgsTreeWidgetItem( QTreeWidgetItem *parent /TransferThis/, const QStringList &strings, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param parent: QTreeWidgetItem item
:param strings: list of strings containing text for each column in the item
:param type: item type
%End

    QgsTreeWidgetItem( QTreeWidgetItem *parent /TransferThis/, QTreeWidgetItem *after, int type = Type );
%Docstring
Constructor for QgsTreeWidgetItem

:param parent: QTreeWidgetItem item
:param after: QTreeWidgetItem to place insert item after in the view
:param type: item type
%End

    void setSortData( int column, const QVariant &value );
%Docstring
Sets the custom sort data for a specified column. If set, this value will be used when
sorting the item instead of the item's display text. If not set, the item's display
text will be used when sorting.

:param column: column index
:param value: sort value

.. seealso:: :py:func:`sortData`
%End

    QVariant sortData( int column ) const;
%Docstring
Returns the custom sort data for a specified column. If set, this value will be used when
sorting the item instead of the item's display text. If not set, the item's display
text will be used when sorting.

.. seealso:: :py:func:`setSortData`
%End

    void setAlwaysOnTopPriority( int priority );
%Docstring
Sets a the item to display always on top of other items in the widget, regardless of the
sort column and sort or display value for the item.

:param priority: priority for sorting always on top items. Items with a lower priority will
be placed above items with a higher priority.

.. seealso:: :py:func:`alwaysOnTopPriority`
%End

    int alwaysOnTopPriority() const;
%Docstring
Returns the item's priority when it is set to show always on top. Items with a lower priority will
be placed above items with a higher priority.

:return: priority, or -1 if item is not set to show always on top

.. seealso:: :py:func:`setAlwaysOnTopPriority`
%End

    virtual bool operator<( const QTreeWidgetItem &other ) const;


    bool operator>=( const QTreeWidgetItem &other ) const;

};

class QgsTreeWidgetItemObject: QObject, QgsTreeWidgetItem
{
%Docstring
Custom QgsTreeWidgetItem with extra signals when item is edited.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstreewidgetitem.h"
%End
  public:

    explicit QgsTreeWidgetItemObject( int type = Type );
%Docstring
Constructor for QgsTreeWidgetItemObject

:param type: item type
%End

    explicit QgsTreeWidgetItemObject( QTreeWidget *parent /TransferThis/, int type = Type );
%Docstring
Constructs a tree widget item of the specified type and appends it to the items in the given parent.
%End

    virtual void setData( int column, int role, const QVariant &value );

%Docstring
Sets the value for the item's column and role to the given value.
%End

  signals:
    void itemEdited( QTreeWidgetItem *item, int column );
%Docstring
This signal is emitted when the contents of the column in the specified item has been edited by the user.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstreewidgetitem.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
