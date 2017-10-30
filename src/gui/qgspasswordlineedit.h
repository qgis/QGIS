/***************************************************************************
                              qgspasswordlineedit.h
                              ------------------------
  begin                : March 13, 2017
  copyright            : (C) 2017 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPASSWORDLINEEDIT_H
#define QGSPASSWORDLINEEDIT_H

#include <QLineEdit>
#include <QAction>

#include "qgis_gui.h"

/**
 * \class QgsPasswordLineEdit
 * \ingroup gui
 * QLineEdit subclass with built in support for showing/hiding
 * entered password.
 * \since QGIS 3.0
 **/
class GUI_EXPORT QgsPasswordLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( bool showLockIcon READ showLockIcon WRITE setShowLockIcon )

  public:

    /**
     * Constructor for QgsPasswordLineEdit.
     * \param parent parent widget
     * \param passwordVisible Initial state of the password's visibility
     */
    QgsPasswordLineEdit( QWidget *parent = nullptr, bool passwordVisible = false );

    /**
     * Define if a lock icon shall be shown on the left of the widget
     * \param visible set to false to hide the lock icon
     */
    void setShowLockIcon( bool visible );

    /**
     * Returns if a lock icon shall be shown on the left of the widget
     */
    bool showLockIcon() const { return mLockIconVisible; }

    /**
     * Set state of the password's visibility
     */
    void setPasswordVisibility( bool visible );

  private slots:
    void togglePasswordVisibility( bool toggled );

  private:

    QAction *mActionShowHidePassword = nullptr;
    QAction *mActionLock = nullptr;

    QIcon mShowPasswordIcon;
    QIcon mHidePasswordIcon;

    bool mLockIconVisible = false;
    QSize mIconsSize;
};

#endif // QGSPASSWORDLINEEDIT_H
