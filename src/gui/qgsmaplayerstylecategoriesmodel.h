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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QAbstractListModel>

#include "qgsmaplayer.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief Model for layer style categories
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMapLayerStyleCategoriesModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapLayerStyleCategoriesModel, for the specified layer \a type.
     */
    explicit QgsMapLayerStyleCategoriesModel( QgsMapLayerType type, QObject *parent = nullptr );

    //! Reset the model data
    void setCategories( QgsMapLayer::StyleCategories categories );

    //! Returns the categories as defined in the model
    QgsMapLayer::StyleCategories categories() const;

    //! Defines if the model should list the AllStyleCategories entry
    void setShowAllCategories( bool showAll );

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
