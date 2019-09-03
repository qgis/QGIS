/***************************************************************************
    qgsbookmarkmodel.h
    ------------------
    Date                 : Septemeber 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBOOKMARKMODEL_H
#define QGSBOOKMARKMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class QgsBookmarkManager;
class QgsBookmark;

/**
 * \ingroup core
 * \class QgsBookmarkManagerModel
 *
 * \brief Implements a model for the contents of QgsBookmarkManager objects.
 *
 * QgsBookmarkModel provides a Qt table model for displaying and manipulating
 * the bookmarks managed by a QgsBookmarkManager object. The model requires
 * both a main manager (usually the application bookmark manager, accessed
 * via QgsApplication::bookmarkManager()) and a project-based manager. The resultant
 * model data is a merge of the bookmarks stored in both managers.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsBookmarkManagerModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum CustomRoles
    {
      RoleExtent = Qt::UserRole, //!< Bookmark extent as a QgsReferencedRectangle
      RoleName, //!< Bookmark name
      RoleId, //!< Bookmark ID
      RoleGroup, //!< Bookmark group
    };

    //! Model columns
    enum Columns
    {
      ColumnName, //!< Name column
      ColumnGroup, //!< Group column
      ColumnXMin, //!< Extent x-minimum
      ColumnYMin, //!< Extent y-minimum
      ColumnXMax, //!< Extent x-maximum
      ColumnYMax, //!< Extent y-maxnimum
      ColumnCrs, //!< CRS of extent
      ColumnStore, //!< Manager storing the bookmark (TRUE if stored in project bookmark manager)
    };

    /**
     * Constructor for QgsBookmarkManagerModel, associated with a main \a manager
     * (usually the application bookmark manager, accessed via QgsApplication::bookmarkManager())
     * and a secondary \a projectManager (a project based bookmark manager).
     */
    QgsBookmarkManagerModel( QgsBookmarkManager *manager, QgsBookmarkManager *projectManager = nullptr, QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  private slots:
    void bookmarkAboutToBeAdded( const QString &id );
    void bookmarkAdded( const QString &id );
    void bookmarkAboutToBeRemoved( const QString &id );
    void bookmarkRemoved( const QString &id );
    void bookmarkChanged( const QString &id );

  private:
    bool mBlocked = false;
    QgsBookmarkManager *mManager = nullptr;
    QgsBookmarkManager *mProjectManager = nullptr;
    QgsBookmark bookmarkForIndex( const QModelIndex &index ) const;

};

/**
 * \ingroup core
 * \class QgsBookmarkManagerProxyModel
 *
 * \brief A QSortFilterProxyModel subclass for sorting the entries in a QgsBookmarkManagerModel.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsBookmarkManagerProxyModel : public QSortFilterProxyModel
{
  public:

    /**
     * Constructor for QgsBookmarkManagerProxyModel, associated with a main \a manager
     * (usually the application bookmark manager, accessed via QgsApplication::bookmarkManager())
     * and a secondary \a projectManager (a project based bookmark manager).
     */
    QgsBookmarkManagerProxyModel( QgsBookmarkManager *manager, QgsBookmarkManager *projectManager = nullptr, QObject *parent SIP_TRANSFERTHIS = nullptr );

  private:

    QgsBookmarkManagerModel *mModel = nullptr;
};

#endif // QGSBOOKMARKMODEL_H
