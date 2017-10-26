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
    QgsGeometryCheckerFixDialog( QgsGeometryChecker *checker, const QList<QgsGeometryCheckError *> &errors, QWidget *parent = nullptr );

  signals:
    void currentErrorChanged( QgsGeometryCheckError *error );

  private:
    QgsGeometryChecker *mChecker = nullptr;
    QList<QgsGeometryCheckError *> mErrors;
    QGroupBox *mResolutionsBox = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QLabel *mStatusLabel = nullptr;
    QProgressBar *mProgressBar = nullptr;
    QButtonGroup *mRadioGroup = nullptr;
    QPushButton *mNextBtn = nullptr;
    QPushButton *mFixBtn = nullptr;
    QPushButton *mSkipBtn = nullptr;

    void showEvent( QShowEvent * ) override;

  private slots:
    void setupNextError();
    void fixError();
    void skipError();
};

#endif // QGS_GEOMETRY_CHECKER_FIX_DIALOG_H
