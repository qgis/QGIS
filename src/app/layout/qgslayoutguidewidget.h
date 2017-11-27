/***************************************************************************
                             qgslayoutguidewidget.h
                             ----------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTGUIDEWIDGET_H
#define QGSLAYOUTGUIDEWIDGET_H

#include "ui_qgslayoutguidewidgetbase.h"
#include "qgspanelwidget.h"
#include <QStyledItemDelegate>


class QgsLayoutView;
class QgsLayout;
class QgsLayoutGuideProxyModel;

class QgsLayoutGuideWidget: public QgsPanelWidget, private Ui::QgsLayoutGuideWidgetBase
{
    Q_OBJECT
  public:
    QgsLayoutGuideWidget( QWidget *parent, QgsLayout *layout, QgsLayoutView *layoutView );

  private slots:

    void addHorizontalGuide();
    void addVerticalGuide();

    void deleteHorizontalGuide();
    void deleteVerticalGuide();

    void pageChanged( int page );

    void clearAll();

    void applyToAll();

  private:

    QgsLayout *mLayout = nullptr;
    QgsLayoutGuideProxyModel *mHozProxyModel = nullptr;
    QgsLayoutGuideProxyModel *mVertProxyModel = nullptr;
    int mPage = 0;

};


class QgsLayoutGuidePositionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsLayoutGuidePositionDelegate( QObject *parent, QgsLayout *layout, QAbstractItemModel *model );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    void setModelData( const QModelIndex &index, const QVariant &value, int role ) const;

  private:

    QgsLayout *mLayout = nullptr;
    QAbstractItemModel *mModel = nullptr;
};

class QgsLayoutGuideUnitDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    QgsLayoutGuideUnitDelegate( QObject *parent, QgsLayout *layout, QAbstractItemModel *model );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    void setModelData( const QModelIndex &index, const QVariant &value, int role ) const;

  private:

    QgsLayout *mLayout = nullptr;

    QAbstractItemModel *mModel = nullptr;
};

#endif // QGSLAYOUTGUIDEWIDGET_H
