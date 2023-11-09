/***************************************************************************
                         qgsdbfilterproxymodel.h  -  description
                         -----------------------
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
 * \brief A class that implements a custom filter and can be used
 * as a proxy for QgsDbTableModel
 * \deprecated since QGIS 3.24
 * \since QGIS 3.0 QSortFilterProxyModel with native recursive filtering can be used instead
*/
class CORE_DEPRECATED_EXPORT QgsDatabaseFilterProxyModel : public QSortFilterProxyModel SIP_DEPRECATED
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
