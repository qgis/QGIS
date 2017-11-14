/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsstylemanagerdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsStyleManagerDialog : QDialog
{

%TypeHeaderCode
#include "qgsstylemanagerdialog.h"
%End
  public:
    QgsStyleManagerDialog( QgsStyle *style, QWidget *parent /TransferThis/ = 0 );

    static QString addColorRampStatic( QWidget *parent, QgsStyle *style,
                                       QString RampType = QString() );
%Docstring
open add color ramp dialog, return color ramp's name if the ramp has been added
 :rtype: str
%End

  public slots:
    void addItem();
    void editItem();
    void removeItem();
    void exportItemsSVG();
    void exportItemsPNG();
    void exportSelectedItemsImages( const QString &dir, const QString &format, QSize size );
    void exportItems();
    void importItems();

    void populateList();
%Docstring
adds symbols of some type to list
%End

    void onFinished();
%Docstring
called when the dialog is going to be closed
%End

    void onClose();
%Docstring
Close the dialog
%End

    void showHelp();
%Docstring
Open the associated help
%End

    void itemChanged( QStandardItem *item );

    void groupChanged( const QModelIndex & );
    void groupRenamed( QStandardItem * );
    int addTag();
%Docstring
add a tag
 :rtype: int
%End
    int addSmartgroup();
%Docstring
add a smartgroup
 :rtype: int
%End
    void removeGroup();
%Docstring
remove a tag or smartgroup
%End

    void tagSymbolsAction();
%Docstring
carry out symbol tagging using check boxes
%End

    void editSmartgroupAction();
%Docstring
edit the selected smart group
%End

    void regrouped( QStandardItem * );
%Docstring
symbol changed from one group
%End

    void filterSymbols( const QString & );
%Docstring
filter the symbols based on input search term
%End

    void symbolSelected( const QModelIndex & );
%Docstring
Perform symbol specific tasks when selected
%End

    void selectedSymbolsChanged( const QItemSelection &selected, const QItemSelection &deselected );
%Docstring
Perform tasks when the selected symbols change
%End

    void grouptreeContextMenu( QPoint );
%Docstring
Context menu for the groupTree
%End

    void listitemsContextMenu( QPoint );
%Docstring
Context menu for the listItems ( symbols list )
%End

  protected slots:
    bool addColorRamp( QAction *action );
%Docstring
 :rtype: bool
%End
    void addFavoriteSelectedSymbols();
%Docstring
Add selected symbols to favorites
%End
    void removeFavoriteSelectedSymbols();
%Docstring
Remove selected symbols from favorites
%End
    void tagSelectedSymbols( bool newTag = false );
%Docstring
Tag selected symbols using menu item selection
%End
    void detagSelectedSymbols();
%Docstring
Remove all tags from selected symbols
%End

  protected:

    void populateTypes();
%Docstring
populate combo box with known style items (symbols, color ramps)
%End

    void populateGroups();
%Docstring
populate the groups
%End
    void setSymbolsChecked( const QStringList & );
%Docstring
to set symbols checked when in editing mode
%End

    void populateSymbols( const QStringList &symbolNames, bool checkable = false );
%Docstring
populate list view with symbols of the current type with the given names
%End

    void populateColorRamps( const QStringList &colorRamps, bool checkable = false );
%Docstring
populate list view with color ramps
%End

    int currentItemType();
%Docstring
 :rtype: int
%End
    QString currentItemName();
%Docstring
 :rtype: str
%End

    bool addSymbol();
%Docstring
add a new symbol to style
 :rtype: bool
%End
    bool addColorRamp();
%Docstring
add a new color ramp to style
 :rtype: bool
%End

    bool editSymbol();
%Docstring
 :rtype: bool
%End
    bool editColorRamp();
%Docstring
 :rtype: bool
%End

    bool removeSymbol();
%Docstring
 :rtype: bool
%End
    bool removeColorRamp();
%Docstring
 :rtype: bool
%End

    void enableSymbolInputs( bool );
%Docstring
Enables or disbables the symbol specific inputs
%End
    void enableGroupInputs( bool );
%Docstring
Enables or disables the groupTree specific inputs
%End
    void enableItemsForGroupingMode( bool );
%Docstring
Enables or disables the groupTree items for grouping mode
%End

    void setBold( QStandardItem * );
%Docstring
sets the text of the item with bold font
%End










};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsstylemanagerdialog.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
