/***************************************************************************
                         qgslocatoroptionswidget.h
                         --------------------------
    begin                : May 2017
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

#ifndef QGSLOCATOROPTIONSWIDGET_H
#define QGSLOCATOROPTIONSWIDGET_H

#include <QItemDelegate>
#include <QTreeView>

#include "qgslocatorfilter.h"
#include "qgslocator.h"

class QToolButton;

class QgsLocatorFiltersModel;
class QgsLocatorWidget;


class QgsLocatorOptionsWidget : public QTreeView
{
    Q_OBJECT

  public:

    QgsLocatorOptionsWidget( QgsLocatorWidget *locator, QWidget *parent = nullptr );

  public slots:

    void commitChanges();

  protected slots:
    void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles ) override;

  private:
    QgsLocatorWidget *mLocatorWidget = nullptr;
    QgsLocator *mLocator = nullptr;
    QgsLocatorFiltersModel *mModel = nullptr;
};


/**
 * \class QgsLocatorFiltersModel
 * \ingroup app
 * An list model for displaying available filters and configuring them.
 * \since QGIS 3.0
 */
class QgsLocatorFiltersModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum Role
    {
      ResultDataRole = Qt::UserRole + 1, //!< QgsLocatorResult data
    };

    enum Columns
    {
      Name = 0,
      Prefix,
      Active,
      Default,
      Config
    };

    /**
     * Constructor for QgsLocatorFiltersModel.
     */
    QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent = nullptr );

    QWidget *configButton( const QModelIndex &index, QWidget *parent = nullptr ) const;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;

    QgsLocatorFilter *filterForIndex( const QModelIndex &index ) const;

  public slots:

    void commitChanges();

  private:

    QgsLocator *mLocator = nullptr;

    // changes are deferred to support cancellation
    QHash< QgsLocatorFilter *, QString > mPrefixes;
    QHash< QgsLocatorFilter *, bool > mEnabledChanges;
    QHash< QgsLocatorFilter *, bool > mDefaultChanges;

    int mIconSize, mRowSize;

};

#endif // QGSLOCATOROPTIONSWIDGET_H


