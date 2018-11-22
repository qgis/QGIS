/***************************************************************************
  qgsproxystyle.h
  ---------------
  Date                 : March 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROXYSTYLE_H
#define QGSPROXYSTYLE_H

#include "qgis_sip.h"
#include "qgsgui.h"
#include <QProxyStyle>

/**
 * A QProxyStyle subclass which correctly sets the base style to match
 * the QGIS application style, and handles object lifetime by correctly
 * parenting to a parent widget.
 * \ingroup gui
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsProxyStyle : public QProxyStyle
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProxyStyle. Ownership is transferred to the \a parent widget.
     *
     * The base style for the QProxyStyle will be set to match the current QGIS application style.
     */
    explicit QgsProxyStyle( QWidget *parent SIP_TRANSFER );
};

#endif // QGSPROXYSTYLE_H
