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

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"

class QResizeEvent;
class QgsCoordinateReferenceSystemProxyModel;

/**
 * \class QgsProjectionSelectionTreeWidget
 * \ingroup gui
 * \brief A widget for selecting a coordinate reference system from a tree.
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
     * Sets whether to show the bounds preview map.
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
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setShowNoProjection().
     * \since QGIS 3.16
     */
    void setNotSetText( const QString &text );

    /**
     * Returns whether the bounds preview map is shown.
     * \see setShowBoundsMap()
     * \since QGIS 3.0
     */
    bool showBoundsMap() const;

    /**
     * Returns TRUE if the current selection in the widget is a valid choice. Valid
     * selections include any projection and also the "no/invalid projection" option
     * (if setShowNoProjection() was called). Invalid selections are the group
     * headers (such as "Geographic Coordinate Systems")
     */
    bool hasValidSelection() const;

    /**
     * The initial "preview" rectangle for the bounds overview map.
     * \see previewRect()
     * \since QGIS 3.0
     */
    QgsRectangle previewRect() const;

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
     * \brief Filters this widget by the given CRSs.
     *
     * Sets this widget to filter the available projections to those listed
     * by the given Coordinate Reference Systems.
     *
     * \param crsFilter a list of the authority:id strings for Coordinate Reference Systems to include
     * in the widget.
     */
    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );

    /**
     * Marks the current selected projection for push to front of recent projections list.
     *
     * \deprecated Has no effect since QGIS 3.20
     */
    Q_DECL_DEPRECATED void pushProjectionToFront() SIP_DEPRECATED;

    /**
     * Clear the list of recent projections.
     *
     * \since QGIS 3.32
     */
    void clearRecentCrs();

  signals:

    /**
     * Emitted when a projection is selected in the widget.
     */
    void crsSelected();

    /**
     * Notifies others that the widget is now fully initialized, including deferred selection of projection.
     * \deprecated no longer emitted
     */
    Q_DECL_DEPRECATED void initialized() SIP_DEPRECATED;

    /**
     * Emitted when a projection is double clicked in the list.
     * \since QGIS 2.14
     */
    void projectionDoubleClicked();

    /**
     * Emitted when the selection in the tree is changed from a valid selection to an invalid selection, or vice-versa.
     *
     * \since QGIS 3.18
     */
    void hasValidSelectionChanged( bool isValid );

  protected:

    // Used to manage column sizes
    void resizeEvent( QResizeEvent *event ) override;

    // Used to catch key presses on the recent projections list
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private:

    void loadUnknownCrs( const QgsCoordinateReferenceSystem &crs );

    void selectCrsByAuthId( const QString &authid );

    enum Roles
    {
      RoleDeprecated = Qt::UserRole,
      RoleWkt,
      RoleProj
    };

    QgsCoordinateReferenceSystemProxyModel *mCrsModel = nullptr;

    //! add recently used CRS
    void insertRecent( const QgsCoordinateReferenceSystem &crs );

    enum Columns { NameColumn, AuthidColumn, QgisCrsIdColumn, ClearColumn };

    //! Most recently used projections
    QList< QgsCoordinateReferenceSystem > mRecentProjections;

    bool mShowMap = true;

    bool mBlockSignals = false;

  private slots:

    void updateBoundsPreview();

    //! Apply projection on double-click
    void lstCoordinateSystemsDoubleClicked( const QModelIndex &index );
    void lstRecent_itemDoubleClicked( QTreeWidgetItem *current, int column );
    void lstCoordinateSystemsSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void lstRecent_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void filterRecentCrsList();

    void removeRecentCrsItem( QTreeWidgetItem *item );
};

#endif
