/***************************************************************************
                         qgssymbolsavedialog.h
                         -------------------------------------
    begin                : November 2016
    copyright            : (C) 2016 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLESAVEDIALOG_H
#define QGSSTYLESAVEDIALOG_H

#include <QDialog>
#include "ui_qgsstylesavedialog.h"

#include "qgsstyle.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \brief a dialog for setting properties of a newly saved style.
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsStyleSaveDialog: public QDialog, private Ui::QgsStyleSaveDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSymbolSaveDialog
     * \param parent parent widget
     * \param type the QgsStyle entity type being saved
     */
    QgsStyleSaveDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsStyle::StyleEntity type = QgsStyle::SymbolEntity );

    //! returns the text value of the name element
    QString name() const;

    //! returns the text value of the tags element
    QString tags() const;

    //! returns whether the favorite element is checked
    bool isFavorite() const;



};

#endif // QGSSTYLESAVEDIALOG_H
