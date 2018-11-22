/***************************************************************************
 *  qgsgeometrycheckersetuptab.h                                           *
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

#ifndef QGS_GEOMETRY_CHECKER_SETUP_TAB_H
#define QGS_GEOMETRY_CHECKER_SETUP_TAB_H

#include <QMutex>

#include "ui_qgsgeometrycheckersetuptab.h"

class QgisInterface;
class QgsVectorLayer;
class QgsGeometryChecker;
class QgsFeaturePool;

class QgsGeometryCheckerSetupTab : public QWidget
{
    Q_OBJECT
  public:
    QgsGeometryCheckerSetupTab( QgisInterface *iface, QDialog *checkerDialog, QWidget *parent = nullptr );
    ~QgsGeometryCheckerSetupTab() override;

    /**
     * Indicates whether the geometry checker is currently running its checks in the background.
     * Useful to figure out whether it is safe to close the dialog and thus destroy the checker.
     */
    bool isRunningInBackground() const { return mIsRunningInBackground; }

  signals:
    void checkerStarted( QgsGeometryChecker *checker );
    void checkerFinished( bool );

  private:
    QgisInterface *mIface = nullptr;
    QDialog *mCheckerDialog;
    Ui::QgsGeometryCheckerSetupTab ui;
    QPushButton *mRunButton = nullptr;
    QPushButton *mAbortButton = nullptr;
    QMutex m_errorListMutex;
    bool mIsRunningInBackground = false;

    QList<QgsVectorLayer *> getSelectedLayers();

  private slots:
    void runChecks();
    void updateLayers();
    void validateInput();
    void selectOutputDirectory();
    void showCancelFeedback();
};

#endif // QGS_GEOMETRY_CHECKER_SETUP_TAB_H
