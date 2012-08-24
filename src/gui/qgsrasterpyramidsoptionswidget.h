/***************************************************************************
                          qgsrasterpyramidsoptionswidget.h
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

#ifndef QGSRASTERPYRAMIDSOPTIONSWIDGET_H
#define QGSRASTERPYRAMIDSOPTIONSWIDGET_H

#include "ui_qgsrasterpyramidsoptionswidgetbase.h"

class QCheckBox;

/** \ingroup gui
 * A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterPyramidsOptionsWidget: public QWidget,
      private Ui::QgsRasterPyramidsOptionsWidgetBase
{
    Q_OBJECT

  public:

  QgsRasterPyramidsOptionsWidget( QWidget* parent = 0, QString provider = "gdal" );
    ~QgsRasterPyramidsOptionsWidget();

    void setProvider( QString provider );
    QStringList createOptions() const { return mPyramidsOptionsWidget->options(); }
    QgsRasterFormatSaveOptionsWidget* createOptionsWidget() { return mPyramidsOptionsWidget; }

  public slots:

    void apply();

  private slots:

    void on_lePyramidsLevels_editingFinished();
    void on_cboPyramidsFormat_currentIndexChanged( int index )
    { mPyramidsOptionsWidget->setEnabled( index != 2 ); }

  private:

    void updateUi();
    
    QString mProvider;
    bool mInternal;
	QString mResamplingMethod;
    QList<int> mOverviewList;
    QMap< int, QCheckBox* > mOverviewCheckBoxes;
};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
