/***************************************************************************
                          qgsseerverfilter.h
 Server I/O filters class for Qgis Mapserver for use by plugins
                             -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
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
 * QgsServerFilter
 * Class defining I/O filters for Qgis Mapserver and
 * implemented in plugins
 *
 */

class QgsServerFilter
{
%TypeHeaderCode
#include "qgsserverfilter.h"
#include "qgsserverinterface.h"
%End

public:

   /** Constructor */
   QgsServerFilter( QgsServerInterface* serverInterface /Transfer/);
   /** Destructor */
   virtual ~QgsServerFilter();
   QgsServerInterface* serverInterface( );
   virtual void requestReady();
   virtual void responseReady();

};
