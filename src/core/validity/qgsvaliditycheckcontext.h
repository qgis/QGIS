/***************************************************************************
    qgsvaliditycheckcontext.h
    --------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVALIDITYCHECKCONTEXT_H
#define QGSVALIDITYCHECKCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsLayout;

/**
 * \class QgsValidityCheckContext
 * \ingroup core
 * \brief Base class for validity check contexts.
 *
 * QgsAbstractValidityCheck subclasses are passed a QgsValidityCheckContext subclass which
 * encapsulates the context around that particular check type. For instance, a QgsAbstractValidityCheck
 * of the QgsAbstractValidityCheck::TypeLayoutCheck type will be passed a QgsLayoutValidityCheckContext
 * context, containing a reference to the QgsLayout to be checked.
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsValidityCheckContext
{
  public:
    // initially nothing in the base class!

};

/**
 * \class QgsLayoutValidityCheckContext
 * \ingroup core
 * \brief Validity check context for print layout validation.
 *
 * QgsLayoutValidityCheckContext are passed to QgsAbstractValidityCheck subclasses which
 * indicate they are of the QgsAbstractValidityCheck::TypeLayoutCheck type.
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsLayoutValidityCheckContext : public QgsValidityCheckContext
{
  public:

    /**
     * Constructor for QgsLayoutValidityCheckContext for the specified \a layout.
     */
    QgsLayoutValidityCheckContext( QgsLayout *layout )
      : layout( layout )
    {}

    /**
     * Pointer to the layout which the check is being run against.
     */
    QgsLayout *layout = nullptr;

};
#endif // QGSVALIDITYCHECKCONTEXT_H
