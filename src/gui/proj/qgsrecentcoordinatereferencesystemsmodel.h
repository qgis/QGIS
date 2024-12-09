/***************************************************************************
                    qgsrecentcoordinatereferencesystemsmodel.h
                    -------------------
    begin                : January 2024
    copyright            : (C) 2024 by Nyall Dawson
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
#ifndef QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H
#define QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgscoordinatereferencesystemmodel.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>

class QgsCoordinateReferenceSystem;

/**
 * \class QgsRecentCoordinateReferenceSystemsModel
 * \ingroup core
 * \brief A model for display of recently used coordinate reference systems.
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsRecentCoordinateReferenceSystemsModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    // *INDENT-OFF*

    /**
     * Custom model roles.
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRecentCoordinateReferenceSystemsModel, Roles ) : int
    {
      Crs SIP_MONKEYPATCH_COMPAT_NAME( RoleCrs ) = Qt::UserRole, //!< Coordinate reference system
      AuthId SIP_MONKEYPATCH_COMPAT_NAME( RoleAuthId ),          //!< CRS authority ID
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsRecentCoordinateReferenceSystemsModel, with the specified \a parent object.
     */
    QgsRecentCoordinateReferenceSystemsModel( QObject *parent SIP_TRANSFERTHIS = nullptr, int subclassColumnCount SIP_PYARGREMOVE = 1 );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    /**
     * Returns the CRS for the corresponding \a index.
     *
     * Returns an invalid CRS if the index is not valid.
     */
    QgsCoordinateReferenceSystem crs( const QModelIndex &index ) const;

  private slots:

    void recentCrsPushed( const QgsCoordinateReferenceSystem &crs );
    void recentCrsRemoved( const QgsCoordinateReferenceSystem &crs );
    void recentCrsCleared();

  private:
    QList<QgsCoordinateReferenceSystem> mCrs;
    int mColumnCount = 1;
};


/**
 * \brief A sort/filter proxy model for recent coordinate reference systems.
 *
 * \ingroup gui
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsRecentCoordinateReferenceSystemsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsRecentCoordinateReferenceSystemsProxyModel, with the given \a parent object.
     */
    explicit QgsRecentCoordinateReferenceSystemsProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr, int subclassColumnCount SIP_PYARGREMOVE = 1 );

    /**
     * Returns the underlying source model.
     */
    QgsRecentCoordinateReferenceSystemsModel *recentCoordinateReferenceSystemsModel();

    /**
     * Returns the underlying source model.
     * \note Not available in Python bindings
     */
    const QgsRecentCoordinateReferenceSystemsModel *recentCoordinateReferenceSystemsModel() const SIP_SKIP;

    /**
     * Set \a filters that affect how CRS are filtered.
     */
    void setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters );

    /**
     * Sets whether deprecated CRS should be filtered from the results.
    */
    void setFilterDeprecated( bool filter );

    /**
     * Sets a \a filter string, such that only coordinate reference systems matching the
     * specified string will be shown.
    */
    void setFilterString( const QString &filter );

    /**
     * Returns any filters that affect how CRS are filtered.
     * \see setFilters()
     */
    QgsCoordinateReferenceSystemProxyModel::Filters filters() const { return mFilters; }

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Returns the CRS for the corresponding \a index.
     *
     * Returns an invalid CRS if the index is not valid.
     */
    QgsCoordinateReferenceSystem crs( const QModelIndex &index ) const;

  private:
    QgsRecentCoordinateReferenceSystemsModel *mModel = nullptr;
    QgsCoordinateReferenceSystemProxyModel::Filters mFilters = QgsCoordinateReferenceSystemProxyModel::Filters();
    bool mFilterDeprecated = false;
    QString mFilterString;
};


#endif // QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H
