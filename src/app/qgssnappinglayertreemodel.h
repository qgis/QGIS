/***************************************************************************
  qgssnappinglayertreemodel.h - QgsSnappingLayerTreeModel

 ---------------------
 begin                : 31.8.2016
 copyright            : (C) 2016 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSNAPPINGLAYERTREEVIEW_H
#define QGSSNAPPINGLAYERTREEVIEW_H



#include <QSortFilterProxyModel>
#include <QItemDelegate>

#include "qgslayertreemodel.h"
#include "qgssnappingconfig.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsProject;


class APP_EXPORT QgsSnappingLayerDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsSnappingLayerDelegate( QgsMapCanvas *canvas, QObject *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  private slots:
    void onScaleChanged();

  private:
    QgsMapCanvas *mCanvas = nullptr;
};


class APP_EXPORT QgsSnappingLayerTreeModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    enum Columns
    {
      LayerColumn = 0,
      TypeColumn,
      ToleranceColumn,
      UnitsColumn,
      AvoidIntersectionColumn,
      MinScaleColumn,
      MaxScaleColumn
    };

    QgsSnappingLayerTreeModel( QgsProject *project, QgsMapCanvas *canvas, QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QModelIndex sibling( int row, int column, const QModelIndex &idx ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

    QgsLayerTreeModel *layerTreeModel() const;
    void setLayerTreeModel( QgsLayerTreeModel *layerTreeModel );
    void resetLayerTreeModel() {  beginResetModel(); endResetModel(); }

    QgsVectorLayer *vectorLayer( const QModelIndex &idx ) const;

  public slots:
    void setFilterText( const QString &filterText = QString() );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private slots:
    void onSnappingSettingsChanged();

  private:
    bool nodeShown( QgsLayerTreeNode *node ) const;

    QgsProject *mProject = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QString mFilterText;
    QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> mIndividualLayerSettings;
    QgsLayerTreeModel *mLayerTreeModel = nullptr;

    void hasRowchanged( QgsLayerTreeNode *node, const QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> &oldSettings );
};

class SnappingLayerDelegateTypeMenu: public QMenu
{
    Q_OBJECT

  public:
    SnappingLayerDelegateTypeMenu( const QString &title, QWidget *parent = nullptr )
      : QMenu( title, parent ) {}

    void mouseReleaseEvent( QMouseEvent *e )
    {
      QAction *action = activeAction();
      if ( action )
        action->trigger();
      else
        QMenu::mouseReleaseEvent( e );
    }

    // set focus to parent so that mTypeButton is not displayed
    void hideEvent( QHideEvent *e )
    {
      qobject_cast<QWidget *>( parent() )->setFocus();
      QMenu::hideEvent( e );
    }
};

#endif // QGSSNAPPINGLAYERTREEVIEW_H
