/***************************************************************************
    qgsglobeinterface.h
     --------------------------------------
    Date                 : 22.8.2013
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

#ifndef QGSGLOBEINTERFACE_H
#define QGSGLOBEINTERFACE_H

#include "qgsplugininterface.h"


class GlobePlugin;

class QgsGlobeInterface : public QgsPluginInterface
{
    Q_OBJECT

  public:
    QgsGlobeInterface( GlobePlugin* const globe );
    
    void syncExtent();

  private:
    GlobePlugin* mGlobe;
};

#endif // QGSGLOBEINTERFACE_H
