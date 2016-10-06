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

#include "qgsfeature.h"
#include "ui_qgsgeometrycheckersetuptab.h"

class QgisInterface;
class QgsVectorLayer;
class QgsGeometryChecker;
class QgsFeaturePool;

class QgsGeometryCheckerSetupTab : public QWidget
{
    Q_OBJECT
  public:
    QgsGeometryCheckerSetupTab( QgisInterface * iface, QWidget* parent = nullptr );
    ~QgsGeometryCheckerSetupTab();

  signals:
    void checkerStarted( QgsGeometryChecker* checker, QgsFeaturePool* featurePool );
    void checkerFinished( bool );

  private:
    QgisInterface* mIface;
    Ui::QgsGeometryCheckerSetupTab ui;
    QPushButton* mRunButton;
    QPushButton* mAbortButton;
    QMutex m_errorListMutex;
    QString mOutputDriverName;

    QgsVectorLayer *getSelectedLayer();

  private slots:
    void runChecks();
    void updateLayers();
    void validateInput();
    void selectOutputFile();
    void showCancelFeedback();
};

#endif // QGS_GEOMETRY_CHECKER_SETUP_TAB_H
