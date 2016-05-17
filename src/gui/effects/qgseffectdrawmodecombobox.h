/***************************************************************************
    qgseffectdrawmodecombobox.h
    ---------------------------
    begin                : March 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEFFECTDRAWMODECOMBOBOX_H
#define QGSEFFECTDRAWMODECOMBOBOX_H

#include <QComboBox>
#include "qgspainteffect.h"

/** \ingroup gui
 * \class QgsEffectDrawModeComboBox
 * \brief A combo box allowing selection of paint effect draw modes
 *
 * \note Added in version 2.9
 */

class GUI_EXPORT QgsEffectDrawModeComboBox : public QComboBox
{
    Q_OBJECT

  public:

    QgsEffectDrawModeComboBox( QWidget* parent = nullptr );

    /** Returns the currently selected draw mode for the combo box
     * @returns current draw mode
     */
    QgsPaintEffect::DrawMode drawMode() const;

    /** Sets the currently selected draw mode for the combo box
     * @param drawMode selected draw mode
     */
    void setDrawMode( QgsPaintEffect::DrawMode drawMode );

};

#endif //QGSEFFECTDRAWMODECOMBOBOX_H
