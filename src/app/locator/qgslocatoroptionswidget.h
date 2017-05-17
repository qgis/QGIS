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

#include "qgslocatorfilter.h"
#include "qgslocator.h"
#include "ui_qgslocatoroptionswidgetbase.h"

class QgsLocatorFiltersModel;

class QgsLocatorOptionsWidget : public QWidget, private Ui::QgsLocatorOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsLocatorOptionsWidget( QgsLocator *locator, QWidget *parent = nullptr );

  private:

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
      Default
    };

    /**
     * Constructor for QgsLocatorFiltersModel.
     */
    QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant zheaderData( int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole ) const;
  private:

    QgsLocator *mLocator = nullptr;
};

#endif // QGSLOCATOROPTIONSWIDGET_H


