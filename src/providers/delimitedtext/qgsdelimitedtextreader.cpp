/***************************************************************************
  qgsdelimitedtextreader.cpp -  Data provider for delimted text
  -------------------
          begin                : 2012-01-20
          copyright            : (C) 201 by Chris Crook
          email                : ccrook at linz.govt.nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdelimitedtextreader.h"

#include <QtGlobal>
#include <QFile>
#include <QDataStream>
#include <QTextStream>

