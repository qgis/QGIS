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
** This work was made possible through a grant by the the John D. and
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

#include <ui_evisdatabaseconnectionguibase.h>
#include "evisdatabaseconnection.h"
#include "evisdatabaselayerfieldselectiongui.h"
#include "evisquerydefinition.h"
#include "qgscontexthelp.h"

#include <QTemporaryFile>
#include <QDialog>

/**
* \class eVisDatabaseConnectionGui
* \brief GUI class for database connections
* This class provides the GUI component for setting up a database connection and making a sql query.
* This class effectively provides access to a wide variety of database types. Upon a sucessful query,
* the results are stored in a tabdelimited file the loaded into qgis using the demlimitedtext data provider
*/
class eVisDatabaseConnectionGui : public QDialog, private Ui::eVisDatabaseConnectionGuiBase
{

    Q_OBJECT

  public:
    /** \brief Constructor */
    eVisDatabaseConnectionGui( QList<QTemporaryFile*>*, QWidget* parent = 0, Qt::WFlags fl = 0 );

    /** \brief Destructor */
    ~eVisDatabaseConnectionGui( );

  private:
    /** \brief Pointer to a database connection */
    eVisDatabaseConnection* mDatabaseConnection;

    /** \brief Pointer to a temporary file which will hold the results of our query */
    QList<QTemporaryFile*>* mTempOutputFileList;

    /** \brief Pointer to another GUI component that will select which fields contain x, y coordinates */
    eVisDatabaseLayerFieldSelectionGui* mDatabaseLayerFieldSelector;

    /** \brief Pointer to a QMap which will hold the definition of preexisting query that can be loaded from an xml file */
    QMap<int, eVisQueryDefinition>* mQueryDefinitionMap;

  private slots:
    /** \brief Slot called after the user selects the x, y fields in the field selection gui component */
    void drawNewVectorLayer( QString, QString, QString );

    void on_buttonBox_accepted( );
    void on_buttonBox_helpRequested( ) { QgsContextHelp::run( metaObject()->className() ); };

    void on_cboxDatabaseType_currentIndexChanged( int );
    void on_cboxPredefinedQueryList_currentIndexChanged( int );
    void on_pbtnConnect_clicked( );
    void on_pbtnLoadPredefinedQueries_clicked( );
    void on_pbtnOpenFile_clicked( );
    void on_pbtnRunQuery_clicked( );

  signals:
    /** \brief signal emited by the drawNewVectorLayer slot */
    void drawVectorLayer( QString, QString, QString );
};

#endif
