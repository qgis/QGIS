/***************************************************************************
  qgsauthenticationwidget.h - QgsAuthenticationWidget

 ---------------------
 begin                : 28.9.2017
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
#ifndef QGSAUTHENTICATIONWIDGET_H
#define QGSAUTHENTICATIONWIDGET_H

#include "qgis_gui.h"
#include "qgis.h"

#include "ui_qgsauthenticationwidget.h"

#include <QWidget>

/** \ingroup gui
 * Widget for entering authentication credentials both in the form username/password
 * and by using QGIS Authentication Database and its authentication configurations.
 */
class GUI_EXPORT QgsAuthenticationWidget : public QWidget, private Ui::QgsAuthenticationWidget
{

    Q_OBJECT

  public:

    /**
     * Create a dialog for setting an associated authentication config, either
     * from existing configs, or creating/removing them from auth database
     * \param parent Parent widget
     * \param dataprovider The key of the calling layer provider, if applicable
     */
    explicit QgsAuthenticationWidget( QWidget *parent SIP_TRANSFERTHIS = 0, const QString &dataprovider = QString() );

};

#endif // QGSAUTHENTICATIONWIDGET_H
