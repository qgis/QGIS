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
#include "qgscoordinatereferencesystemmodel.h"
#include "qgsrecentcoordinatereferencesystemsmodel.h"

#include <QIdentityProxyModel>
#include <QStyledItemDelegate>

class QResizeEvent;
class QTreeWidgetItem;

///@cond PRIVATE
// proxy to expand base recent crs model to three column model
class QgsRecentCoordinateReferenceSystemTableModel : public QgsRecentCoordinateReferenceSystemsProxyModel SIP_SKIP
{
    Q_OBJECT
  public:
    QgsRecentCoordinateReferenceSystemTableModel( QObject *parent );
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
};

class RemoveRecentCrsDelegate : public QStyledItemDelegate SIP_SKIP
{
    Q_OBJECT

  public:
    RemoveRecentCrsDelegate( QObject *parent );
    bool eventFilter( QObject *obj, QEvent *event ) override;

  protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    void setHoveredIndex( const QModelIndex &index );

    QModelIndex mHoveredIndex;
};


///@endcond PRIVATE

/**
 * \class QgsProjectionSelectionTreeWidget
 * \ingroup gui
 * \brief A widget for selecting a coordinate reference system from a tree.
 *
 * This widget implements a tree view of projections, as seen in the
 * QgsProjectionSelectionDialog dialog. In most cases it is more
 * suitable to use the compact QgsProjectionSelectionWidget widget.
 *
 * \see QgsProjectionSelectionDialog
 * \see QgsProjectionSelectionWidget
 */

class GUI_EXPORT QgsProjectionSelectionTreeWidget : public QWidget, private Ui::QgsProjectionSelectorBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProjectionSelectionTreeWidget, with the specified \a parent widget.
     *
     * Since QGIS 3.34, the optional \a filter argument can be used to specify filters on the systems
     * shown in the widget. The default is to show all horizontal and compound CRS in order to match
     * the behavior of older QGIS releases. The \a filter can be altered to also include vertical CRS if desired.
     */
    QgsProjectionSelectionTreeWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsCoordinateReferenceSystemProxyModel::Filters filters = QgsCoordinateReferenceSystemProxyModel::FilterHorizontal | QgsCoordinateReferenceSystemProxyModel::FilterCompound );

    ~QgsProjectionSelectionTreeWidget() override;

    /**
     * Returns the CRS currently selected in the widget.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets whether a "no/invalid" projection option should be shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see showNoProjection()
     */
    void setShowNoProjection( bool show );

    /**
     * Sets whether to show the bounds preview map.
     * \see showBoundsMap()
     */
    void setShowBoundsMap( bool show );

    /**
     * Returns whether the "no/invalid" projection option is shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see setShowNoProjection()
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
     */
    QgsRectangle previewRect() const;

    /**
     * Returns the filters set on the available CRS.
     *
     * \see setFilters()
     * \since QGIS 3.34
     */
    QgsCoordinateReferenceSystemProxyModel::Filters filters() const;

    /**
     * Sets \a filters for the available CRS.
     *
     * \see filters()
     * \since QGIS 3.34
     */
    void setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters );

  public slots:

    /**
     * Sets the initial \a crs to show within the dialog.
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the initial "preview" rectangle for the bounds overview map.
     * \see previewRect()
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
     * \deprecated QGIS 3.40. Has no effect since QGIS 3.20.
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
     * \deprecated QGIS 3.40. No longer emitted.
     */
    Q_DECL_DEPRECATED void initialized() SIP_DEPRECATED;

    /**
     * Emitted when a projection is double clicked in the list.
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
    QgsRecentCoordinateReferenceSystemTableModel *mRecentCrsModel = nullptr;

    enum Columns
    {
      NameColumn,
      AuthidColumn,
      ClearColumn
    };

    bool mShowMap = true;

    bool mBlockSignals = false;

  private slots:

    void updateBoundsPreview();

    //! Apply projection on double-click
    void lstCoordinateSystemsDoubleClicked( const QModelIndex &index );
    void lstRecentDoubleClicked( const QModelIndex &index );
    void lstRecentClicked( const QModelIndex &index );
    void lstCoordinateSystemsSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void lstRecentSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    void removeRecentCrsItem( const QModelIndex &index );
};

#endif
