/***************************************************************************
                          qgsgdalguiutils.h
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALGUIUTILS_H
#define QGSGDALGUIUTILS_H

#include <QString>
#include "qgis_gui.h"

class QWidget;
class QgsGdalOption;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsGdalGuiUtils
 * \brief Utility functions for working with GDAL in GUI classes.
 *
 * \note not available in Python bindings
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsGdalGuiUtils
{
  public:
    /**
     * Create database uri from connection parameters
     * \note not available in python bindings
     */
    static QString createDatabaseURI( const QString &connectionType, const QString &host, const QString &database, QString port, const QString &configId, QString username, QString password, bool expandAuthConfig = false );

    /**
     * Create protocol uri from connection parameters
     * \note not available in python bindings
     */
    static QString createProtocolURI( const QString &type, const QString &url, const QString &configId, const QString &username, const QString &password, bool expandAuthConfig = false );

    /**
     * Creates a new widget for configuration a GDAL \a option.
     *
     * If \a includeDefaultChoices is TRUE then the widget will include an option
     * for the default value.
     */
    static QWidget *createWidgetForOption( const QgsGdalOption &option, QWidget *parent = nullptr, bool includeDefaultChoices = false );
};

#endif // QGSGDALGUIUTILS_H
