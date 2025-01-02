/***************************************************************************
    qgsstacitemlistmodel.h
    ---------------------
    begin                : November 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACITEMLISTMODEL_H
#define QGSSTACITEMLISTMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsStacItem;
class QgsStacCollection;


class QgsStacItemListModel : public QAbstractListModel
{
    Q_OBJECT
  public:
    enum Role
    {
      StacObject = Qt::UserRole + 1,
      Id,
      Title,
      Thumbnail,
      Uris,
      MediaTypes,
      Geometry,
      Extent,
      Collection,
      Formats,
    };

    QgsStacItemListModel( QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    void clear();

    //! Builds collection dictionary. Does not take ownership
    void setCollections( const QVector<QgsStacCollection *> &collections );
    //! Add items to the model. Takes ownership
    void addItems( const QVector<QgsStacItem *> &items );
    //! Returns all items in the model. Does not transfer ownership
    QVector<QgsStacItem *> items() const;

  private:
    QVector<QgsStacItem *> mItems;
    QMap<QString, QPixmap> mThumbnails;
    QMap<QString, QString> mCollections;
};


class QgsStacItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsStacItemDelegate( QObject *parent = nullptr );
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    int mRoundedRectSizePixels = 5;
};

///@endcond
#endif // QGSSTACITEMLISTMODEL_H
