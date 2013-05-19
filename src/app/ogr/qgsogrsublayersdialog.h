/***************************************************************************
    qgsogrsublayersdialog.h  - dialog for selecting ogr sublayers
    ---------------------
    begin                : January 2009
    copyright            : (C) 2009 by Florian El Ahdab
    email                : felahdab at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRSUBLAYERSDIALOG_H
#define QGSOGRSUBLAYERSDIALOG_H

#include <QDialog>
#include <ui_qgsogrsublayersdialogbase.h>
#include "qgscontexthelp.h"

class QgsOGRSublayersDialog : public QDialog, private Ui::QgsOGRSublayersDialogBase
{
    Q_OBJECT
  public:
    QgsOGRSublayersDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsOGRSublayersDialog();
    void populateLayerTable( QStringList theList, QString delim = ":" );
    QStringList getSelection();
    QList<int> getSelectionIndexes();

  public slots:
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
};

#endif
