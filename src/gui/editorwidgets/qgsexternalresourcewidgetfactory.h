/***************************************************************************
   qgsexternalresourcewidgetfactory.h

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCEWIDGETFACTORY_H
#define QGSEXTERNALRESOURCEWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE


/**
 * \ingroup gui
 * \class QgsExternalResourceWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsExternalResourceWidgetFactory : public QgsEditorWidgetFactory
{
  public:

    /**
     * Constructor for QgsExternalResourceWidgetFactory, where \a name is a human-readable
     * name for the factory.
     */
    QgsExternalResourceWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSEXTERNALRESOURCEWIDGETFACTORY_H
