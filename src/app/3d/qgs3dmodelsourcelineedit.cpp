/***************************************************************************
 qgs3dmodelsourcelineedit.cpp
 -----------------------
 begin                : July 2020
 copyright            : (C) 2020 by Mathieu Pellerin
 email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmodelsourcelineedit.h"

//
// Qgs3DModelSourceLineEdit
//

///@cond PRIVATE

QString Qgs3DModelSourceLineEdit::fileFilter() const
{
  return tr( "All files" ) + " (*.*)";
}

QString Qgs3DModelSourceLineEdit::selectFileTitle() const
{
  return tr( "Select 3D Model File" );
}

QString Qgs3DModelSourceLineEdit::fileFromUrlTitle() const
{
  return tr( "3D Model From URL" );
}

QString Qgs3DModelSourceLineEdit::fileFromUrlText() const
{
  return tr( "Enter 3D Model URL" );
}

QString Qgs3DModelSourceLineEdit::embedFileTitle() const
{
  return tr( "Embed 3D Model File" );
}

QString Qgs3DModelSourceLineEdit::extractFileTitle() const
{
  return tr( "Extract 3D Model File" );
}

QString Qgs3DModelSourceLineEdit::defaultSettingsKey() const
{
  return QStringLiteral( "/UI/last3DModelDir" );
}

///@endcond
