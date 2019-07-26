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
#ifndef QGSDELIMITEDTEXTSOURCESELECT_H
#define QGSDELIMITEDTEXTSOURCESELECT_H

#include "ui_qgsdelimitedtextsourceselectbase.h"

#include <QTextStream>
#include "qgshelp.h"
#include "qgsguiutils.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsdelimitedtextfile.h"

class QButtonGroup;
class QgisInterface;

/**
 * \class QgsDelimitedTextSourceSelect
 */
class QgsDelimitedTextSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsDelimitedTextSourceSelectBase
{
    Q_OBJECT

  public:
    QgsDelimitedTextSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  private:
    bool loadDelimitedFileDefinition();
    void updateFieldLists();
    QString selectedChars();
    void setSelectedChars( const QString &delimiters );
    void loadSettings( const QString &subkey = QString(), bool loadGeomSettings = true );
    void saveSettings( const QString &subkey = QString(), bool saveGeomSettings = true );
    void loadSettingsForFile( const QString &filename );
    void saveSettingsForFile( const QString &filename );
    bool trySetXYField( QStringList &fields, QList<bool> &isValidNumber, const QString &xname, const QString &yname );

  private:
    std::unique_ptr<QgsDelimitedTextFile> mFile;
    int mExampleRowCount = 20;
    int mBadRowCount = 0;
    QString mSettingsKey;
    QString mLastFileType;
    QButtonGroup *bgFileFormat = nullptr;
    QButtonGroup *bgGeomType = nullptr;
    void showHelp();

  public slots:
    void addButtonClicked() override;
    void updateFileName();
    void updateFieldsAndEnable();
    void enableAccept();
    bool validate();
};

#endif // QGSDELIMITEDTEXTSOURCESELECT_H
