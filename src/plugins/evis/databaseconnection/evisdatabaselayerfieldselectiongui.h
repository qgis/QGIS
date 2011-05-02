/*
** File: evisdatabaselayerfieldselectiongui.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-07
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
/*  $Id$ */
#ifndef eVisDatabaseLayerFieldSelectionGui_H
#define eVisDatabaseLayerFieldSelectionGui_H

#include <QDialog>
#include <ui_evisdatabaselayerfieldselectionguibase.h>

/**
* \class eVisDatabaseLayerFieldSelectionGui
* \brief GUI component that allows user to select field names that hold the x, y coordinates for the feature
* This class provides the GUI component that allows the user to enter a name for a newly created layer and also
* select the field names that hold the x, y coordinates for the features in this layer.
*/
class eVisDatabaseLayerFieldSelectionGui : public QDialog, private Ui::eVisDatabaseLayerFieldSelectionGuiBase
{
    Q_OBJECT

  public:
    /** \brief Constructor */
    eVisDatabaseLayerFieldSelectionGui( QWidget* parent, Qt::WFlags fl );

    /** \brief Public method that sets the contents of the combo boxes with the available field names */
    void setFieldList( QStringList* );

  public slots:
    void on_buttonBox_accepted( );
    void on_buttonBox_rejected( );

  signals:
    /** \brief Signal emitted when the user has entered the layername, selected the field names, and pressed the accept button */
    void eVisDatabaseLayerFieldsSelected( QString, QString, QString );
};
#endif
