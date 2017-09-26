/***************************************************************************
  qgssourceselectprovider.h - QgsSourceSelectProvider

 ---------------------
 begin                : 1.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSOURCESELECTPROVIDER_H
#define QGSSOURCESELECTPROVIDER_H


#include "qgis_gui.h"
#include "qgis.h"
#include "qgsguiutils.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

class QString;
class QWidget;

/** \ingroup gui
 * This is the interface for those who want to add entries to the QgsDataSourceManagerDialog
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSourceSelectProvider
{
  public:

    //! Provider ordering groups
    enum Ordering
    {
      OrderLocalProvider = 0, //!< Starting point for local file providers (e.g. OGR)
      OrderDatabaseProvider = 1000, //!< Starting point for database providers (e.g. Postgres)
      OrderRemoteProvider = 2000, //!< Starting point for remote (online) providers (e.g. WMS)
      OrderGeoCmsProvider = 3000, //!< Starting point for GeoCMS type providers (e.g. GeoNode)
      OrderOtherProvider = 4000, //!< Starting point for other providers (e.g. plugin based providers)
    };

    virtual ~QgsSourceSelectProvider() = default;

    //! Data Provider key
    virtual QString providerKey() const = 0;

    /** Source select provider name, this is useful to retrieve
     * a particular source select in case the provider has more
     * than one, it should be unique among all providers.
     *
     * The default implementation returns the providerKey()
     */
    virtual QString name() const { return providerKey(); }

    //! Text for the menu item entry, it will be visible to the user so make sure it's translatable
    virtual QString text() const = 0;

    /** Text for the tooltip menu item entry, it will be visible to the user so make sure it's translatable
     *
     * The default implementation returns an empty string.
     */
    virtual QString toolTip() const { return QString(); }

    //! Creates a new instance of an QIcon for the menu item entry
    virtual QIcon icon() const = 0;

    /** Ordering: the source select provider registry will be able to sort
     * the source selects (ascending) using this integer value
     */
    virtual int ordering( ) const { return OrderOtherProvider; }

    /** Create a new instance of QgsAbstractDataSourceWidget (or null).
     * Caller takes responsibility of deleting created.
     */
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const = 0 SIP_FACTORY;

};


#endif // QGSSOURCESELECTPROVIDER_H
