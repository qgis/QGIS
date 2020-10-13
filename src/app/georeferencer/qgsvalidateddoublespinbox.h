/***************************************************************************
    qgsvalidateddoublespinbox.h - Simple extension to QDoubleSpinBox which
    implements a validate function to disallow zero as input.
     --------------------------------------
    Date                 : 23-Feb-2010
    Copyright            : (c) 2010 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALIDATEDDOUBLESPINBOX_H
#define QGSVALIDATEDDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class QgsValidatedDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

  public:
    QgsValidatedDoubleSpinBox( QWidget *widget );

    QValidator::State validate( QString &input, int &pos ) const override;
    StepEnabled stepEnabled() const override;
};

#endif
