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
#include "ui_qgsoptionsbase.h"
/**
 * \class QgsOptions
 * \brief Set user options and preferences
 */
class QgsOptions :public QDialog, private Ui::QgsOptionsBase
{
  Q_OBJECT;
  public:
    /**
     * Constructor
     * @param parent Parent widget (usually a QgisApp)
     * @param name name for the widget
     * @param modal true for modal dialog
     */
    QgsOptions(QWidget *parent=0, const char *name=0, bool modal=true);
    //! Destructor
    ~QgsOptions();
    /**
     * Return the currently selected theme
     * @return theme name (a directory name in the themes directory)
     */
    QString theme();

    public slots:
      //! Slot called when user chooses to change the project wide projection.
      void on_pbnSelectProjection_clicked();
      void on_btnFindBrowser_clicked();
      void on_cbxHideSplash_toggled( bool );
      void saveOptions();
    //! Slot to change the theme this is handled when the user 
    // activates or highlights a theme name in the drop-down list
    void themeChanged(const QString &);
      
    /**
     * Return the desired state of newly added layers. If a layer
     * is to be drawn when added to the map, this function returns
     * true.
     */
    bool newVisible();
  protected:
    //! Populates combo box with ellipsoids
    void getEllipsoidList();
    
    QString getEllipsoidAcronym(QString theEllipsoidName);
    QString getEllipsoidName(QString theEllipsoidAcronym);

  private:
    //! Pointer to our parent
    QWidget *qparent;
    //!Global default projection used for new layers added that have no projection
    long mGlobalSRSID;
};

#endif // #ifndef QGSOPTIONS_H
