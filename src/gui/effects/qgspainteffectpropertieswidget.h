/***************************************************************************
    qgspainteffectpropertieswidget.h
    --------------------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPAINTEFFECTPROPERTIESWIDGET_H
#define QGSPAINTEFFECTPROPERTIESWIDGET_H

#include "ui_qgseffectpropertieswidget.h"

class QgsPaintEffect;


/** \ingroup gui
 * \class QgsPaintEffectPropertiesWidget
 * \brief A widget which modifies the properties of a QgsPaintEffect
 *
 * \note Added in version 2.9
 */

class GUI_EXPORT QgsPaintEffectPropertiesWidget : public QWidget, private Ui::EffectPropertiesWidget
{
    Q_OBJECT

  public:

    /** QgsPaintEffectPropertiesWidget constructor
     * @param effect QgsPaintEffect to modify in the widget
     * @param parent parent widget
     */
    QgsPaintEffectPropertiesWidget( QgsPaintEffect* effect, QWidget* parent = nullptr );

  public slots:

    /** Update widget when effect type changes
     */
    void effectTypeChanged();

    /** Emits the changed signal
     */
    void emitSignalChanged();

  signals:

    /** Emitted when paint effect properties changes
     */
    void changed();

    /** Emitted when paint effect type changes
     */
    void changeEffect( QgsPaintEffect* effect );

  private:

    QgsPaintEffect* mEffect;

    void populateEffectTypes();
    void updateEffectWidget( QgsPaintEffect* effect );

};

#endif //QGSPAINTEFFECTPROPERTIESWIDGET_H
