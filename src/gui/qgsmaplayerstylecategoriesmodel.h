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

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgis_gui.h"
#include <QItemDelegate>
#include <QLabel>

/**
 * \ingroup gui
 * \brief Model for layer style categories
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMapLayerStyleCategoriesModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    //! Custom model roles
    enum class Role : int
    {
      NameRole = Qt::UserRole + 1,
    };

    /**
     * Constructor for QgsMapLayerStyleCategoriesModel, for the specified layer \a type.
     */
    explicit QgsMapLayerStyleCategoriesModel( Qgis::LayerType type, QObject *parent = nullptr );

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

/**
* \ingroup gui
* \class QgsCategoryDisplayLabelDelegate
* \brief A label delegate being able to display html encoded content
* \since QGIS 3.40
*/
class GUI_EXPORT QgsCategoryDisplayLabelDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsCategoryDisplayLabelDelegate( QObject *parent = nullptr );

  protected:
    void drawDisplay( QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

#endif // QGSMAPLAYERSTYLECATEGORIESMODEL_H
