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

#include "ui_qgsprojectionselectorbase.h"

#include <QSet>
#include <QStringList>

#include "qgis.h"
#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"


class QgsVertexMarker;
class QResizeEvent;

/**
 * \class QgsProjectionSelectionTreeWidget
 * \ingroup gui
 * A widget for selecting a coordinate reference system from a tree.
 *
 * This widget implements a tree view of projections, as seen in the
 * QgsProjectionSelectionDialog dialog. In most cases it is more
 * suitable to use the compact QgsProjectionSelectionWidget widget.
 *
 * \see QgsProjectionSelectionDialog.
 * \see QgsProjectionSelectionWidget
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsProjectionSelectionTreeWidget : public QWidget, private Ui::QgsProjectionSelectorBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectionSelectionTreeWidget.
     */
    QgsProjectionSelectionTreeWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProjectionSelectionTreeWidget() override;

    /**
     * Returns the CRS currently selected in the widget.
     * \see setCrs()
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets whether a "no/invalid" projection option should be shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see showNoProjection()
     * \since QGIS 3.0
     */
    void setShowNoProjection( bool show );

    /**
     * Sets whether to show the bounnds preview map.
     * \see showBoundsMap()
     * \since QGIS 3.0
     */
    void setShowBoundsMap( bool show );

    /**
     * Returns whether the "no/invalid" projection option is shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see setShowNoProjection()
     * \since QGIS 3.0
     */
    bool showNoProjection() const;

    /**
     * Returns whether the bounds preview map is shown.
     * \see setShowBoundsMap()
     * \since QGIS 3.0
     */
    bool showBoundsMap() const;

    /**
     * Returns true if the current selection in the widget is a valid choice. Valid
     * selections include any projection and also the "no/invalid projection" option
     * (if setShowNoProjection() was called). Invalid selections are the group
     * headers (such as "Geographic Coordinate Systems"
     */
    bool hasValidSelection() const;

  public slots:

    /**
     * Sets the initial \a crs to show within the dialog.
     * \see crs()
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the initial "preview" rectangle for the bounds overview map.
     * \see previewRect()
     * \since QGIS 3.0
     */
    void setPreviewRect( const QgsRectangle &rect );

    /**
     * The initial "preview" rectangle for the bounds overview map.
     * \see previewRect()
     * \since QGIS 3.0
     */
    QgsRectangle previewRect() const;

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
     * \warning This function's behavior is undefined if it is called after the widget is shown.
     */
    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );

    /**
     * Marks the current selected projection for push to front of recent projections list.
     */
    void pushProjectionToFront();

  signals:

    /**
     * Emitted when a projection is selected in the widget.
     */
    void crsSelected();

    /**
     * Notifies others that the widget is now fully initialized, including deferred selection of projection.
     * \since QGIS 2.4
     */
    void initialized();

    /**
     * Emitted when a projection is double clicked in the list.
     * \since QGIS 2.14
     */
    void projectionDoubleClicked();

  protected:
    // Used to ensure the projection list view is actually populated
    void showEvent( QShowEvent *event ) override;

    // Used to manage column sizes
    void resizeEvent( QResizeEvent *event ) override;

  private:

    /**
     * \brief Populate the proj tree view with user defined projection names...
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     */
    void loadUserCrsList( QSet<QString> *crsFilter = nullptr );

    /**
     * \brief Populate the proj tree view with system projection names...
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     */
    void loadCrsList( QSet<QString> *crsFilter = nullptr );

    /**
     * \brief Makes a \a string safe for use in SQL statements.
     *  This involves escaping single quotes, double quotes, backslashes,
     *  and optionally, percentage symbols.  Percentage symbols are used
     *  as wildcards sometimes and so when using the string as part of the
     *  LIKE phrase of a select statement, should be escaped.
     * \returns The string made safe for SQL statements.
     */
    QString sqlSafeString( const QString &string ) const;

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
    QString ogcWmsCrsFilterAsSqlExpression( QSet<QString> *crsFilter );

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
    void applySelection( int column = QgsProjectionSelectionTreeWidget::None, QString value = QString() );

    /**
       * \brief gets an arbitrary sqlite3 expression from the selection
       *
       * \param e The sqlite3 expression (typically "srid" or "sridid")
       */
    QString getSelectedExpression( const QString &e ) const;

    QString selectedName();

    QString selectedProj4String();

    //! Gets the current QGIS projection identfier
    long selectedCrsId();

    //! Show the user a warning if the srs database could not be found
    void showDBMissingWarning( const QString &fileName );

    enum Roles
    {
      RoleDeprecated = Qt::UserRole,
    };

    // List view nodes for the tree view of projections
    //! User defined projections node
    QTreeWidgetItem *mUserProjList = nullptr;
    //! GEOGCS node
    QTreeWidgetItem *mGeoList = nullptr;
    //! PROJCS node
    QTreeWidgetItem *mProjList = nullptr;

    //! Users custom coordinate system file
    QString mCustomCsFile;
    //! File name of the sqlite3 database
    QString mSrsDatabaseFileName;

    /**
     * Utility method used in conjunction with name based searching tool
     */
    long getLargestCrsIdMatch( const QString &sql );

    //! add recently used CRS
    void insertRecent( long crsId );

    //! Has the Projection List been populated?
    bool mProjListDone = false;

    //! Has the User Projection List been populated?
    bool mUserProjListDone = false;


    //! Has the Recent Projection List been populated?
    bool mRecentProjListDone = false;

    enum Columns { NameColumn, AuthidColumn, QgisCrsIdColumn, None };
    int mSearchColumn = QgsProjectionSelectionTreeWidget::None;
    QString mSearchValue;

    bool mPushProjectionToFront = false;

    //! The set of OGC WMS CRSs that want to be applied to this widget
    QSet<QString> mCrsFilter;

    //! Most recently used projections (trimmed at 25 entries)
    QStringList mRecentProjections;

    //! Hide deprecated CRSes
    void hideDeprecated( QTreeWidgetItem *item );

    QgsRubberBand *mPreviewBand;
    QgsRubberBand *mPreviewBand2;
    QgsVertexMarker *mVertexMarker;

    bool mShowMap = true;

    QList<QgsMapLayer *> mLayers;

    QgsRectangle mPreviewRect;


  private slots:
    //! Gets list of authorities
    void updateBoundsPreview();
    QStringList authorities();

    //! Apply projection on double-click
    void lstCoordinateSystems_itemDoubleClicked( QTreeWidgetItem *current, int column );
    void lstRecent_itemDoubleClicked( QTreeWidgetItem *current, int column );
    void lstCoordinateSystems_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void lstRecent_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void updateFilter();
};

#endif
