/***************************************************************************
   qgsfieldproxymodel.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDPROXYMODEL_H
#define QGSFIELDPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "qgis_core.h"

#include "qgis_sip.h"

class QgsFieldModel;

/**
 * \ingroup core
 * \brief The QgsFieldProxyModel class provides an easy to use model to display the list of fields of a layer.
 */
class CORE_EXPORT QgsFieldProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    //! Field type filters
    enum Filter SIP_ENUM_BASETYPE( IntFlag )
    {
      String = 1 << 0, //!< String fields
      Int = 1 << 1, //!< Integer fields
      LongLong = 1 << 2, //!< Longlong fields
      Double = 1 << 3, //!< Double fields
      Numeric = Int | LongLong | Double, //!< All numeric fields
      Date = 1 << 4, //!< Date or datetime fields
      Time = 1 << 5, //!< Time fields
      HideReadOnly = 1 << 6,  //!< Hide read-only fields
      DateTime = 1 << 7, //!< Datetime fields
      Binary = 1 << 8, //!< Binary fields, since QGIS 3.34
      Boolean = 1 << 9, //!< Boolean fields, since QGIS 3.34
      OriginProvider = 1 << 10, //!< Fields with a provider origin, since QGIS 3.38
      AllTypes = Numeric | Date | String | Time | DateTime | Binary | Boolean, //!< All field types
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * \brief QgsFieldProxModel creates a proxy model with a QgsFieldModel as source model.
     * It can be used to filter the fields based on their types.
     */
    explicit QgsFieldProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns the QgsFieldModel used in this QSortFilterProxyModel
    QgsFieldModel *sourceFieldModel() { return mModel; }

    /**
     * Set flags that affect how fields are filtered in the model.
     * \param filters are Filter flags
     * \see filters()
     */
    QgsFieldProxyModel *setFilters( QgsFieldProxyModel::Filters filters );

    /**
     * Returns the filters controlling displayed fields.
     * \see setFilters()
     */
    const Filters &filters() const { return mFilters; }

  private:
    Filters mFilters;
    QgsFieldModel *mModel = nullptr;

    //! Returns TRUE if the specified index represents a read only field
    bool isReadOnly( const QModelIndex &index ) const;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFieldProxyModel::Filters )

#endif // QGSFIELDPROXYMODEL_H
