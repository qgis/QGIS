/***************************************************************************
 *   qgsprojectionselector.h
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

#include <QSet>

class QResizeEvent;

/**
  @author Tim Sutton
  */
class GUI_EXPORT QgsProjectionSelector: public QWidget, private Ui::QgsProjectionSelectorBase
{
  Q_OBJECT
    public:
      QgsProjectionSelector(QWidget* parent,
                            const char * name = "",
                            Qt::WFlags fl = 0);

      ~QgsProjectionSelector();

      /**
       * \brief Populate the proj tree view with user defined projection names...
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \todo Should this be public?
       */
      void applyUserProjList(QSet<QString> * crsFilter = 0);

      /**
       * \brief Populate the proj tree view with system projection names...
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \todo Should this be public?
       */
      void applyProjList(QSet<QString> * crsFilter = 0);


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

      //! Gets the current EPSG-style projection identifier
      long getCurrentEpsg();

    public slots:
      void setSelectedSRSName(QString theSRSName);

      QString getSelectedName();

      void setSelectedSRSID(long theSRSID);

      //void setSelectedEPSG(long epsg);

      QString getCurrentProj4String();

      //! Gets the current PostGIS-style projection identifier
      long getCurrentSRID();

      //! Gets the current QGIS projection identfier
      long getCurrentSRSID();

      /**
       * \brief filters this widget by the given CRSs
       *
       * Sets this widget to filter the available projections to those listed
       * by the given Coordinate Reference Systems.
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \note This function only deals with EPSG labels only at this time.
       *
       * \warning This function's behaviour is undefined if it is called after the widget is shown.
       */
      void setOgcWmsCrsFilter(QSet<QString> crsFilter);

      void on_pbnFind_clicked();

    protected:
      /** Used to ensure the projection list view is actually populated */
      void showEvent ( QShowEvent * theEvent );

      /** Used to manage column sizes */
      void resizeEvent ( QResizeEvent * theEvent );

  private:
      /**
       * \brief converts the CRS group to a SQL expression fragment
       *
       * Converts the given Coordinate Reference Systems to a format suitable
       * for use in SQL for querying against the QGIS SRS database.
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \note This function only deals with EPSG labels only at this time.
       */
      QString ogcWmsCrsFilterAsSqlExpression(QSet<QString> * crsFilter);

      /**
       * \brief does the legwork of applying the SRS Name Selection
       *
       * \warning This function does nothing unless getUserList() and getUserProjList()
       *          Have already been called
       *
       * \warning This function only expands the parents of the selection and
       *          does not scroll the list to the selection if the widget is not visible.
       *          Therefore you will typically want to use this in a showEvent().
       */
      void applySRSNameSelection();

      /**
       * \brief does the legwork of applying the SRS ID Selection
       *
       * \warning This function does nothing unless getUserList() and getUserProjList()
       *          Have already been called
       *
       * \warning This function only expands the parents of the selection and
       *          does not scroll the list to the selection if the widget is not visible.
       *          Therefore you will typically want to use this in a showEvent().
       */
      void applySRSIDSelection();

      /**
       * \brief gets an arbitrary sqlite3 attribute of type "long" from the selection
       *
       * \param attributeName   The sqlite3 column name, typically "srid" or "epsg"
       */
      long getCurrentLongAttribute(QString attributeName);

      /** Show the user a warning if the srs database could not be found */
      const void showDBMissingWarning(const QString theFileName);
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

      //! Has the Projection List been populated?
      bool mProjListDone;

      //! Has the User Projection List been populated?
      bool mUserProjListDone;

      //! Is there a pending selection to be made by SRS Name?
      bool mSRSNameSelectionPending;

      //! Is there a pending selection to be made by SRS ID?
      bool mSRSIDSelectionPending;

      //! The SRS Name that wants to be selected on this widget
      QString mSRSNameSelection;

      //! The SRS ID that wants to be selected on this widget
      long mSRSIDSelection;

      //! The set of OGC WMS CRSs that want to be applied to this widget
      QSet<QString> mCrsFilter;

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
