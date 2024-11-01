/***************************************************************************
    qgshighlightablelineedit.h
     -------------------------
    Date                 : March 2020
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

#ifndef QGSHIGHLIGHTABLELINEEDIT_H
#define QGSHIGHLIGHTABLELINEEDIT_H

#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsfilterlineedit.h"

/**
 * \class QgsHighlightableLineEdit
 * \ingroup gui
 *
 * \brief A QgsFilterLineEdit subclass with the ability to "highlight" the edges of the widget.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsHighlightableLineEdit : public QgsFilterLineEdit
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsHighlightableLineEdit, with the specified \a parent widget.
     */
    QgsHighlightableLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns TRUE if the line edit is currently highlighted.
     * \see setHighlighted()
     */
    bool isHighlighted() const { return mHighlight; }

    /**
     * Sets whether the line edit is currently \a highlighted.
     * \see isHighlighted()
     */
    void setHighlighted( bool highlighted );

  protected:
    void paintEvent( QPaintEvent *e ) override;

  private:
    bool mHighlight = false;
};


#endif // QGSHIGHLIGHTABLELINEEDIT_H
