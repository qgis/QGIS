/***************************************************************************
    qgslayoutmanagermodel.h
    ------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTMANAGERMODEL_H
#define QGSLAYOUTMANAGERMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include "qgsprojectstoredobjectmanagermodel.h"


class QgsProject;
class QgsPrintLayout;
class QgsStyleEntityVisitorInterface;
class QgsLayoutManager;
class QgsMasterLayoutInterface;


/**
 * \ingroup core
 * \class QgsLayoutManagerModel
 *
 * \brief List model representing the print layouts and reports available in a
 * layout manager.
 *
 * \since QGIS 3.8
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsLayoutManagerModel : public QgsProjectStoredObjectManagerModelBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsLayoutManagerModel : public QgsProjectStoredObjectManagerModel< QgsMasterLayoutInterface >
{
#endif
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsLayoutManagerModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLayoutManagerModel, Role ) : int
    {
      Layout SIP_MONKEYPATCH_COMPAT_NAME(LayoutRole) = Qt::UserRole + 1, //!< Layout object
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsLayoutManagerModel, showing the layouts from the specified \a manager.
     */
    explicit QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the layout at the corresponding \a index.
     * \see indexFromLayout()
     */
    QgsMasterLayoutInterface *layoutFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a \a layout.
     * \see layoutFromIndex()
     */
    QModelIndex indexFromLayout( QgsMasterLayoutInterface *layout ) const;

    /**
     * Sets whether an optional empty layout ("not set") option is present in the model.
     * \see allowEmptyLayout()
     */
    void setAllowEmptyLayout( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty layout ("not set") choice.
     * \see setAllowEmptyLayout()
     */
    bool allowEmptyLayout() const { return allowEmptyObject(); }

};


/**
 * \ingroup core
 * \class QgsLayoutManagerProxyModel
 *
 * \brief QSortFilterProxyModel subclass for QgsLayoutManagerModel.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsLayoutManagerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    //! Available filter flags for filtering the model
    enum Filter SIP_ENUM_BASETYPE( IntFlag )
    {
      FilterPrintLayouts = 1 << 1, //!< Includes print layouts
      FilterReports = 1 << 2, //!< Includes reports
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsLayoutManagerProxyModel.
     */
    explicit QgsLayoutManagerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Returns the current filters used for filtering available layouts.
     *
     * \see setFilters()
     */
    QgsLayoutManagerProxyModel::Filters filters() const;

    /**
     * Sets the current \a filters used for filtering available layouts.
     *
     * \see filters()
     */
    void setFilters( QgsLayoutManagerProxyModel::Filters filters );

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     * \since QGIS 3.12
     */
    QString filterString() const { return mFilterString; }

  public slots:

    /**
     * Sets a \a filter string, such that only layouts with names containing the
     * specified string will be shown.
     *
     * \see filterString()
     * \since QGIS 3.12
    */
    void setFilterString( const QString &filter );

  private:

    Filters mFilters = Filters( FilterPrintLayouts | FilterReports );

    QString mFilterString;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayoutManagerProxyModel::Filters )
#endif // QGSLAYOUTMANAGERMODEL_H
