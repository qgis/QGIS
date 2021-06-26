/***************************************************************************
    qgsdevtoolwidget.h
    ------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDEVTOOLWIDGET_H
#define QGSDEVTOOLWIDGET_H

#include "qgspanelwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsDevToolWidget
 * \brief A panel widget that can be shown in the developer tools panel.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDevToolWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDevToolWidget, with the specified \a parent widget.
     */
    QgsDevToolWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

};

#endif // QGSDEVTOOLWIDGET_H
