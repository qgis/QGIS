/***************************************************************************
                         qgsadvancedattrsearch.h 
                dialog for entering advanced search strings
                          --------------------
    begin                : 2005-09-08
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
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

#ifndef QGSADVANCEDATTRSEARCH_H
#define QGSADVANCEDATTRSEARCH_H

#include "qgsadvancedattrsearchbase.uic.h"

class QgsAdvancedAttrSearch : public QgsAdvancedAttrSearchBase
{
  Q_OBJECT
      
  public:
    QgsAdvancedAttrSearch(QWidget *parent = 0, const char *name = 0);

  public slots:
    virtual void showHelp();

};

#endif
