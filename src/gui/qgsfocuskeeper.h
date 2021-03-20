/***************************************************************************
    qgsfocuskeeper.h
     ---------------
    Date                 : May 2020
    Copyright            : (C) 2020 Even Rouault
    Email                : even dot rouault at spatialys dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFOCUSKEEPER_H
#define QGSFOCUSKEEPER_H

#include "qgis_gui.h"

#define SIP_NO_FILE

#include <QObject>
#include <QPointer>

class QWidget;

/**
 * \ingroup gui
 * \class QgsFocusKeeper
 * \brief Trick to keep a widget focused and avoid QT crashes
 * \note not available in Python bindings
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFocusKeeper : public QObject
{
    Q_OBJECT

    QPointer<QWidget> mWidgetToKeepFocused;

  public:
    QgsFocusKeeper();
    ~QgsFocusKeeper() override;

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;
};

#endif // QGSFOCUSKEEPER_H
