/***************************************************************************
                         qgsdbfilterproxymodel.h  -  description
                         -----------------------
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDBFILTERPROXYMODEL_H
#define QGSDBFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "qgis_sip.h"

#include "qgis_core.h"

/**
 * \class QgsDatabaseFilterProxyModel
 * \ingroup core
 * A class that implements a custom filter and can be used
 * as a proxy for QgsDbTableModel
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsDatabaseFilterProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDatabaseFilterProxyModel.
     */
    QgsDatabaseFilterProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Calls QSortFilterProxyModel::setFilterWildcard and triggers update
    void _setFilterWildcard( const QString &pattern );

    //! Calls QSortFilterProxyModel::setFilterRegExp and triggers update
    void _setFilterRegExp( const QString &pattern );

  protected:
    bool filterAcceptsRow( int row, const QModelIndex &source_parent ) const override;
};

#endif
