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

#include "qgssourceselectdialog.h"

class QCheckBox;

class QgsAfsSourceSelect: public QgsSourceSelectDialog
{
    Q_OBJECT

  public:
    QgsAfsSourceSelect( QWidget* parent, Qt::WindowFlags fl, bool embeddedMode = false );

  protected:
    bool connectToService( const QgsOWSConnection& connection ) override;
    void buildQuery( const QgsOWSConnection &connection, const QModelIndex& ) override;
    QString getLayerURI( const QgsOWSConnection &connection,
                         const QString& layerTitle, const QString& layerName,
                         const QString& crs = QString(),
                         const QString& filter = QString(),
                         const QgsRectangle& bBox = QgsRectangle() ) const override;
};

#endif // QGSAFSSOURCESELECT_H
