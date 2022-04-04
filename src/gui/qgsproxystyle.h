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

///@cond PRIVATE
#ifndef SIP_RUN

/**
 * Application style, applies custom style overrides for the QGIS application.
 * \ingroup gui
 */
class GUI_EXPORT QgsAppStyle : public QProxyStyle
{
    Q_OBJECT

  public:

    explicit QgsAppStyle( const QString &base );
    QPixmap generatedIconPixmap( QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt ) const override;
    void polish( QWidget *widget ) override;

    QString baseStyle() const { return mBaseStyle; }

    /**
     * Returns a new QgsAppStyle instance, with the same base style as this instance.
     *
     * Caller takes ownership of the returned object.
     */
    QProxyStyle *clone() SIP_FACTORY;

  private:

    QString mBaseStyle;
};

#endif
///@endcond

#endif // QGSPROXYSTYLE_H
