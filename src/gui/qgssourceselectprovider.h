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
#include "qgsabstractdatasourcewidget.h"

class QString;

/** \ingroup gui
 * This is the interface for those who want to add entries to the \see QgsDataSourceManagerDialog
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSourceSelectProvider
{
  public:
    virtual ~QgsSourceSelectProvider() = default;

    //! Provider key
    virtual QString providerKey() const = 0;

    //! Text for the menu item entry
    virtual QString text() const = 0;

    //! Creates a new instance of an QIcon for the menu item entry
    //! Caller takes responsibility of deleting created.
    virtual QIcon icon() const = 0 SIP_FACTORY;

    //! Create a new instance of QgsAbstractDataSourceWidget (or null).
    //! Caller takes responsibility of deleting created.
    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( ) = 0 SIP_FACTORY;

};


#endif // QGSSOURCESELECTPROVIDER_H
