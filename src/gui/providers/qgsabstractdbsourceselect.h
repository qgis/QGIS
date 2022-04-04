/***************************************************************************
   qgsabstractdbsourceselect.h
    --------------------------------------
   Date                 : 08.11.2021
   Copyright            : (C) 2021 Denis Rouzaud
   Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSABSTRACTDBSOURCESELECT_H
#define QGSABSTRACTDBSOURCESELECT_H


#include "qgis_gui.h"
#include "ui_qgsdbsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"

class QSortFilterProxyModel;
class QgsAbstractDbTableModel;
class QItemDelegate;

/**
 * \ingroup gui
 * \brief The QgsAbstractDbSourceSelect class is a base class for database source widget selector
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsAbstractDbSourceSelect : public QgsAbstractDataSourceWidget, protected Ui::QgsDbSourceSelectBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsAbstractDbSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  protected:
    //! Sets the source model for the table and optionally a delegate
    void init( QgsAbstractDbTableModel *model, QItemDelegate *delegate = nullptr );

    //! Returns the proxy model used to filter the results
    QSortFilterProxyModel *proxyModel() {return mProxyModel;}

    QPushButton *mBuildQueryButton = nullptr;

  protected slots:

    /**
     * This is called to define the SQL query and must be re-implemented.
     * The implementation should call QgsAbstractDbTableModel::setSql
     * The index is in the source model.
     */
    virtual void setSql( const QModelIndex &index ) = 0;

    //! Called on click for the source \a index
    virtual void treeviewClicked( const QModelIndex &index );

    //! Called on double-click for the source \a index
    virtual void treeviewDoubleClicked( const QModelIndex &index );

  private:
    void filterResults();

    QSortFilterProxyModel *mProxyModel = nullptr;
    QMenu *mSearchSettingsMenu = nullptr;

    QAction *mSearchColumnAllAction = nullptr;
    QList<QAction *> mSearchColumnActions;
    QAction *mSearchModeWildCardAction = nullptr;
    QAction *mSearchModeRegexAction = nullptr;
};

#endif // QGSABSTRACTDBSOURCESELECT_H
