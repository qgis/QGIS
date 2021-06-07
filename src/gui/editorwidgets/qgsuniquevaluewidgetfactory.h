/***************************************************************************
    qgsuniquevaluewidgetfactory.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUNIQUEVALUEWIDGETFACTORY_H
#define QGSUNIQUEVALUEWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsUniqueValueWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsUniqueValueWidgetFactory : public QgsEditorWidgetFactory
{
  public:

    /**
     * Constructor for QgsUniqueValueWidgetFactory, where \a name is a human-readable
     * name for the factory.
     */
    QgsUniqueValueWidgetFactory( const QString &name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
};

#endif // QGSUNIQUEVALUEWIDGETFACTORY_H
