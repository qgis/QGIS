/***************************************************************************
    qgsloadstylefromdbdialog.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Emilio Loi
    email                : loi at faunalia dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOADFILEFROMDBDIALOG_H
#define QGSLOADFILEFROMDBDIALOG_H

#include "ui_qgsloadstylefromdbdialog.h"
#include "qgisgui.h"
#include "qgsfield.h"

class APP_EXPORT QgsLoadStyleFromDBDialog : public QDialog, private Ui::QgsLoadStyleFromDBDialogLayout
{
    QString mSelectedStyleId;
    int mSectionLimit;
    QString qmlStyle;
    Q_OBJECT
  public:
    explicit QgsLoadStyleFromDBDialog( QWidget *parent = nullptr );

    ~QgsLoadStyleFromDBDialog();

    void initializeLists( const QStringList& ids, const QStringList& names, const QStringList& descriptions, int sectionLimit );
    QString getSelectedStyleId();

  public slots:
    void cellSelectedRelatedTable( int r );
    void cellSelectedOthersTable( int r );

  private:

};

#endif //QGSLOADFILEFROMDBDIALOG_H
