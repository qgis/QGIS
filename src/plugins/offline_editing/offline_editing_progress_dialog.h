/***************************************************************************
    progress_dialog.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_OFFLINE_EDITING_PROGRESS_DIALOG_H
#define QGS_OFFLINE_EDITING_PROGRESS_DIALOG_H

#include <QDialog>
#include "ui_offline_editing_progress_dialog_base.h"

class QgsOfflineEditingProgressDialog : public QDialog, private Ui::QgsOfflineEditingProgressDialogBase
{
    Q_OBJECT

  public:
    QgsOfflineEditingProgressDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr );
    virtual ~QgsOfflineEditingProgressDialog();

    void setTitle( const QString& title );
    void setCurrentLayer( int layer, int numLayers );
    void setupProgressBar( const QString& format, int maximum );
    void setProgressValue( int value );

  private:
    int mProgressUpdate;
};

#endif // QGS_OFFLINE_EDITING_PROGRESS_DIALOG_H

