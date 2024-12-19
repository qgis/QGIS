/***************************************************************************
    qgscodeeditorjs.h - A Javascript editor based on QScintilla
     --------------------------------------
    Date                 : June 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORJS_H
#define QGSCODEEDITORJS_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

SIP_IF_MODULE( HAVE_QSCI_SIP )


/**
 * \ingroup gui
 * \brief A Javascript editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsCodeEditorJavascript : public QgsCodeEditor
{
    Q_OBJECT

  public:

    //! Constructor for QgsCodeEditorJavascript
    QgsCodeEditorJavascript( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    Qgis::ScriptLanguage language() const override;

  protected:
    void initializeLexer() override;
};

#endif // QGSCODEEDITORJS_H
