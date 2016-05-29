/***************************************************************************
    qgsplugininterface.sip
     --------------------------------------
    Date                 : 21.8.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @brief Trivial base class for plugin interfaces
 */
class QgsPluginInterface : QObject
{
%TypeHeaderCode
#include "qgsplugininterface.h"
%End

  protected:
    /** Should only be instantiated from subclasses */
    QgsPluginInterface( QObject* parent = 0 );
};
