/***************************************************************************
  qgsbrowsertreeview.h
  --------------------------------------
  Date                 : January 2015
  Copyright            : (C) 2015 by Radim Blazek
  Email                : radim.blazek@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBROWSERTREEVIEW_H
#define QGSBROWSERTREEVIEW_H

#include <QTreeView>
#include "qgis_sip.h"
#include "qgis_gui.h"

#ifdef SIP_RUN
// this is needed for the "convert to subclass" code below to compile
% ModuleHeaderCode
#include "qgsbrowsertreeview.h"
% End
#endif

class QgsBrowserGuiModel;
class QgsDataItem;

/**
 * \ingroup gui
 * \brief The QgsBrowserTreeView class extends QTreeView with save/restore tree state functionality.
 *
 * \see QgsBrowserModel
 * \since QGIS 2.8
 */
class GUI_EXPORT QgsBrowserTreeView : public QTreeView
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsBrowserTreeView *>( sipCpp ) != nullptr )
      sipType = sipType_QgsBrowserTreeView;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT
  public:

    //! Constructor for QgsBrowserTreeView
    QgsBrowserTreeView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setModel( QAbstractItemModel *model ) override;
    //! Sets the browser model
    void setBrowserModel( QgsBrowserGuiModel *model );
    //! Returns the browser model
    QgsBrowserGuiModel *browserModel() { return mBrowserModel; }
    void showEvent( QShowEvent *e ) override;
    void hideEvent( QHideEvent *e ) override;

    // returns true if at least one descendat is expanded, used in refresh
    bool hasExpandedDescendant( const QModelIndex &index ) const;

    // Set section where to store settings (because we have 2 browser dock widgets)
    void setSettingsSection( const QString &section ) { mSettingsSection = section; }

    /**
     * Sets the \a item currently selected in the view.
     *
     * Returns TRUE if the item was found and could be selected.
     *
     * \since QGIS 3.28
     */
    bool setSelectedItem( QgsDataItem *item );

    /**
     * Expands out a file \a path in the view.
     *
     * The \a path must correspond to a valid directory existing on the file system.
     *
     * \since QGIS 3.28
     */
    void expandPath( const QString &path );

  protected:

    void keyPressEvent( QKeyEvent *event ) override;

  protected slots:
    void rowsInserted( const QModelIndex &parentIndex, int start, int end ) override;

  private:
    QString mSettingsSection;
    // initial expanded paths
    QStringList mExpandPaths;
    void saveState();
    void restoreState();

    QString expandedPathsKey() const;

    // Get list of expanded items paths recursively
    QStringList expandedPathsList( const QModelIndex &index );

    // Expand path recursively to root
    void expandTree( const QModelIndex &index );

    // returns true if expanded from root to item
    bool treeExpanded( const QModelIndex &index );
    QgsBrowserGuiModel *mBrowserModel = nullptr;
};

#endif // QGSBROWSERTREEVIEW_H
