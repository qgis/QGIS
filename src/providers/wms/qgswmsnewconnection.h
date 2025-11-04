/***************************************************************************
  qgswmsnewconnection.h - QgsWmsNewConnection

 ---------------------
 begin                : 16.10.2025
 copyright            : (C) 2025 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWMSNEWCONNECTION_H
#define QGSWMSNEWCONNECTION_H

#include "qgsnewhttpconnection.h"
#include "qgswmscapabilities.h"

class QgsWmsNewConnection : public QgsNewHttpConnection
{
    Q_OBJECT
  public:
    QgsWmsNewConnection( QWidget *parent = nullptr, const QString &connName = QString() );

  private slots:

    void detectFormat();

  private:
    void initWmsSpecificSettings();
    std::unique_ptr<QgsWmsCapabilities> mCapabilities;
};

#endif // QGSWMSNEWCONNECTION_H
