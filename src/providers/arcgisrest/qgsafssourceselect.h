/***************************************************************************
      qgsafssourceselect.h
      --------------------
    begin                : Jun 02, 2015
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

#ifndef QGSAFSSOURCESELECT_H
#define QGSAFSSOURCESELECT_H

#include "qgsguiutils.h"
#include "qgsproviderregistry.h"
#include "qgsarcgisservicesourceselect.h"

class QCheckBox;

class QgsAfsSourceSelect: public QgsArcGisServiceSourceSelect
{
    Q_OBJECT

  public:
    QgsAfsSourceSelect( QWidget *parent, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
  protected:
    bool connectToService( const QgsOwsConnection &connection ) override;
    void buildQuery( const QgsOwsConnection &connection, const QModelIndex & ) override;
    QString getLayerURI( const QgsOwsConnection &connection,
                         const QString &layerTitle, const QString &layerName,
                         const QString &crs = QString(),
                         const QString &filter = QString(),
                         const QgsRectangle &bBox = QgsRectangle(), const QString &layerId = QString() ) const override;
  private:
    //! A layer is added from the dialog
    void addServiceLayer( QString uri, QString typeName ) override;

};

#endif // QGSAFSSOURCESELECT_H
