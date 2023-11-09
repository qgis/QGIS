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

    /**
     * Returns the entered name for the new symbol.
     */
    QString name() const;

    /**
     * Sets the default \a tags for the newly created item.
     *
     * \since QGIS 3.10
     */
    void setDefaultTags( const QString &tags );

    /**
     * Returns any tags entered for the new symbol (as a comma separated value list).
     */
    QString tags() const;

    /**
     * Returns TRUE if the favorite is checked for the symbol.
     */
    bool isFavorite() const;

    /**
     * Returns the type of style entity to save.
     * \since QGIS 3.10
     */
    QgsStyle::StyleEntity selectedType() const;

    /**
     * Returns the destination style database.
     *
     * \since QGIS 3.26
     */
    QgsStyle *destinationStyle();

  private:

    QgsStyle::StyleEntity mType = QgsStyle::SymbolEntity;

};

#endif // QGSSTYLESAVEDIALOG_H
