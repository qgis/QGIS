/***************************************************************************
    qgsmaplayeractioncontext.h
    ---------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERACTIONCONTEXT_H
#define QGSMAPLAYERACTIONCONTEXT_H

#include "qgis_sip.h"
#include <QPointer>

#include "qgis.h"
#include "qgis_gui.h"

class QgsAttributeDialog;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Encapsulates the context in which a QgsMapLayerAction action is executed.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsMapLayerActionContext
{
  public:

    QgsMapLayerActionContext();

    /**
     * Returns the attribute dialog associated with the action's execution.
     *
     * May be NULLPTR if the action is not being executed from an attribute dialog.
     *
     * \see setAttributeDialog()
     */
    QgsAttributeDialog *attributeDialog() const;

    /**
     * Sets the attribute \a dialog associated with the action's execution.
     *
     * \see attributeDialog()
     */
    void setAttributeDialog( QgsAttributeDialog *dialog );

    /**
     * Returns the message bar associated with the action's execution.
     *
     * May be NULLPTR.
     *
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const;

    /**
     * Sets the message \a bar associated with the action's execution.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

  private:

    QPointer< QgsAttributeDialog > mAttributeDialog;
    QPointer< QgsMessageBar > mMessageBar;
};

Q_DECLARE_METATYPE( QgsMapLayerActionContext )

#endif // QGSMAPLAYERACTIONCONTEXT_H
