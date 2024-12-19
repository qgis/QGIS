/***************************************************************************
    qgscodeeditorjson.h - A JSON editor based on QScintilla
     --------------------------------------
    Date                 : 4.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORJSON_H
#define QGSCODEEDITORJSON_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

SIP_IF_MODULE( HAVE_QSCI_SIP )


/**
 * \ingroup gui
 * \brief A JSON editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsCodeEditorJson : public QgsCodeEditor
{
    Q_OBJECT

  public:

    //! Constructor for QgsCodeEditorJson
    QgsCodeEditorJson( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::ScriptLanguage language() const override;

  protected:
    void initializeLexer() override;

};

#endif // QGSCODEEDITORJSON_H
