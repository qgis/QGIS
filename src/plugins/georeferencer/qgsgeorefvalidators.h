/***************************************************************************
     qgsgeorefvalidators.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSGEOREFVALIDATORS_H
#define QGSGEOREFVALIDATORS_H

#include <QValidator>

class QgsDMSAndDDValidator : public QValidator
{
  public:
    QgsDMSAndDDValidator( QObject *parent );
    State validate( QString &input, int &pos ) const;
};

#endif // QGSGEOREFVALIDATORS_H
