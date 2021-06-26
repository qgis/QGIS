/***************************************************************************
    qgslayoutqptdrophandler.h
    -------------------------
    begin                : December 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTQPTDROPHANDLER_H
#define QGSLAYOUTQPTDROPHANDLER_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgslayoutcustomdrophandler.h"

/**
 * \ingroup gui
 * Layout drop handler for handling QPT files
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutQptDropHandler : public QgsLayoutCustomDropHandler
{
    Q_OBJECT

  public:

    //! constructor
    QgsLayoutQptDropHandler( QObject *parent = nullptr );

    bool handleFileDrop( QgsLayoutDesignerInterface *iface, QPointF point, const QString &file ) override;
};

#endif // QGSLAYOUTQPTDROPHANDLER_H
