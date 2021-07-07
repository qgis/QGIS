/***************************************************************************
                             qgsmaplayerserverproperties.cpp
                              ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerserverproperties.h"
#include "qgsmaplayer.h"

#include <QDomNode>

QgsMapLayerServerProperties::QgsMapLayerServerProperties( QgsMapLayer *layer )
  : mLayer( layer )
{
}
