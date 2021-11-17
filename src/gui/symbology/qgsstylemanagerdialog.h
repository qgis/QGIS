/***************************************************************************
    qgsstylemanagerdialog.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEV2MANAGERDIALOG_H
#define QGSSTYLEV2MANAGERDIALOG_H

#include <QDialog>
#include <QStandardItem>
#include <QAction>
#include <QMenu>

#include "ui_qgsstylemanagerdialogbase.h"
#include "qgshelp.h"
#include "qgsstylemodel.h"
#include "qgis_gui.h"

class QgsStyle;
class QgsTemporaryCursorOverride;
class QgsMessageBar;

#ifndef SIP_RUN
///@cond PRIVATE
class QgsCheckableStyleModel: public QgsStyleProxyModel
{
    Q_OBJECT
  public:

    explicit QgsCheckableStyleModel( QgsStyleModel *sourceModel, QObject *parent = nullptr, bool readOnly = false );
    explicit QgsCheckableStyleModel( QgsStyle *style, QObject *parent = nullptr, bool readOnly = false );

    void setCheckable( bool checkable );
    void setCheckTag( const QString &tag );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

  private:

    QgsStyle *mStyle = nullptr;
    bool mCheckable = false;
    QString mCheckTag;
    bool mReadOnly = false;

};
#endif
///@endcond

/**
 * \ingroup gui
 * \class QgsStyleManagerDialog
 *
 * \brief A dialog allowing users to customize and populate a QgsStyle.
 */
class GUI_EXPORT QgsStyleManagerDialog : public QDialog, private Ui::QgsStyleManagerDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStyleManagerDialog, with the specified \a parent widget and window \a flags.
     *
     * The \a style argument specifies the linked QgsStyle database. Symbols and objects contained within
     * this style will be shown in the dialog, and changes made within the dialog will be applied to \a style.
     * The \a style object must last for the lifetime of the dialog.
     */
    QgsStyleManagerDialog( QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(),
                           bool readOnly = false );

    // TODO QGIS 4.0 -- rename "RampType" to "rampType".

    /**
     * Opens the add color ramp dialog, returning the new color ramp's name if the ramp has been added.
     *
     * The \a RampType argument should be set to a string representing a valid QgsColorRamp::type() value.
     */
    static QString addColorRampStatic( QWidget *parent, QgsStyle *style,
                                       const QString &RampType = QString() );

    /**
     * Sets whether the favorites group should be shown. The default is to show the group.
     *
     * \since QGIS 3.6
     */
    void setFavoritesGroupVisible( bool show );

    /**
     * Sets whether smart groups should be shown. The default is to show the groups.
     *
     * \since QGIS 3.6
     */
    void setSmartGroupsVisible( bool show );

    /**
     * Sets the base \a name for the style, which is used by the dialog to reflect the
     * original style/XML file name.
     *
     * \a name should be stripped of any extensions and folder information, e.g. "transport_styles",
     * not "d:/stuff/transport_styles.xml".
     *
     * \since QGIS 3.6
     */
    void setBaseStyleName( const QString &name );

  public slots:

    // TODO QGIS 4.0 -- most of this should be private

    /**
     * Raises, unminimizes and activates this window
     * \since QGIS 3.4
     */
    void activate();

    /**
     * Triggers the dialog for adding a new item, based on the currently
     * selected item type tab.
     */
    void addItem();

    /**
     * Triggers the dialog for editing the current item.
     */
    void editItem();

    /**
     * Removes the current selected item.
     */
    void removeItem();

    /**
     * Triggers the dialog to export selected items as SVG files.
     *
     * \see exportItemsPNG()
     * \see exportSelectedItemsImages()
     */
    void exportItemsSVG();

    /**
     * Triggers the dialog to export selected items as PNG files.
     *
     * \see exportItemsSVG()
     * \see exportSelectedItemsImages()
     */
    void exportItemsPNG();

    /**
     * Triggers the dialog to export selected items as images of the specified \a format and \a size.
     *
     * \see exportItemsSVG()
     * \see exportItemsPNG()
     */
    void exportSelectedItemsImages( const QString &dir, const QString &format, QSize size );

    /**
     * Triggers the dialog to export items.
     *
     * \see importItems()
     */
    void exportItems();

    /**
     * Triggers the dialog to import items.
     *
     * \see exportItems()
     */
    void importItems();

    /**
     * Refreshes the list of items.
     */
    void populateList();

    /**
     * Called when the dialog is going to be closed.
     */
    void onFinished();

    //! Closes the dialog
    void onClose();

    //! Opens the associated help
    void showHelp();

    /**
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void itemChanged( QStandardItem *item ) SIP_DEPRECATED;

    /**
     * Triggered when the current group (or tag) is changed.
     */
    void groupChanged( const QModelIndex & );

    /**
     * Triggered when a group \a item is renamed.
     */
    void groupRenamed( QStandardItem *item );

    /**
     * Triggers the dialog to add a new tag.
     */
    int addTag();

    /**
     * Triggers the dialog to add a new smart group.
     */
    int addSmartgroup();

    /**
     * Removes the selected tag or smartgroup.
     */
    void removeGroup();

    /**
     * Toggles the interactive item tagging mode.
     */
    void tagSymbolsAction();

    /**
     * Triggers the dialog for editing the selected smart group.
     */
    void editSmartgroupAction();

    /**
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void regrouped( QStandardItem * ) SIP_DEPRECATED;

    /**
     * Sets the \a filter string to filter symbols by.
     */
    void filterSymbols( const QString &filter );

    //! Perform symbol specific tasks when selected
    void symbolSelected( const QModelIndex & );

    //! Perform tasks when the selected symbols change
    void selectedSymbolsChanged( const QItemSelection &selected, const QItemSelection &deselected );

    //! Context menu for the groupTree
    void grouptreeContextMenu( QPoint );

    //! Context menu for the listItems ( symbols list )
    void listitemsContextMenu( QPoint );

  protected slots:

    /**
     * Triggers adding a new color ramp.
     *
     * If \a type is set to a string representing a valid QgsColorRamp::type() value then a new ramp
     * of that type will be created. If \a type is empty, then the user will be prompted to select
     * the color ramp type.
     */
    bool addColorRamp( const QString &type = QString() );

    //! Add selected symbols to favorites
    void addFavoriteSelectedSymbols();
    //! Remove selected symbols from favorites
    void removeFavoriteSelectedSymbols();
    //! Tag selected symbols using menu item selection
    void tagSelectedSymbols( bool newTag = false );
    //! Remove all tags from selected symbols
    void detagSelectedSymbols();

  protected:

    /**
     * Populate combo box with known style items (symbols, color ramps).
     *
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void populateTypes() SIP_DEPRECATED;

    //! populate the groups
    void populateGroups();

    /**
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void setSymbolsChecked( const QStringList & ) SIP_DEPRECATED;

    /**
     * Populates the list view with symbols of the current type with the given names.
     *
     * \deprecated No longer required in QGIS 3.6, as the model is updated live. Has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void populateSymbols( const QStringList &symbolNames, bool checkable = false ) SIP_DEPRECATED;

    /**
     * Populates the list view with color ramps of the current type with the given names.
     *
     * \deprecated No longer required in QGIS 3.6, as the model is updated live. Has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void populateColorRamps( const QStringList &colorRamps, bool checkable = false ) SIP_DEPRECATED;

    int currentItemType();
    QString currentItemName();

    //! add a new symbol to style
    bool addSymbol( int symbolType = -1 );

    bool editSymbol();
    bool editColorRamp();

    /**
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED bool removeSymbol() SIP_DEPRECATED;

    /**
     * \deprecated since QGIS 3.6 - has no effect and will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED bool removeColorRamp() SIP_DEPRECATED;

    //! Enables or disbables the symbol specific inputs
    void enableSymbolInputs( bool );
    //! Enables or disables the groupTree specific inputs
    void enableGroupInputs( bool );
    //! Enables or disables the groupTree items for grouping mode
    void enableItemsForGroupingMode( bool );

    //! sets the text of the item with bold font
    void setBold( QStandardItem * );

  private slots:

    void tabItemType_currentChanged( int );

    void copyItemsToDefault();

    void copyItem();

    void pasteItem();

    void setThumbnailSize( int );

  private:
    int selectedItemType();

    /**
     * Returns TRUE if the "All" tab is selected.
     */
    bool allTypesSelected() const;

    struct ItemDetails
    {
      QgsStyle::StyleEntity entityType;
      Qgis::SymbolType symbolType;
      QString name;
    };

    enum GroupModelRoles
    {
      Name = Qt::UserRole + 1,
      TagName
    };

    QList< ItemDetails > selectedItems();

    /**
     * Returns count of items copied, excluding skipped items.
     */
    static int copyItems( const QList< ItemDetails > &items, QgsStyle *src, QgsStyle *dst,
                          QWidget *parentWidget, std::unique_ptr<QgsTemporaryCursorOverride> &cursorOverride,
                          bool isImport, const QStringList &importTags, bool addToFavorites, bool ignoreSourceTags );

    QgsMessageBar *mMessageBar = nullptr;

    QgsStyle *mStyle = nullptr;

    QgsCheckableStyleModel *mModel = nullptr;

    QString mStyleFilename;

    bool mModified = false;

    //! Mode to display the symbol list
    bool mGroupingMode = false;

    //! space to store symbol tags
    QStringList mTagList;

    //! Context menu for the symbols/colorramps
    QMenu *mGroupMenu = nullptr;

    //! Sub-menu of \c mGroupMenu, dynamically filled to show one entry for every group
    QMenu *mGroupListMenu = nullptr;

    //! Context menu for the group tree
    QMenu *mGroupTreeContextMenu = nullptr;

    //! Menu for the "Add item" toolbutton when in colorramp mode
    QMenu *mMenuBtnAddItemColorRamp = nullptr;

    //! Menu for the "Add item" toolbutton when in all symbols mode
    QMenu *mMenuBtnAddItemAll = nullptr;

    //! Menu for the "Add item" toolbutton when in label settings mode
    QMenu *mMenuBtnAddItemLabelSettings = nullptr;

    //! Menu for the "Add item" toolbutton when in legend patch shape mode
    QMenu *mMenuBtnAddItemLegendPatchShape = nullptr;

    //! Menu for the "Add item" toolbutton when in 3d symbol mode
    QMenu *mMenuBtnAddItemSymbol3D = nullptr;

    QAction *mActionCopyToDefault = nullptr;

    QAction *mActionCopyItem = nullptr;
    QAction *mActionPasteItem = nullptr;

    int mBlockGroupUpdates = 0;

    bool mReadOnly = false;
    bool mFavoritesGroupVisible = true;
    bool mSmartGroupVisible = true;
    QString mBaseName;

    static QString sPreviousTag;

    bool addTextFormat();
    bool editTextFormat();

    bool addLabelSettings( QgsWkbTypes::GeometryType type );
    bool editLabelSettings();

    bool addLegendPatchShape( Qgis::SymbolType type );
    bool editLegendPatchShape();

    bool addSymbol3D( const QString &type );
    bool editSymbol3D();

    friend class QgsStyleExportImportDialog;
};

#endif
