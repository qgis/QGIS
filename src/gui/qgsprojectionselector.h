/***************************************************************************
 *   qgsprojectionselector.h                                               *
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSCRSSELECTOR_H
#define QGSCRSSELECTOR_H

#include <ui_qgsprojectionselectorbase.h>

#include <QSet>
#include <QStringList>

#include "qgis.h"

class QResizeEvent;

/** \ingroup gui
 * A widget for selecting a Coordinate reference system from a tree.
 * @see QgsGenericProjectionSelector.
  @author Tim Sutton
  */
class GUI_EXPORT QgsProjectionSelector: public QWidget, private Ui::QgsProjectionSelectorBase
{
    Q_OBJECT
  public:
    QgsProjectionSelector( QWidget* parent,
                           const char * name = "",
                           Qt::WFlags fl = 0 );

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
    void loadUserCrsList( QSet<QString> * crsFilter = 0 );

    /**
     * \brief Populate the proj tree view with system projection names...
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     * \todo Should this be public?
     */
    void loadCrsList( QSet<QString> * crsFilter = 0 );


    /*!
     * \brief Make the string safe for use in SQL statements.
     *  This involves escaping single quotes, double quotes, backslashes,
     *  and optionally, percentage symbols.  Percentage symbols are used
     *  as wildcards sometimes and so when using the string as part of the
     *  LIKE phrase of a select statement, should be escaped.
     * \arg const QString in The input string to make safe.
     * \return The string made safe for SQL statements.
     */
    const QString sqlSafeString( const QString theSQL );

    //! Gets the current EpsgCrsId-style projection identifier
    // @deprecated there are other authorities - use selectedAuthId()
    Q_DECL_DEPRECATED long selectedEpsg();

    //! Gets the current InternalCrsId-style projection identifier
    long selectedSrsid();

    //! Gets the current authority-style projection identifier
    QString selectedAuthId();

  public slots:
    void setSelectedCrsName( QString theCRSName );

    QString selectedName();

    void setSelectedCrsId( long theCRSID );

    void setSelectedAuthId( QString authId );

    //! Get the selected coordinate system
    // @deprecated there are other authorities - so not always defined
    Q_DECL_DEPRECATED void setSelectedEpsg( long epsg );

    QString selectedProj4String();

    //! Gets the current PostGIS-style projection identifier
    long selectedPostgresSrId();

    //! Gets the current QGIS projection identfier
    long selectedCrsId();

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
     * \warning This function's behaviour is undefined if it is called after the widget is shown.
     */
    void setOgcWmsCrsFilter( QSet<QString> crsFilter );

    void on_pbnFind_clicked();
    void on_lstRecent_currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * );
    void on_cbxHideDeprecated_stateChanged();
    void on_leSearch_textChanged(const QString &);

  protected:
    /** Used to ensure the projection list view is actually populated */
    void showEvent( QShowEvent * theEvent );

    /** Used to manage column sizes */
    void resizeEvent( QResizeEvent * theEvent );

  private:
    /**
     * \brief converts the CRS group to a SQL expression fragment
     *
     * Converts the given Coordinate Reference Systems to a format suitable
     * for use in SQL for querying against the QGIS CRS database.
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     */
    QString ogcWmsCrsFilterAsSqlExpression( QSet<QString> * crsFilter );

    /**
     * \brief does the legwork of applying CRS Selection
     *
     * \warning This function does nothing unless getUserList() and getUserProjList()
     *          Have already been called
     *
     * \warning This function only expands the parents of the selection and
     *          does not scroll the list to the selection if the widget is not visible.
     *          Therefore you will typically want to use this in a showEvent().
     */
    void applySelection();

    /**
       * \brief gets an arbitrary sqlite3 expression from the selection
       *
       * \param e The sqlite3 expression (typically "srid" or "sridid")
       */
    QString getSelectedExpression( QString e );

    /** Show the user a warning if the srs database could not be found */
    void showDBMissingWarning( const QString theFileName );
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
    long getLargestCRSIDMatch( QString theSql );

    //! add recently used CRS
    void insertRecent( long theCrsId );

    //! Has the Projection List been populated?
    bool mProjListDone;

    //! Has the User Projection List been populated?
    bool mUserProjListDone;

    //! Has the Recent Projection List been populated?
    bool mRecentProjListDone;

    //! Is there a pending selection to be made by CRS Name?
    bool mCRSNameSelectionPending;

    //! Is there a pending selection to be made by CRS ID?
    bool mCRSIDSelectionPending;

    //! Is there a pending selection to be made by Authority ID?
    bool mAuthIDSelectionPending;

    //! The CRS Name that wants to be selected on this widget
    QString mCRSNameSelection;

    //! The CRS ID that wants to be selected on this widget
    long mCRSIDSelection;

    //! The Authority ID that wants to be selected on this widget
    QString mAuthIDSelection;

    //! The set of OGC WMS CRSs that want to be applied to this widget
    QSet<QString> mCrsFilter;

    //! Most recently used projections (trimmed at 25 entries)
    QStringList mRecentProjections;

    //! hide deprecated CRSes
    void hideDeprecated( QTreeWidgetItem *item );

  private slots:
    /**private handler for when user selects a cs
     *it will cause wktSelected and sridSelected events to be spawned
     */
    void coordinateSystemSelected( QTreeWidgetItem* );

    //! get list of authorities
    QStringList authorities();

  signals:
    void sridSelected( QString theSRID );
    //! Refresh any listening canvases
    void refresh();
    //! Let listeners know if find has focus so they can adjust the default button
    void searchBoxHasFocus( bool );
};

#endif
