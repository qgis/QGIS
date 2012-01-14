/***************************************************************************
                        qgsvectoroverlayplugin.h  -  description
                        ------------------------
   begin                : January 2007
   copyright            : (C) 2007 by Marco Hugentobler
   email                : marco dot hugentobler at karto dot baug dot ethz dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTOROVERLAYPLUGIN_H
#define QGSVECTOROVERLAYPLUGIN_H

#include "qgisplugin.h"

class QgsApplyDialog;
class QgsVectorLayer;
class QWidget;

/**Interface class for vector overlay plugins*/
class QgsVectorOverlayPlugin: public QgisPlugin
{
  public:
    QgsVectorOverlayPlugin( const QString& name, const QString& description, const QString& category, const QString& version ): QgisPlugin( name, description, category, version, QgisPlugin::VECTOR_OVERLAY ) {}

    virtual ~QgsVectorOverlayPlugin() {}

    /**Returns a dialog which can be embedded into the vector layer properties*/
    virtual QgsApplyDialog* dialog( QgsVectorLayer* vl ) const = 0;
};

#endif
