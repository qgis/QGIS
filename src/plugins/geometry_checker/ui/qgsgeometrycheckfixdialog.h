/***************************************************************************
 *  qgsgeometrycheckerfixdialog.h                                          *
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

#ifndef QGS_GEOMETRY_CHECKER_FIX_DIALOG_H
#define QGS_GEOMETRY_CHECKER_FIX_DIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QLabel;
class QGroupBox;
class QButtonGroup;
class QProgressBar;
class QgisInterface;
class QgsGeometryChecker;
class QgsGeometryCheckError;

class QgsGeometryCheckerFixDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsGeometryCheckerFixDialog( QgsGeometryChecker* checker, const QList<QgsGeometryCheckError *>& errors, QgisInterface *iface, QWidget* parent = nullptr );

  signals:
    void currentErrorChanged( QgsGeometryCheckError* error );

  private:
    QgsGeometryChecker* mChecker;
    QList<QgsGeometryCheckError *> mErrors;
    QgisInterface* mIface;
    QGroupBox* mResolutionsBox;
    QDialogButtonBox* mButtonBox;
    QLabel* mStatusLabel;
    QProgressBar* mProgressBar;
    QButtonGroup* mRadioGroup;
    QPushButton* mNextBtn;
    QPushButton* mFixBtn;
    QPushButton* mSkipBtn;

    void showEvent( QShowEvent * ) override;

  private slots:
    void setupNextError();
    void fixError();
    void skipError();
};

#endif // QGS_GEOMETRY_CHECKER_FIX_DIALOG_H
