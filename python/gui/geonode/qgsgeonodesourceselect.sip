/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/geonode/qgsgeonodesourceselect.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsGeonodeItemDelegate : QItemDelegate
{
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsgeonodesourceselect.h"
%End
  public:
    explicit QgsGeonodeItemDelegate( QObject *parent = 0 );
};

class QgsGeoNodeSourceSelect: QDialog
{

%TypeHeaderCode
#include "qgsgeonodesourceselect.h"
%End
  public:

    QgsGeoNodeSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool embeddedMode = false );
    ~QgsGeoNodeSourceSelect();

  signals:
    void connectionsChanged();
    void addRasterLayer( const QString &rasterLayerPath,
                         const QString &baseName,
                         const QString &providerKey );
    void addRasterLayer();

    void addWfsLayer(
      const QString &uri,
      const QString &layerName,
      const QString &providerKey );

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/geonode/qgsgeonodesourceselect.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
