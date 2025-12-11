/***************************************************************************
  qgslayermetadatasourceselectprovider.h - QgsLayerMetadataSourceSelectProvider

 ---------------------
 begin                : 6.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATASOURCESELECTPROVIDER_H
#define QGSLAYERMETADATASOURCESELECTPROVIDER_H

#include "qgis_gui.h"
#include "qgssourceselectprovider.h"

#define SIP_NO_FILE


/**
 * \ingroup gui
 * \brief Source select provider for layer metadata.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsLayerMetadataSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QgsLayerMetadataSourceSelectProvider();

    // QgsSourceSelectProvider interface
  public:
    [[nodiscard]] QString providerKey() const override;
    [[nodiscard]] QString text() const override;
    [[nodiscard]] QString toolTip() const override;
    [[nodiscard]] QIcon icon() const override;
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ) const override;
    [[nodiscard]] int ordering() const override;
};

#endif // QGSLAYERMETADATASOURCESELECTPROVIDER_H
