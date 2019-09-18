/***************************************************************************
                              qgswmsdimensiondialog.h
                              ------------------
  begin                : August 20, 2019
  copyright            : (C) 2019 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSDIMENSIONDIALOG_H
#define QGSWMSDIMENSIONDIALOG_H

#include "ui_qgswmsdimensiondialogbase.h"
#include "qgsvectorlayerserverproperties.h"
#include "qgis_app.h"

class QgsVectorLayer;

class APP_EXPORT QgsWmsDimensionDialog: public QDialog, private Ui::QgsWmsDimensionDialogBase
{
    Q_OBJECT
  public:
    QgsWmsDimensionDialog( QgsVectorLayer *layer, QStringList alreadyDefinedDimensions, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );

    QgsVectorLayerServerProperties::WmsDimensionInfo info() const;

    void setInfo( const QgsVectorLayerServerProperties::WmsDimensionInfo &info );

  private slots:
    void nameChanged( const QString &name );
    void fieldChanged();
    void defaultDisplayChanged( int index );

  private:
    //! Target layer
    QgsVectorLayer *mLayer = nullptr;
};


#endif // QGSWMSDIMENSIONDIALOG_H
