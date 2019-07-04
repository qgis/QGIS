/*
** File: evisdatabaseconnectiongui.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-07
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#ifndef eVisDatabaseConnectionGUI_H
#define eVisDatabaseConnectionGUI_H

#include "ui_evisdatabaseconnectionguibase.h"
#include "evisdatabaseconnection.h"
#include "evisdatabaselayerfieldselectiongui.h"
#include "evisquerydefinition.h"
#include "qgshelp.h"

#include <QTemporaryFile>
#include <QDialog>

/**
* \class eVisDatabaseConnectionGui
* \brief GUI class for database connections
* This class provides the GUI component for setting up a database connection and making a sql query.
* This class effectively provides access to a wide variety of database types. Upon a successful query,
* the results are stored in a tabdelimited file the loaded into qgis using the demlimitedtext data provider
*/
class eVisDatabaseConnectionGui : public QDialog, private Ui::eVisDatabaseConnectionGuiBase
{

    Q_OBJECT

  public:
    //! \brief Constructor
    eVisDatabaseConnectionGui( QList<QTemporaryFile *> *, QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr );

  private:
    //! \brief Pointer to a database connection
    eVisDatabaseConnection *mDatabaseConnection = nullptr;

    //! \brief Pointer to a temporary file which will hold the results of our query
    QList<QTemporaryFile *> *mTempOutputFileList;

    //! \brief Pointer to another GUI component that will select which fields contain x, y coordinates
    eVisDatabaseLayerFieldSelectionGui *mDatabaseLayerFieldSelector = nullptr;

    //! \brief Pointer to a QMap which will hold the definition of preexisting query that can be loaded from an xml file
    QMap<int, eVisQueryDefinition> *mQueryDefinitionMap;

  private slots:
    //! \brief Slot called after the user selects the x, y fields in the field selection gui component
    void drawNewVectorLayer( const QString &, const QString &, const QString & );

    void buttonBox_accepted();
    void showHelp();

    void cboxDatabaseType_currentIndexChanged( int );
    void cboxPredefinedQueryList_currentIndexChanged( int );
    void pbtnConnect_clicked();
    void pbtnLoadPredefinedQueries_clicked();
    void pbtnOpenFile_clicked();
    void pbtnRunQuery_clicked();

  signals:
    //! \brief signal emitted by the drawNewVectorLayer slot
    void drawVectorLayer( const QString &, const QString &, const QString & );
};

#endif
