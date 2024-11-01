/***************************************************************************
    qgsstylegroupselectiondialog.h
    ---------------------
    begin                : Oct 2015
    copyright            : (C) 2015 by Alessandro Pasotti
    email                : elpaso at itopen dot it

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEV2GROUPSELECTIONDIALOG_H
#define QGSSTYLEV2GROUPSELECTIONDIALOG_H

#include <QDialog>
#include <QStandardItem>
#include "ui_qgsstylegroupselectiondialogbase.h"
#include "qgis_gui.h"


class QgsStyle;

/**
 * \ingroup gui
 * \class QgsStyleGroupSelectionDialog
 */
class GUI_EXPORT QgsStyleGroupSelectionDialog : public QDialog, private Ui::SymbolsGroupSelectionDialogBase
{
    Q_OBJECT

  public:
    QgsStyleGroupSelectionDialog( QgsStyle *style, QWidget *parent = nullptr );
    //! Sets bold font for item
    void setBold( QStandardItem *item );

  signals:
    //! tag with tagName has been selected
    void tagSelected( const QString &tagName );
    //! tag with tagName has been deselected
    void tagDeselected( const QString &tagName );
    //! smartgroup with groupName has been selected
    void smartgroupSelected( const QString &groupName );
    //! smart group with groupName has been deselected
    void smartgroupDeselected( const QString &groupName );
    //! all deselected
    void allDeselected();
    //! all selected
    void allSelected();

    /**
     * Favorites has been deselected
     * \since QGIS 3.14
     */
    void favoritesDeselected();

    /**
     * Favorites has need selected
     * \since QGIS 3.14
     */
    void favoritesSelected();

  private slots:
    void groupTreeSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  private:
    /**
     * \brief build group tree
     * \param parent
     */
    void buildTagTree( QStandardItem *&parent );
    QgsStyle *mStyle = nullptr;
};

#endif // QGSSTYLEV2GROUPSELECTIONDIALOG_H
