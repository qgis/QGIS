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

#include "qgisgui.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>

/** \ingroup gui
 * A generic dialog with layout and button box
 */
class GUI_EXPORT QgsDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsDialog( QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags,
               QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close,
               Qt::Orientation orientation = Qt::Horizontal );
    ~QgsDialog();

    //! Returns the central layout. Widgets added to it must have this dialog as parent.
    QVBoxLayout *layout() { return mLayout; }
    //! Returns the button box.
    QDialogButtonBox *buttonBox() { return mButtonBox; }

  protected:
    QVBoxLayout *mLayout;
    QDialogButtonBox *mButtonBox;
};

#endif
