/***************************************************************************
                         qgstemporalcontroller.h
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPORALCONTROLLER_H
#define QGSTEMPORALCONTROLLER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include <QObject>


#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgstemporalnavigationobject.h>
% End
#endif

/**
 * \class QgsTemporalController
 * \ingroup core
 * A controller base class for temporal objects, contains a signal for notifying
 * updates of the objects temporal range.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalController : public QObject
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsTemporalNavigationObject *>( sipCpp ) )
    {
      sipType = sipType_QgsTemporalNavigationObject;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsTemporalController, with the specified \a parent object.
     *
    */
    QgsTemporalController( QObject *parent SIP_TRANSFERTHIS = nullptr );

  signals:

    /**
     * Signals that a temporal \a range has changed and needs to be updated in
     * all connected objects.
     */
    void updateTemporalRange( const QgsDateTimeRange &range );

};

#endif // QGSTEMPORALCONTROLLER_H
