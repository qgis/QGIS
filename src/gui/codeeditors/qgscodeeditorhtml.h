/***************************************************************************
    qgscodeeditorhtml.h - A HTML editor based on QScintilla
     --------------------------------------
    Date                 : 20-Jul-2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow.nathan (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORHTML_H
#define QGSCODEEDITORHTML_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

SIP_IF_MODULE( HAVE_QSCI_SIP )

/**
 * \ingroup gui
 * \brief A HTML editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note may not be available in Python bindings, depending on platform support
 * \since QGIS 2.6
 */
class GUI_EXPORT QgsCodeEditorHTML : public QgsCodeEditor
{
    Q_OBJECT

  public:

    //! Constructor for QgsCodeEditorHTML
    QgsCodeEditorHTML( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::ScriptLanguage language() const override;

  protected:
    void initializeLexer() override;
};

#endif
