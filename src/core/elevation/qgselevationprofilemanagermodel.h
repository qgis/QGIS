/***************************************************************************
    qgselevationprofilemanagermodel.cpp
    --------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILEMANAGERMODEL_H
#define QGSELEVATIONPROFILEMANAGERMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include "qgsprojectstoredobjectmanagermodel.h"


class QgsProject;
class QgsElevationProfile;
class QgsElevationProfileManager;


/**
 * \ingroup core
 * \class QgsElevationProfileManagerModel
 *
 * \brief List model representing the elevation profiles available in a
 * elevation profile manager.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsElevationProfileManagerModel : public QgsProjectStoredObjectManagerModelBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsElevationProfileManagerModel : public QgsProjectStoredObjectManagerModel< QgsElevationProfile >
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
      ElevationProfile = Qt::UserRole + 1, //!< Elevation profile object
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsElevationProfileManagerModel, showing the elevation profiles from the specified \a manager.
     */
    explicit QgsElevationProfileManagerModel( QgsElevationProfileManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the profile at the corresponding \a index.
     * \see indexFromProfile()
     */
    QgsElevationProfile *profileFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a \a profile.
     * \see profileFromIndex()
     */
    QModelIndex indexFromProfile( QgsElevationProfile *profile ) const;

};


/**
 * \ingroup core
 * \class QgsElevationProfileManagerProxyModel
 *
 * \brief QSortFilterProxyModel subclass for QgsElevationProfileManagerModel.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsElevationProfileManagerProxyModel : public QgsProjectStoredObjectManagerProxyModelBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsElevationProfileManagerProxyModel : public QgsProjectStoredObjectManagerProxyModel< QgsElevationProfile >
{
#endif
    Q_OBJECT
  public:

    /**
     * Constructor for QgsElevationProfileManagerProxyModel.
     */
    explicit QgsElevationProfileManagerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

};

#endif // QGSELEVATIONPROFILEMANAGERMODEL_H
