/***************************************************************************
                             qgswindowmanagerinterface.h
                             ---------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPWINDOWMANAGER_H
#define QGSAPPWINDOWMANAGER_H

#include "qgis.h"
#include "qgswindowmanagerinterface.h"
#include <QPointer>

class QgsStyleManagerDialog;

/**
 * \ingroup gui
 * \brief Implementation of QgsWindowManagerInterface for the QGIS application.
 */
class QgsAppWindowManager : public QgsWindowManagerInterface
{
  public:

    QgsAppWindowManager() = default;
    ~QgsAppWindowManager();

    QWidget *openStandardDialog( QgsWindowManagerInterface::StandardDialog dialog ) override;

  private:
    QPointer< QgsStyleManagerDialog > mStyleManagerDialog;
};


#endif // QGSAPPWINDOWMANAGER_H
