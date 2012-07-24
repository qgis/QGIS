/***************************************************************************
                          qgsrasterformatsaveoptionswidget.h
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERFORMATSAVEOPTIONSWIDGET_H
#define QGSRASTERFORMATSAVEOPTIONSWIDGET_H

#include "ui_qgsrasterformatsaveoptionswidgetbase.h"

/** \ingroup gui
 * A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterFormatSaveOptionsWidget: public QWidget, private Ui::QgsRasterFormatSaveOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsRasterFormatSaveOptionsWidget( QWidget* parent = 0, QString format = "GTiff", QString provider = "gdal" );
    ~QgsRasterFormatSaveOptionsWidget();

    void setFormat( QString format );
    void setProvider( QString provider );
    QStringList options() const;
    void showProfileButtons( bool show = true );

  public slots:

    void apply();
    void helpOptions();
    bool validateOptions( bool gui = true );

  private slots:

    void on_mProfileNewButton_clicked();
    void on_mProfileDeleteButton_clicked();
    void on_mProfileResetButton_clicked();
    void on_mOptionsAddButton_clicked();
    void on_mOptionsDeleteButton_clicked();
    void on_mOptionsLineEdit_editingFinished();
    void optionsTableChanged();
    void optionsTableEnableDeleteButton();
    void updateOptions();

  private:

    QString mFormat;
    QString mProvider;
    QMap< QString, QString> mOptionsMap;
    static QMap< QString, QStringList > mBuiltinProfiles;

    QString settingsKey( QString profile ) const;
    QString currentProfileKey() const;
    QString createOptions( QString profile ) const;
    void deleteCreateOptions( QString profile );
    void setCreateOptions( );
    void setCreateOptions( QString profile, QString options );
    void setCreateOptions( QString profile, QStringList list );
    QStringList profiles() const;
    void updateProfiles();
    bool eventFilter( QObject *obj, QEvent *event );

};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
