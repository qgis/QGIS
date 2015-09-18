/***************************************************************************
    qgsdashspacedialog.h
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDASHSPACEDIALOG_H
#define QGSDASHSPACEDIALOG_H

#include "ui_qgsdashspacedialogbase.h"

/** A dialog to enter a custom dash space pattern for lines*/
class GUI_EXPORT QgsDashSpaceDialog: public QDialog, private Ui::QgsDashSpaceDialogBase
{
    Q_OBJECT
  public:
    QgsDashSpaceDialog( const QVector<qreal>& v, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDashSpaceDialog();

    QVector<qreal> dashDotVector() const;

  private slots:
    void on_mAddButton_clicked();
    void on_mRemoveButton_clicked();
};

#endif // QGSDASHSPACEDIALOG_H
