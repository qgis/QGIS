/***************************************************************************
    qgsstylev2groupselectiondialog.h
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
#include "ui_qgsstylev2groupselectiondialogbase.h"


class QgsStyleV2;

/** \ingroup gui
 * \class QgsStyleV2GroupSelectionDialog
 */
class GUI_EXPORT QgsStyleV2GroupSelectionDialog : public QDialog, private Ui::SymbolsV2GroupSelectionDialogBase
{
    Q_OBJECT

  public:
    QgsStyleV2GroupSelectionDialog( QgsStyleV2* style, QWidget *parent = nullptr );
    ~QgsStyleV2GroupSelectionDialog();
    //! Set bold font for item
    void setBold( QStandardItem *item );

  signals:
    //! group with groupName has been selected
    void groupSelected( const QString& groupName );
    //! group with groupName has been deselected
    void groupDeselected( const QString& groupName );
    //! smartgroup with groupName has been selected
    void smartgroupSelected( const QString& groupName );
    //! smart group with groupName has been deselected
    void smartgroupDeselected( const QString& groupName );
    //! all deselected
    void allDeselected();
    //! all selected
    void allSelected();

  private slots:
    void groupTreeSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

  private:
    /**
     * @brief build group tree
     * @param parent
     */
    void buildGroupTree( QStandardItem *&parent );
    QgsStyleV2* mStyle;

};

#endif // QGSSTYLEV2GROUPSELECTIONDIALOG_H
