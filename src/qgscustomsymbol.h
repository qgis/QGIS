/***************************************************************************
                          qgscustomsymbol.h  -  description
                             -------------------
    begin                : Sat Jul 27 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSCUSTOMSYMBOL_H
#define QGSCUSTOMSYMBOL_H

#include "qgssymbol.h"

/**
  *@author Gary E.Sherman
  */

class QgsCustomSymbol : public QgsSymbol  {
public: 
	QgsCustomSymbol();
	~QgsCustomSymbol();
};

#endif
