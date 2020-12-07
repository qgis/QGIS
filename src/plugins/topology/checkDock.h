/***************************************************************************
  checkDock.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHECKDOCK_H
#define CHECKDOCK_H

#include "qgsdockwidget.h"

#include "qgsgeometry.h"
//#include "qgsvertexmarker.h"
#include "qgsspatialindex.h"

#include "ui_checkDock.h"
#include "rulesDialog.h"
#include "topolError.h"
#include "topolTest.h"
#include "dockModel.h"
#include "qobjectuniqueptr.h"

class QgsRubberBand;
class QgsVertexMarker;
class QgisApp;
class QgisInterface;
class checkDock;

class checkDock : public QgsDockWidget, private Ui::checkDock
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param qIface  pointer to QgisInterface instance that is passed to the rulesDialog
     * \param parent parent object
     */
    checkDock( QgisInterface *qIface, QWidget *parent = nullptr );
    ~checkDock() override;

  private slots:

    /**
     * Launches the configuration dialog
     */
    void configure();

    /**
     * Launches fixing routine
     */
    void fix();

    /**
     * Validates the whole layer
     */
    void validateAll();

    /**
     * Validates the current extent
     */
    void validateExtent();

    /**
     * Validates only selected features
     */
    void validateSelected();

    /**
     * toggles the visibility of rubber band error markers
     */
    void toggleErrorMarker();

    /**
     * Handles error selection
     * \param index clicked index in the table
     */
    void errorListClicked( const QModelIndex &index );

    /**
     * Deletes allocated errors' data
     */
    void deleteErrors();

    /**
     * Filters all errors involving features from specified layer
     * \param layerId layer ID
     */
    void parseErrorListByLayer( const QString &layerId );

    /**
     * Clears rubberbands when window is hidden
     * \param visible true if the window is visible
     */
    void updateRubberBands( bool visible );


  private:
    rulesDialog *mConfigureDialog = nullptr;

    QObjectUniquePtr<QgsRubberBand> mRBConflict;
    QObjectUniquePtr<QgsRubberBand> mRBFeature1;
    QObjectUniquePtr<QgsRubberBand> mRBFeature2;

    QgsVertexMarker *mVMConflict = nullptr;
    QgsVertexMarker *mVMFeature1 = nullptr;
    QgsVertexMarker *mVMFeature2 = nullptr;
    QList<QgsRubberBand *> mRbErrorMarkers;

    ErrorList mErrorList;
    DockModel *mErrorListModel = nullptr;

    QgisInterface *qgsInterface = nullptr;

    //pointer to topology tests table
    QTableWidget *mTestTable = nullptr;

    topolTest *mTest = nullptr;

    /**
     * Runs tests from the test table
     * \param type validation type - what features to check
     */
    void runTests( ValidateType type );

    /**
     * Validates topology
     * \param type validation type - what features to check
     */
    void validate( ValidateType type );

    /**
     * Filters all errors involving specified feature
     * \param featureId feature ID
     */
    void parseErrorListByFeature( int featureId );

    /**
     * Deletes vertex markers
     */
    void clearVertexMarkers();
};

#endif
