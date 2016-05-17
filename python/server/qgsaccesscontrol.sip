/***************************************************************************
                          qgsaccesscontrol.sip
                          --------------------
 Access control helper for Qgis Server plugins

  begin                : 2015-05-19
  copyright            : (C) 2015 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * \class QgsAccessControl
 * \brief Class defining access control helper for QGIS Server.
 */
class QgsAccessControl : QgsFeatureFilterProvider
{
%TypeHeaderCode
#include "qgsaccesscontrol.h"
#include "qgsaccesscontrolfilter.h"

#include <QMultiMap>
%End

};
