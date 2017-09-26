/***************************************************************************
      qgsamssourceselect.h
      --------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAMSSOURCESELECT_H
#define QGSAMSSOURCESELECT_H

#include "qgsarcgisservicesourceselect.h"
#include "qgsproviderregistry.h"

class QCheckBox;

class QgsAmsSourceSelect: public QgsArcGisServiceSourceSelect
{
    Q_OBJECT

  public:
    QgsAmsSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  protected:
    bool connectToService( const QgsOwsConnection &connection ) override;
    QString getLayerURI( const QgsOwsConnection &connection,
                         const QString &layerTitle, const QString &layerName,
                         const QString &crs = QString(),
                         const QString &filter = QString(),
                         const QgsRectangle &bBox = QgsRectangle() ) const override;
  private:
    //! A layer is added from the dialog
    virtual void addServiceLayer( QString uri, QString typeName ) override;
};

#endif // QGSAMSSOURCESELECT_H
