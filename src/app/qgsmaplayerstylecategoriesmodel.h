/***************************************************************************
  qgsmaplayerstylecategoriesmodel.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSTYLECATEGORIESMODEL_H
#define QGSMAPLAYERSTYLECATEGORIESMODEL_H

#include <QAbstractListModel>

#include "qgsmaplayer.h"

class QgsMapLayerStyleCategoriesModel : public QAbstractListModel
{
  public:
    enum DataRole
    {
      ReadableCategory = Qt::UserRole + 1, //!< Translated and readable
      ToolTip,
      Icon,
    };

    explicit QgsMapLayerStyleCategoriesModel( QObject *parent = nullptr );

    //! reset the model data
    void setCategories( QgsMapLayer::StyleCategories categories );

    //! return the categories as defined in the model
    QgsMapLayer::StyleCategories categories() const;

    //! defines if the model should list the AllStyleCategories entry
    void setShowAllCategories( bool showAll );

    //! return the category for the given index
    QgsMapLayer::StyleCategory index2category( const QModelIndex &index ) const;

    int rowCount( const QModelIndex & = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex & ) const override;

  private:
    //! current data as flags
    QgsMapLayer::StyleCategories mCategories;
    //! map of existing categories
    QList<QgsMapLayer::StyleCategory> mCategoryList;
    //! display All categories on first line
    bool mShowAllCategories = false;
};

#endif // QGSMAPLAYERSTYLECATEGORIESMODEL_H
