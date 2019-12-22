/***************************************************************************
    qgslabelengineconfigdialog.h
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELENGINECONFIGDIALOG_H
#define QGSLABELENGINECONFIGDIALOG_H

#include <QDialog>

#include "ui_qgslabelengineconfigdialog.h"
#include "qgis_app.h"
#include "qgslabelingenginesettings.h"

class QgsMessageBar;

class APP_EXPORT QgsLabelEngineConfigWidget : public QgsPanelWidget, private Ui::QgsLabelEngineConfigWidgetBase
{
    Q_OBJECT
  public:
    QgsLabelEngineConfigWidget( QWidget *parent = nullptr );

    QMenu *menuButtonMenu() override;
    QString menuButtonTooltip() const override;

  public slots:
    void apply();
    void setDefaults();
    void showHelp();

  private:
    QgsMessageBar *mMessageBar = nullptr;
    QMenu *mWidgetMenu = nullptr;

    QgsLabelingEngineSettings::PlacementEngineVersion mPreviousEngineVersion = QgsLabelingEngineSettings::PlacementEngineVersion2;
};

class APP_EXPORT QgsLabelEngineConfigDialog : public QDialog
{
    Q_OBJECT

  public:

    QgsLabelEngineConfigDialog( QWidget *parent = nullptr );

    void accept() override;
  private:
    QgsLabelEngineConfigWidget *mWidget = nullptr;

};

#endif // QGSLABELENGINECONFIGDIALOG_H
