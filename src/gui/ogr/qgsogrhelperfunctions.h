/***************************************************************************
                          qgsogrhelperfunctions.h
    helper functions to create ogr uris for database and protocol drivers
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include "qgis_gui.h"

#define SIP_NO_FILE

/**
 * CreateDatabaseURI
 * \brief Create database uri from connection parameters
 * \note not available in python bindings
 */
QString GUI_EXPORT createDatabaseURI( const QString &connectionType, const QString &host, const QString &database, QString port, const QString &configId, QString username, QString password, bool expandAuthConfig = false );

/**
 * CreateProtocolURI
 * \brief Create protocol uri from connection parameters
 * \note not available in python bindings
 */
QString GUI_EXPORT createProtocolURI( const QString &type, const QString &url, const QString &configId, const QString &username, const QString &password, bool expandAuthConfig = false );
