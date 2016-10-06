/***************************************************************************
     qgsgeorefdelegates.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDELEGATES_H
#define QGSDELEGATES_H

#include <QStyledItemDelegate>

class QgsNonEditableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsNonEditableDelegate( QWidget *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override
    {
      Q_UNUSED( parent );
      Q_UNUSED( option );
      Q_UNUSED( index );
      return nullptr;
    }
};

class QgsDmsAndDdDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsDmsAndDdDelegate( QWidget *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const override;

  private:
    double dmsToDD( const QString& dms ) const;
};

class QgsCoordDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsCoordDelegate( QWidget *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const override;
};

#endif // QGSDELEGATES_H
