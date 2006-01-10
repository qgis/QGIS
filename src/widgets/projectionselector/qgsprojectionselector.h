/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSPROJECTIONSELECTOR_H
#define QGSPROJECTIONSELECTOR_H

#include "ui_qgsprojectionselectorbase.h"


/**
  @author Tim Sutton
  */
class QgsProjectionSelector: public QWidget, private Ui::QgsProjectionSelectorBase
{
  Q_OBJECT
    public:
      QgsProjectionSelector( QWidget* parent , const char* name="", Qt::WFlags fl =0  );
      ~QgsProjectionSelector();
      //! Populate the proj tree view with  user defined projection names...
      void getUserProjList();
      //! Populate the proj tree view with system projection names...
      void getProjList();
      void updateProjAndEllipsoidAcronyms(int theSrsid,QString theProj4String);
      /*!
       * \brief Make the string safe for use in SQL statements.
       *  This involves escaping single quotes, double quotes, backslashes,
       *  and optionally, percentage symbols.  Percentage symbols are used
       *  as wildcards sometimes and so when using the string as part of the
       *  LIKE phrase of a select statement, should be escaped.
       * \arg const QString in The input string to make safe.
       * \return The string made safe for SQL statements.
       */
      const QString stringSQLSafe(const QString theSQL);

    public slots:
      void setSelectedSRSName(QString theSRSName);
      QString getSelectedName();
      void setSelectedSRSID(long theSRSID);
      QString getCurrentProj4String();
      long getCurrentSRID(); //posgis style projection identifier
      long getCurrentSRSID();//qgis projection identfier
      void on_pbnFind_clicked();

    private:

      // List view nodes for the tree view of projections
      //! User defined projections node
      QTreeWidgetItem *mUserProjList;
      //! GEOGCS node
      QTreeWidgetItem *mGeoList;
      //! PROJCS node
      QTreeWidgetItem *mProjList;
      //! Users custom coordinate system file
      QString mCustomCsFile;
      //! File name of the sqlite3 database
      QString mSrsDatabaseFileName;

      /** 
       * Utility method used in conjunction with name based searching tool 
       */
      long getLargestSRSIDMatch(QString theSql);

    private slots:
      /**private handler for when user selects a cs
       *it will cause wktSelected and sridSelected events to be spawned
       */
      void coordinateSystemSelected(QTreeWidgetItem*);

    signals:
      void sridSelected(QString theSRID);
      //! Refresh any listening canvases
      void refresh();
      //! Let listeners know if find has focus so they can adjust the default button
      void searchBoxHasFocus(bool);
};

#endif
