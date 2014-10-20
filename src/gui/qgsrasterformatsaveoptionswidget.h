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

#include "qgsrasterdataprovider.h"

class QgsRasterLayer;

/** \ingroup gui
 * A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterFormatSaveOptionsWidget: public QWidget,
      private Ui::QgsRasterFormatSaveOptionsWidgetBase
{
    Q_OBJECT

  public:

    enum Type
    {
      Default, // everything except profile buttons (save as dlg)
      Full, // everything (options dlg)
      Table, // just table
      LineEdit, // just the line edit
      ProfileLineEdit // Profile + LineEdit
    };

    QgsRasterFormatSaveOptionsWidget( QWidget* parent = 0, QString format = "GTiff",
                                      QgsRasterFormatSaveOptionsWidget::Type type = Default,
                                      QString provider = "gdal" );
    ~QgsRasterFormatSaveOptionsWidget();

    void setFormat( QString format );
    void setProvider( QString provider );
    void setRasterLayer( QgsRasterLayer* rasterLayer ) { mRasterLayer = rasterLayer; mRasterFileName = QString(); }
    void setRasterFileName( const QString& file ) { mRasterLayer = 0; mRasterFileName = file; }
    QStringList options() const;
    void setType( QgsRasterFormatSaveOptionsWidget::Type type = Default );
    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat format )
    { mPyramids = true; mPyramidsFormat = format; }

  public slots:

    void apply();
    void helpOptions();
    QString validateOptions( bool gui = true, bool reportOk = true );
    void updateProfiles();

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
    void swapOptionsUI( int newIndex = -1 );
    void updateControls();

  protected:
    virtual void showEvent( QShowEvent * event );

  signals:
    void optionsChanged();

  private:

    QString mFormat;
    QString mProvider;
    QgsRasterLayer* mRasterLayer;
    QString mRasterFileName;
    QMap< QString, QString> mOptionsMap;
    static QMap< QString, QStringList > mBuiltinProfiles;
    bool mPyramids;
    QgsRaster::RasterPyramidsFormat mPyramidsFormat;

    QString settingsKey( QString profile ) const;
    QString currentProfileKey() const;
    QString createOptions( QString profile ) const;
    void deleteCreateOptions( QString profile );
    void setCreateOptions();
    void setCreateOptions( QString profile, QString options );
    void setCreateOptions( QString profile, QStringList list );
    QStringList profiles() const;
    bool eventFilter( QObject *obj, QEvent *event );

};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
