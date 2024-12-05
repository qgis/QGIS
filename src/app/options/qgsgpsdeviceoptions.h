/***************************************************************************
    qgsgpsdeviceoptions.h
    -------------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPSDEVICEOPTIONS_H
#define QGSGPSDEVICEOPTIONS_H

#include "ui_qgsgpsdevicedialogbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgscodeeditor.h"

class QgsBabelGpsDeviceFormat;

/**
 * \ingroup app
 * \class QgsGpsDeviceOptionsWidget
 * \brief An options widget showing GPS device configuration.
 *
 * \since QGIS 3.22
 */
class QgsGpsDeviceOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsGpsDeviceWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsGpsDeviceOptionsWidget with the specified \a parent widget.
     */
    QgsGpsDeviceOptionsWidget( QWidget *parent );
    QString helpKey() const override;
    void apply() override;

  private slots:
    void addNewDevice();
    void removeCurrentDevice();
    void updateDeviceList( const QString &selection = QString() );
    void selectedDeviceChanged( QListWidgetItem *current );
    void updateCurrentDevice();
    void renameCurrentDevice();

  private:
    QMap<QString, QStringList> mDevices;
    bool mBlockStoringChanges = false;
};


class QgsGpsDeviceOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    QgsGpsDeviceOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;
};


#endif // QGSGPSDEVICEOPTIONS_H
