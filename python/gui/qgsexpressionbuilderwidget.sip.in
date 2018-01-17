/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionbuilderwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsExpressionItem : QStandardItem
{
%Docstring
An expression item that can be used in the QgsExpressionBuilderWidget tree.
%End

%TypeHeaderCode
#include "qgsexpressionbuilderwidget.h"
%End
  public:
    enum ItemType
    {
      Header,
      Field,
      ExpressionNode
    };

    QgsExpressionItem( const QString &label,
                       const QString &expressionText,
                       const QString &helpText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode );

    QgsExpressionItem( const QString &label,
                       const QString &expressionText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode );

    QString getExpressionText() const;

    QString getHelpText() const;
%Docstring
Get the help text that is associated with this expression item.

:return: The help text.
%End

    void setHelpText( const QString &helpText );
%Docstring
Set the help text for the current item

.. note::

   The help text can be set as a html string.
%End

    QgsExpressionItem::ItemType getItemType() const;
%Docstring
Get the type of expression item, e.g., header, field, ExpressionNode.

:return: The QgsExpressionItem.ItemType
%End

    static const int CUSTOM_SORT_ROLE;
    static const int ITEM_TYPE_ROLE;

};

class QgsExpressionItemSearchProxy : QSortFilterProxyModel
{
%Docstring
Search proxy used to filter the QgsExpressionBuilderWidget tree.
The default search for a tree model only searches top level this will handle one
level down
%End

%TypeHeaderCode
#include "qgsexpressionbuilderwidget.h"
%End
  public:
    QgsExpressionItemSearchProxy();

    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;


  protected:

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

};


class QgsExpressionBuilderWidget : QWidget
{
%Docstring
A reusable widget that can be used to build a expression string.
See QgsExpressionBuilderDialog for example of usage.
%End

%TypeHeaderCode
#include "qgsexpressionbuilderwidget.h"
%End
  public:

    QgsExpressionBuilderWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Create a new expression builder widget with an optional parent.
%End
    ~QgsExpressionBuilderWidget();

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets layer in order to get the fields and values

.. note::

   this needs to be called before calling loadFieldNames().
%End

    void loadFieldNames();
%Docstring
Loads all the field names from the layer.
@remarks Should this really be public couldn't we just do this for the user?
%End

    void loadFieldNames( const QgsFields &fields );

    void loadFieldsAndValues( const QMap<QString, QStringList> &fieldValues );
%Docstring
Loads field names and values from the specified map.

.. note::

   The field values must be quoted appropriately if they are strings.

.. versionadded:: 2.12
%End

    void setGeomCalculator( const QgsDistanceArea &da );
%Docstring
Sets geometry calculator used in distance/area calculations.
%End

    QString expressionText();
%Docstring
Gets the expression string that has been set in the expression area.

:return: The expression as a string. *
%End

    void setExpressionText( const QString &expression );
%Docstring
Sets the expression string for the widget
%End

    QgsExpressionContext expressionContext() const;
%Docstring
Returns the expression context for the widget. The context is used for the expression
preview result and for populating the list of available functions and variables.

.. seealso:: :py:func:`setExpressionContext`

.. versionadded:: 2.12
%End

    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
Sets the expression context for the widget. The context is used for the expression
preview result and for populating the list of available functions and variables.

:param context: expression context

.. seealso:: :py:func:`expressionContext`

.. versionadded:: 2.12
%End

    void registerItem( const QString &group, const QString &label, const QString &expressionText,
                       const QString &helpText = QString(),
                       QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode,
                       bool highlightedItem = false, int sortOrder = 1 );
%Docstring
Registers a node item for the expression builder.

:param group: The group the item will be show in the tree view.  If the group doesn't exsit it will be created.
:param label: The label that is show to the user for the item in the tree.
:param expressionText: The text that is inserted into the expression area when the user double clicks on the item.
:param helpText: The help text that the user will see when item is selected.
:param type: The type of the expression item.
:param highlightedItem: set to true to make the item highlighted, which inserts a bold copy of the item at the top level
:param sortOrder: sort ranking for item
%End

    bool isExpressionValid();

    void saveToRecent( const QString &collection = "generic" );
%Docstring
Adds the current expression to the given collection.
By default it is saved to the collection "generic".
%End

    void loadRecent( const QString &collection = "generic" );
%Docstring
Loads the recent expressions from the given collection.
By default it is loaded from the collection "generic".
%End

    void newFunctionFile( const QString &fileName = "scratch" );
%Docstring
Create a new file in the function editor
%End

    void saveFunctionFile( QString fileName );
%Docstring
Save the current function editor text to the given file.
%End

    void loadCodeFromFile( QString path );
%Docstring
Load code from the given file into the function editor
%End

    void loadFunctionCode( const QString &code );
%Docstring
Load code into the function editor
%End

    void updateFunctionFileList( const QString &path );
%Docstring
Update the list of function files found at the given path
%End

    QStandardItemModel *model();
%Docstring
Returns a pointer to the dialog's function item model.
This method is exposed for testing purposes only - it should not be used to modify the model.

.. versionadded:: 3.0
%End

    QgsProject *project();
%Docstring
Returns the project currently associated with the widget.

.. seealso:: :py:func:`setProject`

.. versionadded:: 3.0
%End

    void setProject( QgsProject *project );
%Docstring
Sets the ``project`` currently associated with the widget. This
controls which layers and relations and other project-specific items are shown in the widget.

.. seealso:: :py:func:`project`

.. versionadded:: 3.0
%End

    bool evalError() const;
%Docstring
Will be set to true if the current expression text reported an eval error
with the context.

.. versionadded:: 3.0
%End

    bool parserError() const;
%Docstring
Will be set to true if the current expression text reports a parser error
with the context.

.. versionadded:: 3.0
%End

  public slots:

    void loadSampleValues();
%Docstring
Load sample values into the sample value area
%End

    void loadAllValues();
%Docstring
Load all unique values from the set layer into the sample area
%End

    void autosave();
%Docstring
Auto save the current Python function code.
%End

    void setAutoSave( bool enabled );
%Docstring
Enabled or disable auto saving. When enabled Python scripts will be auto saved
when text changes.

:param enabled: True to enable auto saving.
%End

  signals:

    void expressionParsed( bool isValid );
%Docstring
Emitted when the user changes the expression in the widget.
Users of this widget should connect to this signal to decide if to let the user
continue.

:param isValid: Is true if the expression the user has typed is valid.
%End

    void evalErrorChanged();
%Docstring
Will be set to true if the current expression text reported an eval error
with the context.

.. versionadded:: 3.0
%End

    void parserErrorChanged();
%Docstring
Will be set to true if the current expression text reported a parser error
with the context.

.. versionadded:: 3.0
%End

  protected:
    virtual void showEvent( QShowEvent *e );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionbuilderwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
