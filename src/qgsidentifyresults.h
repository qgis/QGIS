/***************************************************************************
                          qgsidentifyresults.h  -  description
                             -------------------
    begin                : Fri Oct 25 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIDENTIFYRESULTS_H
#define QGSIDENTIFYRESULTS_H

#include "qgsidentifyresultsbase.h"

/**
  *@author Gary E.Sherman
  */

class QgsIdentifyResults:public QgsIdentifyResultsBase
{
  public:
	QgsIdentifyResults();
	~QgsIdentifyResults();
  /** No descriptions */
	void addAttribute(QString field, QString value);
	void setTitle(QString title);
};

#endif
