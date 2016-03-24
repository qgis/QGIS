/***************************************************************************
 *                           qgsserverinterface.cpp
 *                            Abstract base class for interfaces to functions in QgsServer
 *                           -------------------
 *      begin                : 2014-29-09
 *      copyright            : (C) 2014 by Alessandro Pasotti
 *      email                : a dot pasotti at itopen dot com
 * ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "qgsserverinterface.h"

/** Constructor */
QgsServerInterface::QgsServerInterface():
    mConfigFilePath( QString() )
{
}

/** Destructor */
QgsServerInterface::~QgsServerInterface()
{
}
