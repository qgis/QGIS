/***************************************************************************
    qgsstacsourceselectprovider.h
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACSOURCESELECTPROVIDER_H
#define QGSSTACSOURCESELECTPROVIDER_H

#include "qgssourceselectprovider.h"
// #include "qgis_gui.h"

#define SIP_NO_FILE

class QgsStacSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QgsStacSourceSelectProvider();

    // QgsSourceSelectProvider interface
    QString providerKey() const override;
    QString text() const override;
    QString toolTip() const override;
    QIcon icon() const override;
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ) const override;
    int ordering() const override;
};

#endif // QGSSTACSOURCESELECTPROVIDER_H
