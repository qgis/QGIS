/***************************************************************************
    qgsrelationmanagerdialog.h
     --------------------------------------
    Date                 : 23.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONMANAGERDIALOG_H
#define QGSRELATIONMANAGERDIALOG_H

#include <QWidget>
#include "ui_qgsrelationmanagerdialogbase.h"
#include "qgis_app.h"

class QgsRelation;
class QgsPolymorphicRelation;
class QgsRelationManager;
class QgsRelationManagerTreeModel;
class QgsVectorLayer;

class APP_EXPORT QgsRelationManagerDialog : public QWidget, private Ui::QgsRelationManagerDialogBase
{
    Q_OBJECT

  public:
    explicit QgsRelationManagerDialog( QgsRelationManager *relationMgr, QWidget *parent = nullptr );

    void setLayers( const QList<QgsVectorLayer *> & );

    bool addRelation( const QgsRelation &rel );
    int addPolymorphicRelation( const QgsPolymorphicRelation &relation );
    QList< QgsRelation > relations();
    QList< QgsPolymorphicRelation > polymorphicRelations();

  private slots:
    void mBtnAddRelation_clicked();
    void mActionAddPolymorphicRelation_triggered();
    void mActionEditPolymorphicRelation_triggered();
    void mBtnDiscoverRelations_clicked();
    void mBtnRemoveRelation_clicked();
    void onSelectionChanged();

  private:
    bool addRelationPrivate( const QgsRelation &rel, QTreeWidgetItem *parentItem = nullptr );

    QgsRelationManager *mRelationManager = nullptr;
    QList< QgsVectorLayer * > mLayers;
    QString getUniqueId( const QString &idTmpl, const QString &ids ) const;
};

class RelationNameEditorDelegate: public QStyledItemDelegate
{
    Q_OBJECT
  public:
    RelationNameEditorDelegate( const QList<int> &editableColumns, QObject *parent = nullptr )
      : QStyledItemDelegate( parent )
      , mEditableColumns( editableColumns )
    {}

    virtual QWidget *createEditor( QWidget *parentWidget, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      if ( mEditableColumns.contains( index.column() ) )
        return QStyledItemDelegate::createEditor( parentWidget, option, index );

      return nullptr;
    }
  private:
    QList<int> mEditableColumns;
};

#endif // QGSRELATIONMANAGERDIALOG_H
