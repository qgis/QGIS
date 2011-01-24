/***************************************************************************
  roadgraphplugin.h
  --------------------------------------
  Date                 : 2010-10-10
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPH_SETTINGSDLG_H
#define ROADGRAPH_SETTINGSDLG_H

#include <QDialog>

// forward declaration QT-classes
class QComboBox;

// forward declaration Qgis-classes

//forward declaration RoadGraph plugins classes
class RgGraphDirector;

/**
@author Sergey Yakushev
*/
/**
* \class RgSettingsDlg
* \brief implement of settings dialog
*/
class RgSettingsDlg : public QDialog
{
    Q_OBJECT
  public:
    RgSettingsDlg( RgGraphDirector *director, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~RgSettingsDlg();

    QString timeUnitName();

    void setTimeUnitName( const QString& );

    QString distanceUnitName();

    void setDistanceUnitName( const QString& );

  private:
    static const int context_id = 0;

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();

  private:
    // futurame by used
    QComboBox *mcbGraphDirector;

    /**
     * current graph director
     */
    RgGraphDirector *mDirector;

    QWidget *mSettingsWidget;

    /**
     * plugin distance unit
     */
    QComboBox *mcbPluginsDistanceUnit;

    /**
     * plugin time unit
     */
    QComboBox *mcbPluginsTimeUnit;
};
#endif
