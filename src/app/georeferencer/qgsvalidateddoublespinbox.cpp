/***************************************************************************
    qgsvalidateddoublespinbox.cpp
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

#include "qgsvalidateddoublespinbox.h"
#include "moc_qgsvalidateddoublespinbox.cpp"

QgsValidatedDoubleSpinBox::QgsValidatedDoubleSpinBox( QWidget *widget )
  : QDoubleSpinBox( widget )
{}

QValidator::State QgsValidatedDoubleSpinBox::validate( QString &input, int &pos ) const
{
  const QValidator::State state = QDoubleSpinBox::validate( input, pos );
  if ( state != QValidator::Acceptable )
  {
    return state;
  }

  // A value of zero is acceptable as intermediate result,
  // but not as final entry
  const double val = valueFromText( input );
  if ( val == 0.0 )
  {
    return QValidator::Intermediate;
  }
  return QValidator::Acceptable;
}

QAbstractSpinBox::StepEnabled QgsValidatedDoubleSpinBox::stepEnabled() const
{
  StepEnabled mayStep = StepNone;

  // Zero is off limits, so handle the logic differently
  // (always exclude zero from the permitted interval)
  if ( minimum() == 0.0 )
  {
    if ( value() - singleStep() > minimum() )
    {
      mayStep |= StepDownEnabled;
    }
  }
  else // closed interval
  {
    if ( value() - singleStep() >= minimum() )
    {
      mayStep |= StepDownEnabled;
    }
  }

  if ( maximum() == 0.0 )
  {
    if ( value() + singleStep() < maximum() )
    {
      mayStep |= StepUpEnabled;
    }
  }
  else
  {
    if ( value() + singleStep() <= maximum() )
    {
      mayStep |= StepUpEnabled;
    }
  }
  return mayStep;
}
