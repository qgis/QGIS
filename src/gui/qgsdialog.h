/***************************************************************************
                          qgsdialog.h
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIALOG_H
#define QGSDIALOG_H

#include "qgsguiutils.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include "qgis_gui.h"
#include "qgis.h"

/**
 * \ingroup gui
 * A generic dialog with layout and button box
 */
class GUI_EXPORT QgsDialog : public QDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDialog.
     */
    QgsDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
               QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close,
               Qt::Orientation orientation = Qt::Horizontal );

    //! Returns the central layout. Widgets added to it must have this dialog as parent.
    QVBoxLayout *layout() { return mLayout; }
    //! Returns the button box.
    QDialogButtonBox *buttonBox() { return mButtonBox; }

  protected:
    QVBoxLayout *mLayout = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif
