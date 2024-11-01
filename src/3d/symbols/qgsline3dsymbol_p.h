/***************************************************************************
  qgsline3dsymbol_p.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINE3DSYMBOL_P_H
#define QGSLINE3DSYMBOL_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgsfeature3dhandler_p.h"

class QgsLine3DSymbol;
class QgsAbstract3DSymbol;

namespace Qgs3DSymbolImpl
{
  //! factory method for QgsLine3DSymbol
  QgsFeature3DHandler *handlerForLine3DSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol );
} // namespace Qgs3DSymbolImpl

/// @endcond

#endif // QGSLINE3DSYMBOL_P_H
