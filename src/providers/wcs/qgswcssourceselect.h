/***************************************************************************
    qgswcssourceselect.h  -  selector for WCS layers
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    generalized          : (C) 2012 Radim Blazek, based on qgsowsconnection.h

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWCSSOURCESELECT_H
#define QGSWCSSOURCESELECT_H

#include "qgsowssourceselect.h"
#include "qgsdatasourceuri.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"
#include "qgswcscapabilities.h"

#include "qgsdataprovider.h"

#include <QStringList>
#include <QPushButton>

class QgisApp;
class QgsDataProvider;
class QButtonGroup;
class QgsNumericSortTreeWidgetItem;
class QDomDocument;
class QDomElement;

/*!
 * \brief  Dialog to create connections and add layers from WMS, WFS, WCS etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WMS server to the map canvas.
 */
class QgsWCSSourceSelect : public QgsOWSSourceSelect
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWCSSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsWCSSourceSelect();

  public slots:

  signals:
    void addRasterLayer( QString const & rasterLayerPath,
                         QString const & baseName,
                         QString const & providerKey );

  private:
    QgsWcsCapabilities mCapabilities;

    QString selectedIdentifier();

    // QgsWcsCapabilities virtual methods
    void populateLayerList() override;
    void addClicked() override;
    void on_mLayersTreeWidget_itemSelectionChanged() override;
    void enableLayersForCrs( QTreeWidgetItem *item ) override;
    void updateButtons() override;
    QList<QgsOWSSourceSelect::SupportedFormat> providerFormats() override;
    QStringList selectedLayersFormats() override;
    QStringList selectedLayersCRSs() override;
    QStringList selectedLayersTimes() override;
};
#endif // QGSWCSSOURCESELECT_H


