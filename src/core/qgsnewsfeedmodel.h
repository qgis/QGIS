/***************************************************************************
    qgsnewsfeedmodel.h
    ------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWSFEEDMODEL_H
#define QGSNEWSFEEDMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsnewsfeedparser.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

/**
 * \ingroup core
 * A model for published QGIS news feeds.
 *
 * This class is designed to work with QgsNewsFeedParser, for displaying
 * feeds from a https://github.com/elpaso/qgis-feed server instance.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsNewsFeedModel : public QAbstractItemModel
{
    Q_OBJECT
  public:

    /**
     * Custom model roles.
     */
    enum Role
    {
      Key = Qt::UserRole + 1, //!< Entry unique key
      Title, //!< Entry title
      Content, //!< Entry content
      ImageUrl, //!< Optional entry image URL
      Image, //!< Optional entry image
      Link, //!< Optional entry URL link
      Sticky, //!< Whether entry is sticky
    };

    /**
     * Constructor for QgsNewsFeedModel, with the specified \a parent object.
     *
     * The \a parser argument must specify a valid QgsNewsFeedParser object, which
     * must exist for the lifetime of this model.
     */
    QgsNewsFeedModel( QgsNewsFeedParser *parser, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

  private slots:

    void onEntryAdded( const QgsNewsFeedParser::Entry &entry );
    void onEntryRemoved( const QgsNewsFeedParser::Entry &entry );
    void onImageFetched( int key, const QPixmap &pixmap );

  private:

    QgsNewsFeedParser *mParser = nullptr;
    QList< QgsNewsFeedParser::Entry > mEntries;
};

/**
 * \ingroup core
 * A proxy model for use with QgsNewsFeedModel.
 *
 * QgsNewsFeedProxyModel applies custom sorting to the entries in a QgsNewsFeedModel.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsNewsFeedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsNewsFeedProxyModel, with the specified \a parent object.
     *
     * The \a parser argument must specify a valid QgsNewsFeedParser object, which
     * must exist for the lifetime of this model.
     */
    explicit QgsNewsFeedProxyModel( QgsNewsFeedParser *parser, QObject *parent SIP_TRANSFERTHIS = nullptr );

  protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:

    QgsNewsFeedModel *mModel = nullptr;

};

#endif // QGSNEWSFEEDMODEL_H
