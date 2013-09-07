/***************************************************************************
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
    QStringList mIds, mNames, mDescriptions;
    QString qmlStyle;
    Q_OBJECT
  public:
    explicit QgsLoadStyleFromDBDialog( QWidget *parent = 0 );

    void initializeLists( QStringList ids, QStringList names, QStringList descriptions, int sectionLimit );
    QString getSelectedStyleId();

  public slots:
    void cellSelectedRelatedTable( int r );
    void cellSelectedOthersTable( int r );

  private:

};

#endif //QGSLOADFILEFROMDBDIALOG_H
