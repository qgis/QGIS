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
 * \class QgsServerFilter
 * \brief Class defining I/O filters for Qgis Mapserver and
 * implemented in plugins.
 *
 * Filters can define any (or none) of the following hooks:
 *  * requestReady() - called when request is ready
 *  * responseReady() - called when the response is ready
 */

class QgsServerFilter
{
%TypeHeaderCode
#include "qgsserverfilter.h"
#include "qgsserverinterface.h"
%End

public:

   /** Constructor
    * QgsServerInterface passed to plugins constructors
    * and must be passed to QgsServerFilter instances.
    */
   QgsServerFilter( QgsServerInterface* serverInterface /Transfer/);
   /** Destructor */
   virtual ~QgsServerFilter();
   /** Return the QgsServerInterface instance*/
   QgsServerInterface* serverInterface( );
   /** Method called when the QgsRequestHandler is ready and populated with
    * parameters, just before entering the main switch for core services.*/
   virtual void requestReady();
   /** Method called when the QgsRequestHandler processing has done and
    * the response is ready, just after the main switch for core services.
    */
   virtual void responseReady();

};
