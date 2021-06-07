/***************************************************************************
    qgsdatetimeeditfactory.h
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEEDITFACTORY_H
#define QGSDATETIMEEDITFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsDateTimeEditFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsDateTimeEditFactory : public QgsEditorWidgetFactory
{
  public:

    /**
     * Constructor for QgsDateTimeEditFactory, where \a name is a human-readable
     * name for the factory.
     */
    QgsDateTimeEditFactory( const QString &name );

    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    QgsSearchWidgetWrapper *createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QHash<const char *, int> supportedWidgetTypes() override;
    unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const override;
};

#endif // QGSDATETIMEEDITFACTORY_H
