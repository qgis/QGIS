/***************************************************************************
    qgsselectivemaskingsourcesetmanagermodel.cpp
    --------------------
    Date                 : January 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSELECTIVEMASKINGSOURCESETMANAGERMODEL_H
#define QGSSELECTIVEMASKINGSOURCESETMANAGERMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsprojectstoredobjectmanagermodel.h"

#include <QAbstractListModel>
#include <QObject>
#include <QSortFilterProxyModel>

class QgsProject;
class QgsSelectiveMaskingSourceSet;
class QgsSelectiveMaskingSourceSetManager;


/**
 * \ingroup core
 * \class QgsSelectiveMaskingSourceSetManagerModel
 *
 * \brief List model representing the selective masking source sets available in a
 * selective masking source sets manager.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsSelectiveMaskingSourceSetManagerModel : public QgsProjectStoredObjectManagerModelBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsSelectiveMaskingSourceSetManagerModel : public QgsProjectStoredObjectManagerModel< QgsSelectiveMaskingSourceSet >
{
#endif
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     */
    enum class CustomRole : int
    {
      Object = Qt::UserRole + 1, //!< Object
      IsEmptyObject = Qt::UserRole + 2, //!< TRUE if row represents the empty object
      SetId = Qt::UserRole + 3, //!< Selective masking source set unique ID
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsSelectiveMaskingSourceSetManagerModel, showing the sets from the specified \a manager.
     */
    explicit QgsSelectiveMaskingSourceSetManagerModel( QgsSelectiveMaskingSourceSetManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the set at the corresponding \a index.
     * \see indexFromSet()
     */
    QgsSelectiveMaskingSourceSet *setFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a \a set.
     * \see setFromIndex()
     */
    QModelIndex indexFromSet( QgsSelectiveMaskingSourceSet *set ) const;

    QVariant data( const QModelIndex &index, int role ) const override;

};


/**
 * \ingroup core
 * \class QgsSelectiveMaskingSourceSetManagerProxyModel
 *
 * \brief QSortFilterProxyModel subclass for QgsSelectiveMaskingSourceSetManagerModel.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsSelectiveMaskingSourceSetManagerProxyModel : public QgsProjectStoredObjectManagerProxyModelBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsSelectiveMaskingSourceSetManagerProxyModel : public QgsProjectStoredObjectManagerProxyModel< QgsSelectiveMaskingSourceSet >
{
#endif
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSelectiveMaskingSourceSetManagerProxyModel.
     */
    explicit QgsSelectiveMaskingSourceSetManagerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

};

#endif // QGSSELECTIVEMASKINGSOURCESETMANAGERMODEL_H
