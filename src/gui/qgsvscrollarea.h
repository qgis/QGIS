/***************************************************************************
                              qgsvscrollarea.h
                              ------------------------
  begin                : September 2017
  copyright            : (C) 2017 Sandro Mani
  email                : manisandro at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVSCROLLAREA_H
#define QGSVSCROLLAREA_H

#include "qgis_gui.h"
#include "qgsscrollarea.h"

/**
 * \ingroup gui
 * QgsVScrollArea is a QScrollArea subclass which only displays a vertical
 * scrollbar and fits the width to the contents.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsVScrollArea : public QgsScrollArea
{
    Q_OBJECT

  public:

    /**
     * QgsVScrollArea
     * \param parent The parent widget
     */
    QgsVScrollArea( QWidget *parent = nullptr );

    bool eventFilter( QObject *o, QEvent *e ) override;
};

#endif // QGSVSCROLLAREA_H
