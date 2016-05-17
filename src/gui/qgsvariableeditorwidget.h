/***************************************************************************
     qgsvariableeditorwidget.h
     -------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVARIABLEEDITORWIDGET_H
#define QGSVARIABLEEDITORWIDGET_H

#include "qgis.h"
#include <QWidget>
#include <QTreeWidget>
#include <QItemDelegate>

class QTableWidget;
class QgsExpressionContextScope;
class QPushButton;
class QgsExpressionContext;
class QgsVariableEditorTree;
class VariableEditorDelegate;

/** \ingroup gui
 * \class QgsVariableEditorWidget
 * A tree based widget for editing expression context scope variables. The widget allows editing
 * variables from a QgsExpressionContextScope, and can optionally also show inherited
 * variables from a QgsExpressionContext.
 * \note added in QGIS 2.12
 */

class GUI_EXPORT QgsVariableEditorWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( QString settingGroup READ settingGroup WRITE setSettingGroup )

  public:

    /** Constructor for QgsVariableEditorWidget.
     * @param parent parent widget
     */
    QgsVariableEditorWidget( QWidget *parent = nullptr );

    ~QgsVariableEditorWidget();

    /** Overwrites the QgsExpressionContext for the widget. Setting a context
     * allows the widget to show all inherited variables for the context,
     * and highlight any overridden variables within scopes.
     * @param context expression context
     * @see context()
     */
    void setContext( QgsExpressionContext* context );

    /** Returns the current expression context for the widget. QgsVariableEditorWidget widgets
     * are created with an empty context by default.
     * @see setContext()
     */
    QgsExpressionContext* context() const { return mContext.data(); }

    /** Reloads all scopes from the editor's current context. This method should be called
     * after adding or removing scopes from the attached context.
     * @see context()
     */
    void reloadContext();

    /** Sets the editable scope for the widget. Only variables from the editable scope can
     * be modified by users.
     * @param scopeIndex index of current editable scope. Set to -1 to disable
     * editing and make the widget read-only.
     * @see editableScope()
     */
    void setEditableScopeIndex( int scopeIndex );

    /** Returns the current editable scope for the widget.
     * @returns editable scope, or 0 if no editable scope is set
     * @see setEditableScopeIndex()
     */
    QgsExpressionContextScope* editableScope() const;

    /** Sets the setting group for the widget. QgsVariableEditorWidget widgets with
     * the same setting group will synchronise their settings, eg the size
     * of columns in the tree widget.
     * @param group setting group
     * @see settingGroup()
     */
    void setSettingGroup( const QString &group ) { mSettingGroup = group; }

    /** Returns the setting group for the widget. QgsVariableEditorWidget widgets with
     * the same setting group will synchronise their settings, eg the size
     * of columns in the tree widget.
     * @returns setting group name
     * @see setSettingGroup()
     */
    QString settingGroup() const { return mSettingGroup; }

    /** Returns a map variables set within the editable scope. Read only variables are not
     * returned. This method can be used to retrieve the variables edited an added by
     * users via the widget.
     */
    QgsStringMap variablesInActiveScope() const;

  signals:

    /** Emitted when the user has modified a scope using the widget.
     */
    void scopeChanged();

  protected:

    void showEvent( QShowEvent *event ) override;

  private:

    QScopedPointer<QgsExpressionContext> mContext;
    int mEditableScopeIndex;
    QgsVariableEditorTree* mTreeWidget;
    QPushButton* mAddButton;
    QPushButton* mRemoveButton;
    QString mSettingGroup;
    bool mShown;

    QString saveKey() const;

  private slots:

    void on_mAddButton_clicked();
    void on_mRemoveButton_clicked();
    void selectionChanged();

};


/// @cond PRIVATE

/* QgsVariableEditorTree is NOT part of the public QGIS api. It's only
 * public here as Qt meta objects can't be nested classes
 */

class QgsVariableEditorTree : public QTreeWidget
{
    Q_OBJECT

  public:

    enum VaribleRoles
    {
      ContextIndex = Qt::UserRole,
      RowBaseColor
    };

    explicit QgsVariableEditorTree( QWidget *parent = nullptr );

    QTreeWidgetItem *indexToItem( const QModelIndex &index ) const { return itemFromIndex( index ); }
    QModelIndex itemToIndex( QTreeWidgetItem* item ) const { return indexFromItem( item ); }
    QString variableNameFromItem( QTreeWidgetItem* item ) const { return item ? item->text( 0 ) : QString(); }
    QString variableNameFromIndex( const QModelIndex& index ) const { return variableNameFromItem( itemFromIndex( index ) ); }
    QgsExpressionContextScope* scopeFromItem( QTreeWidgetItem* item ) const;
    QTreeWidgetItem* itemFromVariable( QgsExpressionContextScope* scope, const QString& name ) const;
    void setEditableScopeIndex( int scopeIndex ) { mEditableScopeIndex = scopeIndex; }
    QgsExpressionContextScope* editableScope();
    void setContext( QgsExpressionContext* context ) { mContext = context; }
    void refreshTree();
    void removeItem( QTreeWidgetItem* item );
    void renameItem( QTreeWidgetItem *item, const QString &name );
    void resetTree();
    void emitChanged();

  signals:

    void scopeChanged();

  protected:
    void keyPressEvent( QKeyEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void drawRow( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QColor rowColor( int index ) const;
    void toggleContextExpanded( QTreeWidgetItem *item );
    void editNext( const QModelIndex &index );

    QModelIndex moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers ) override;

    QIcon mExpandIcon;

  private:

    VariableEditorDelegate* mEditorDelegate;
    int mEditableScopeIndex;
    QgsExpressionContext* mContext;
    QMap< QPair<int, QString>, QTreeWidgetItem* > mVariableToItem;
    QMap< int, QTreeWidgetItem* > mScopeToItem;

    void refreshScopeItems( QgsExpressionContextScope* scope, int scopeIndex );
    void refreshScopeVariables( QgsExpressionContextScope* scope, int scopeIndex );
};


class VariableEditorDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    VariableEditorDelegate( QObject *parent = nullptr, QgsVariableEditorTree *tree = nullptr )
        : QItemDelegate( parent )
        , mParentTree( tree )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget* widget, QAbstractItemModel* model,
                       const QModelIndex & index ) const override;
    void setEditorData( QWidget *, const QModelIndex & ) const override {}

  private:
    QgsVariableEditorTree *mParentTree;
};

/// @endcond

#endif //QGSVARIABLEEDITORWIDGET_H
