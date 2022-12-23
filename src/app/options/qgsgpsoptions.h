/***************************************************************************
    qgsgpsoptions.h
    ---------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPSOPTIONS_H
#define QGSGPSOPTIONS_H

#include "ui_qgsgpsoptionswidgetbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgis_app.h"

/**
 * \ingroup app
 * \class QgsGpsOptionsWidget
 * \brief An options widget showing GPS settings.
 *
 * \since QGIS 3.28
 */
class APP_EXPORT QgsGpsOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsGpsOptionsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGpsOptionsWidget with the specified \a parent widget.
     */
    QgsGpsOptionsWidget( QWidget *parent );

    void apply() override;

  private slots:

    void refreshDevices();
    void timestampFormatChanged( int index );

  private:

    void updateTimeZones();

    bool mBlockStoringChanges = false;
    QIntValidator *mAcquisitionIntValidator = nullptr;
    QIntValidator *mDistanceThresholdValidator = nullptr;

    friend class TestQgsGpsIntegration;
};


class QgsGpsOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsGpsOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;

};


#endif // QGSGPSOPTIONS_H
