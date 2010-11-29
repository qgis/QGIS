/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include "ui_qgsdelimitedtextpluginguibase.h"
#include <QTextStream>
#include "qgscontexthelp.h"

class QgisInterface;

/**
 * \class QgsDelimitedTextPluginGui
 */
class QgsDelimitedTextPluginGui : public QDialog, private Ui::QgsDelimitedTextPluginGuiBase
{
    Q_OBJECT

  public:
    QgsDelimitedTextPluginGui( QgisInterface * _qI, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsDelimitedTextPluginGui();

    QString readLine( QTextStream & stream );
    QStringList splitLine( QString line );

  private:
    bool haveValidFileAndDelimiters();
    void updateFieldLists();
    void getOpenFileName();

    QgisInterface * qI;
    QAbstractButton *pbnOK;

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_btnBrowseForFile_clicked();

  public slots:
    void updateFieldsAndEnable();
    void enableAccept();

  signals:
    void drawRasterLayer( QString );
    void drawVectorLayer( QString, QString, QString );
};

#endif
