/***************************************************************************
                          qgsoptions.h
              Set user options and preferences
                             -------------------
    begin                : May 28, 2004
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
#ifndef QGSOPTIONS_H
#define QGSOPTIONS_H
class QString;
#ifdef WIN32
#include "qgsoptionsbase.h"
#else
#include "qgsoptionsbase.uic.h"
#endif
/**
 * \class QgsOptions
 * \brief Set user options and preferences
 */
class QgsOptions :public QgsOptionsBase{
  Q_OBJECT;
  public:
    /**
     * Constructor
     * @param parent Parent widget (usually a QgisApp)
     * @param name name for the widget
     */
    QgsOptions(QWidget *parent=0, const char *name=0);
    //! Destructor
    ~QgsOptions();
    /**
     * Return the currently selected theme
     * @return theme name (a directory name in the themes directory)
     */
    QString theme();
    public slots:
      //! Slot to change the theme this is handled when the user 
      // activates or highlights a theme name in the drop-down list
      void themeChanged(const QString &);
  private:
    //! Pointer to our parent
    QWidget *qparent;
};

#endif // #ifndef QGSOPTIONS_H
