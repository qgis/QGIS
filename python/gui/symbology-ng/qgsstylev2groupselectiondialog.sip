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


class QgsStyleV2GroupSelectionDialog : public QDialog, private Ui::SymbolsV2GroupSelectionDialogBase
{
%TypeHeaderCode
#include <qgsstylev2groupselectiondialog.h>
%End

  public:
    QgsStyleV2GroupSelectionDialog( QgsStyleV2* style, QWidget *parent = 0 );
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

};

