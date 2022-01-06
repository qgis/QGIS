/***************************************************************************
    qgsexpressiontreeview.h
     --------------------------------------
    Date                 : march 2020 - quarantine day 9
    Copyright            : (C) 2020 by Denis Rouzaud
    Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONTREEVIEW_H
#define QGSEXPRESSIONTREEVIEW_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QPointer>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"


class QgsVectorLayer;



/**
 * \ingroup gui
 * \brief An expression item that can be used in the QgsExpressionBuilderWidget tree.
  */
class GUI_EXPORT QgsExpressionItem : public QStandardItem
{
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
                       QgsExpressionItem::ItemType itemType = ExpressionNode )
      : QStandardItem( label )
    {
      mExpressionText = expressionText;
      mHelpText = helpText;
      mType = itemType;
      setData( itemType, ITEM_TYPE_ROLE );
    }

    QgsExpressionItem( const QString &label,
                       const QString &expressionText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode )
      : QStandardItem( label )
    {
      mExpressionText = expressionText;
      mType = itemType;
      setData( itemType, ITEM_TYPE_ROLE );
    }

    QString getExpressionText() const { return mExpressionText; }

    /**
     * Gets the help text that is associated with this expression item.
      *
      * \returns The help text.
      */
    QString getHelpText() const { return mHelpText; }

    /**
     * Set the help text for the current item
      *
      * \note The help text can be set as a html string.
      */
    void setHelpText( const QString &helpText ) { mHelpText = helpText; }

    /**
     * Gets the type of expression item, e.g., header, field, ExpressionNode.
      *
      * \returns The QgsExpressionItem::ItemType
      */
    QgsExpressionItem::ItemType getItemType() const { return mType; }

    //! Custom sort order role
    static const int CUSTOM_SORT_ROLE = Qt::UserRole + 1;
    //! Item type role
    static const int ITEM_TYPE_ROLE = Qt::UserRole + 2;
    //! Search tags role
    static const int SEARCH_TAGS_ROLE = Qt::UserRole + 3;
    //! Item name role
    static const int ITEM_NAME_ROLE = Qt::UserRole + 4;
    //! Layer ID role \since QGIS 3.24
    static const int LAYER_ID_ROLE = Qt::UserRole + 5;

  private:
    QString mExpressionText;
    QString mHelpText;
    QgsExpressionItem::ItemType mType;
};


/**
 * \ingroup gui
 * \brief Search proxy used to filter the QgsExpressionBuilderWidget tree.
  * The default search for a tree model only searches top level this will handle one
  * level down
  */
class GUI_EXPORT QgsExpressionItemSearchProxy : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    QgsExpressionItemSearchProxy();

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

    /**
     * Sets the search filter \a string.
     *
     * \since QGIS 3.22
     */
    void setFilterString( const QString &string );

  protected:

    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:

    QString mFilterString;
};

/**
 * \ingroup gui
 * \class QgsExpressionTreeView
 * \brief QgsExpressionTreeView is a tree view to list all expressions
 * functions, variables and fields that can be used in an expression.
 * \see QgsExpressionBuilderWidget
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsExpressionTreeView : public QTreeView
{
    Q_OBJECT
  public:

    /**
     * \ingroup gui
     * \class MenuProvider
     * \brief Implementation of this interface can be implemented to allow QgsExpressionTreeView
     * instance to provide custom context menus (opened upon right-click).
     * \since QGIS 3.14
     */
    class MenuProvider
    {
      public:
        //! Constructor
        explicit MenuProvider() = default;
        virtual ~MenuProvider() = default;

        //! Returns a newly created menu instance
        virtual QMenu *createContextMenu( QgsExpressionItem *item ) SIP_FACTORY {Q_UNUSED( item ) return nullptr;}
    };

    //! Constructor
    QgsExpressionTreeView( QWidget *parent = nullptr );

    /**
     * Sets layer in order to get the fields and values
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * This allows loading fields without specifying a layer
     */
    void loadFieldNames( const QgsFields &fields );

    /**
     * Sets the expression context for the tree view. The context is used
     * to populate the list of available functions and variables.
     * \param context expression context
     * \see expressionContext
     */
    void setExpressionContext( const QgsExpressionContext &context );

    /**
     * Returns the expression context for the widget. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * \see setExpressionContext
     */
    QgsExpressionContext expressionContext() const { return mExpressionContext; }

    /**
     * Returns the project currently associated with the widget.
     * \see setProject()
     */
    QgsProject *project();

    /**
     * Sets the \a project currently associated with the widget. This
     * controls which layers and relations and other project-specific items are shown in the widget.
     * \see project()
     */
    void setProject( QgsProject *project );

    /**
     * Sets the menu provider.
     * This does not take ownership of the provider
     */
    void setMenuProvider( MenuProvider *provider );

    /**
     * Refreshes the content of the tree
     */
    void refresh();

    /**
     * Returns the current item or a nullptr
     */
    QgsExpressionItem *currentItem() const;

    /**
     * Returns a pointer to the dialog's function item model.
     * This method is exposed for testing purposes only - it should not be used to modify the model
     * \note will be removed in QGIS 4
     * \deprecated since QGIS 3.14
     */
    Q_DECL_DEPRECATED QStandardItemModel *model() SIP_SKIP; // TODO remove QGIS 4

    /**
     * Loads the recent expressions from the given \a collection.
     * By default it is loaded from the collection "generic".
     */
    void loadRecent( const QString &collection = QStringLiteral( "generic" ) );

    /**
     * Adds the current expression to the given \a collection.
     * By default it is saved to the collection "generic".
     */
    void saveToRecent( const QString &expressionText, const QString &collection = "generic" );

    /**
     * Stores the user \a expression with given \a label and \a helpText.
     */
    void saveToUserExpressions( const QString &label, const QString &expression, const QString &helpText );

    /**
     * Removes the expression \a label from the user stored expressions.
     */
    void removeFromUserExpressions( const QString &label );

    /**
     * Loads the user expressions.
     * This is done on request since it can be very slow if there are thousands of user expressions
     */
    void loadUserExpressions( );

    /**
     * Returns the list of expression items matching a \a label.
     */
    const QList<QgsExpressionItem *> findExpressions( const QString &label );

    /**
     * Returns the user expression labels
     */
    QStringList userExpressionLabels() const SIP_SKIP;

    /**
     * Create the expressions JSON document storing all the user expressions to be exported.
     * \returns the created expressions JSON file
     */
    QJsonDocument exportUserExpressions();

    /**
     * Load and permanently store the expressions from the expressions JSON document.
     * \param expressionsDocument the parsed expressions JSON file
     */
    void loadExpressionsFromJson( const QJsonDocument &expressionsDocument );

  signals:
    //! Emitted when a expression item is double clicked
    void expressionItemDoubleClicked( const QString &text );

    //! Emitter when the current expression item changed
    void currentExpressionItemChanged( QgsExpressionItem *item );

  public slots:
    //! Sets the text to filter the expression tree
    void setSearchText( const QString &text );


  private slots:
    void onDoubleClicked( const QModelIndex &index );

    void showContextMenu( QPoint pt );

    void currentItemChanged( const QModelIndex &index, const QModelIndex & );

  private:
    void updateFunctionTree();

    /**
     * Registers a node item for the expression builder.
     * \param group The group the item will be show in the tree view.  If the group doesn't exist it will be created.
     * \param label The label that is show to the user for the item in the tree.
     * \param expressionText The text that is inserted into the expression area when the user double clicks on the item.
     * \param helpText The help text that the user will see when item is selected.
     * \param type The type of the expression item.
     * \param highlightedItem set to TRUE to make the item highlighted, which inserts a bold copy of the item at the top level
     * \param sortOrder sort ranking for item
     * \param icon custom icon to show for item
     * \param tags tags to find function
     * \param name name of the item
     */
    QgsExpressionItem *registerItem( const QString &group, const QString &label, const QString &expressionText,
                                     const QString &helpText = QString(),
                                     QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode,
                                     bool highlightedItem = false, int sortOrder = 1,
                                     const QIcon &icon = QIcon(),
                                     const QStringList &tags = QStringList(),
                                     const QString &name = QString() );

    /**
     * Registers a node item for the expression builder, adding multiple items when the function exists in multiple groups
     * \param groups The groups the item will be show in the tree view.  If a group doesn't exist it will be created.
     * \param label The label that is show to the user for the item in the tree.
     * \param expressionText The text that is inserted into the expression area when the user double clicks on the item.
     * \param helpText The help text that the user will see when item is selected.
     * \param type The type of the expression item.
     * \param highlightedItem set to TRUE to make the item highlighted, which inserts a bold copy of the item at the top level
     * \param sortOrder sort ranking for item
     * \param tags tags to find function
     */
    void registerItemForAllGroups( const QStringList &groups, const QString &label, const QString &expressionText,
                                   const QString &helpText = QString(),
                                   QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode,
                                   bool highlightedItem = false, int sortOrder = 1, const QStringList &tags = QStringList() );

    void loadExpressionContext();
    void loadRelations();
    void loadLayers();
    void loadLayerFields( QgsVectorLayer *layer, QgsExpressionItem *parentItem );
    void loadFieldNames();

    /**
     * Display a message box to ask the user what to do when an expression
     * with the same \a label already exists. Answering "Yes" will replace
     * the old expression with the one from the file, while "No" will keep
     * the old expression.
     * \param isApplyToAll whether the decision of the user should be applied to any future label collision
     * \param isOkToOverwrite whether to overwrite the old expression with the new one in case of label collision
     * \param label the label of the expression
     * \param oldExpression the old expression for a given label
     * \param newExpression the new expression for a given label
     */
    void showMessageBoxConfirmExpressionOverwrite( bool &isApplyToAll, bool &isOkToOverwrite, const QString &label, const QString &oldExpression, const QString &newExpression );


    std::unique_ptr<QStandardItemModel> mModel;
    std::unique_ptr<QgsExpressionItemSearchProxy> mProxyModel;
    QMap<QString, QgsExpressionItem *> mExpressionGroups;

    MenuProvider *mMenuProvider = nullptr;

    QgsVectorLayer *mLayer = nullptr;
    QPointer< QgsProject > mProject;
    QgsExpressionContext mExpressionContext;
    QString mRecentKey;

    QStringList mUserExpressionLabels;
};

#endif // QGSEXPRESSIONTREEVIEW_H
