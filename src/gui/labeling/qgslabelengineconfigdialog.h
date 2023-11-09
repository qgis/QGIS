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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>

#include "ui_qgslabelengineconfigdialog.h"
#include "qgslabelingenginesettings.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Widget for configuring the labeling engine
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLabelEngineConfigWidget : public QgsPanelWidget, private Ui::QgsLabelEngineConfigWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsLabelEngineConfigWidget( QgsMapCanvas *canvas, QWidget *parent = nullptr );

    QMenu *menuButtonMenu() override;
    QString menuButtonTooltip() const override;

  public slots:
    //! Applies the changes
    void apply();
    //! Resets the settings to the defaults
    void setDefaults();
    //! Shows the help
    void showHelp();

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QMenu *mWidgetMenu = nullptr;

    Qgis::LabelPlacementEngineVersion mPreviousEngineVersion = Qgis::LabelPlacementEngineVersion::Version2;
};

/**
 * \ingroup gui
 * \brief Dialog for configuring the labeling engine
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLabelEngineConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    //! constructor
    QgsLabelEngineConfigDialog( QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void accept() override;
  private:
    QgsLabelEngineConfigWidget *mWidget = nullptr;

};

#endif // QGSLABELENGINECONFIGDIALOG_H
