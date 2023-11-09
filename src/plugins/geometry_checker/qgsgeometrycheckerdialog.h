/***************************************************************************
 *  qgsgeometrycheckerdialog.h                                             *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_CHECKER_DIALOG_H
#define QGS_GEOMETRY_CHECKER_DIALOG_H

#include "qgshelp.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QMap>
#include <QTabWidget>

class QgisInterface;
class QgsFeaturePool;
class QgsGeometryChecker;
class QgsVectorLayer;

class QgsGeometryCheckerDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsGeometryCheckerDialog( QgisInterface *iface, QWidget *parent = nullptr );


  private:
    QgisInterface *mIface = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QTabWidget *mTabWidget = nullptr;

    void done( int r ) override;
    void closeEvent( QCloseEvent *ev ) override;

  private slots:
    void onCheckerStarted( QgsGeometryChecker *checker );
    void onCheckerFinished( bool successful );
    void showHelp();
};

#endif // QGS_GEOMETRY_CHECKER_DIALOG_H
