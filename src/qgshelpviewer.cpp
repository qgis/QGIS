/***************************************************************************
                          qgshelpviewer.cpp 
 Simple help browser
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
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



#include <qvariant.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "qgshelpviewer.h"
QgsHelpViewer::QgsHelpViewer(QWidget * parent, const char *name, bool modal, WFlags fl):QgsHelpViewerBase(parent, name, modal, fl)
{

}

/*
 *  Destroys the object and frees any allocated resources
 */
QgsHelpViewer::~QgsHelpViewer()
{
  // no need to delete child widgets, Qt does it all for us
}
void QgsHelpViewer::showContent(QString path, QString doc)
{
  textBrowser->mimeSourceFactory()->addFilePath(path);
  textBrowser->setSource(doc);
}
