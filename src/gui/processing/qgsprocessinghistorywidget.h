/***************************************************************************
                             qgsprocessinghistorywidget.h
                             ------------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGHISTORYWIDGET_H
#define QGSPROCESSINGHISTORYWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgspanelwidget.h"

#include <QDialog>

class QgsHistoryWidget;
class QDialogButtonBox;

/**
 * \ingroup gui
 * \brief A widget for showing Processing algorithm execution history.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingHistoryWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingHistoryWidget, with the specified \a parent widget.
     */
    QgsProcessingHistoryWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  public slots:

    /**
     * Clears the Processing history (after user confirmation).
     */
    void clearHistory();

    /**
     * Opens helps for the widget.
     */
    void openHelp();

    /**
     * Interactively allows users to save the history log.
     */
    void saveLog();

  private:
    QgsHistoryWidget *mHistoryWidget = nullptr;
};

/**
 * \ingroup gui
 * \brief A dialog for showing Processing algorithm execution history.
 *
 * \warning This is not part of stable API -- it is exposed to Python for internal use by Processing only!
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingHistoryDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingHistoryDialog.
     */
    QgsProcessingHistoryDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private:
    QgsProcessingHistoryWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSPROCESSINGHISTORYWIDGET_H
