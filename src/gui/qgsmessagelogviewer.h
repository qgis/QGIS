/***************************************************************************
                          qgsmessagelogviewer.h  -  description
                             -------------------
    begin                : October 2011
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGELOGVIEWER_H
#define QGSMESSAGELOGVIEWER_H

#include <ui_qgsmessagelogviewer.h>
#include <qgisgui.h>
#include "qgsmessagelog.h"

#include <QString>

class QStatusBar;
class QToolButton;
class QShowEvent;
class QHideEvent;

/** \ingroup gui
 * A generic message for displaying QGIS log messages.
 * \note added in 1.8
 */
class GUI_EXPORT QgsMessageLogViewer: public QDialog, private Ui::QgsMessageLogViewer
{
    Q_OBJECT
  public:
    QgsMessageLogViewer( QStatusBar *statusBar = 0, QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsMessageLogViewer();

    //! @note added in 2.4
    void setShowToolTips( bool enabled ) { mShowToolTips = enabled; }
    //! @note added in 2.4
    bool showToolTips() const { return mShowToolTips; }

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );

  protected:
    void showEvent( QShowEvent * );
    void hideEvent( QHideEvent * );

  private:
    QToolButton *mButton;
    int mCount;
    bool mShowToolTips;

  private slots:
    void closeTab( int index );
    void buttonToggled( bool checked );
    void buttonDestroyed();
};

#endif
