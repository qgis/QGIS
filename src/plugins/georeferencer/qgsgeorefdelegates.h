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
    QgsNonEditableDelegate( QWidget *parent = 0 );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const
    {
      Q_UNUSED( parent );
      Q_UNUSED( option );
      Q_UNUSED( index );
      return 0;
    }
};

class QgsDmsAndDdDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsDmsAndDdDelegate( QWidget *parent = 0 );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const;

  private:
    QString dmsToDD( QString dms ) const;
};

class QgsCoordDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsCoordDelegate( QWidget *parent = 0 );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;

//  void setEditorData(QWidget *editor, const QModelIndex &index);
//  void setModelData(QWidget *editor, QAbstractItemModel *model,
//                    const QModelIndex &index);
//
//  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
//                            const QModelIndex &index);
};

#endif // QGSDELEGATES_H
