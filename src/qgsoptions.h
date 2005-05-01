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
      //! Slot called when user chooses to change the project wide projection.
      void pbnSelectProjection_clicked();
      void findBrowser();
      void setCurrentTheme();
      void addTheme(QString item);
      void cbxHideSplash_toggled( bool );
      void saveOptions();
      
    /**
     * Return the desired state of newly added layers. If a layer
     * is to be drawn when added to the map, this function returns
     * true.
     */
    bool newVisible();
  private:
    //! Pointer to our parent
    QWidget *qparent;
    //!Global default projection used for new layers added that have no projection
    long mGlobalSRSID;
};

#endif // #ifndef QGSOPTIONS_H
