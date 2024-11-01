/***************************************************************************
                             qgslayoutinterface.h
                             --------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTINTERFACE_H
#define QGSLAYOUTINTERFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>
#include <QIcon>
#include <QDomElement>

class QgsProject;
class QgsReadWriteContext;
class QgsStyleEntityVisitorInterface;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsprintlayout.h"
#include "qgsreport.h"
  % End
#endif

  /**
 * \ingroup core
 * \class QgsMasterLayoutInterface
 * \brief Interface for master layout type objects, such as print layouts and reports.
 */
  class CORE_EXPORT QgsMasterLayoutInterface
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->layoutType() )
    {
      case QgsMasterLayoutInterface::PrintLayout:
        sipType = sipType_QgsPrintLayout;
        *sipCppRet = static_cast<QgsPrintLayout *>( sipCpp );
        break;
      case QgsMasterLayoutInterface::Report:
        *sipCppRet = static_cast<QgsReport *>( sipCpp );
        sipType = sipType_QgsReport;
        break;
      default:
        sipType = NULL;
    }
    SIP_END
#endif

  public:
    //! Master layout type
    enum Type
    {
      PrintLayout = 0, //!< Individual print layout (QgsPrintLayout)
      Report = 1,      //!< Report (QgsReport)
    };

    virtual ~QgsMasterLayoutInterface() = default;

    /**
     * Creates a clone of the layout. Ownership of the returned layout
     * is transferred to the caller.
     */
    virtual QgsMasterLayoutInterface *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the master layout type.
     */
    virtual QgsMasterLayoutInterface::Type layoutType() const = 0;

    /**
     * Returns the layout's name.
     * \see setName()
     */
    virtual QString name() const = 0;

    /**
     * Returns an icon for the layout.
     */
    virtual QIcon icon() const = 0;

    /**
     * Sets the layout's name.
     * \see name()
     */
    virtual void setName( const QString &name ) = 0;

    /**
     * The project associated with the layout. Used to get access to layers, map themes,
     * relations and various other bits. It is never NULLPTR.
     */
    virtual QgsProject *layoutProject() const = 0;

    /**
     * Returns the layout's state encapsulated in a DOM element.
     * \see readLayoutXml()
     */
    virtual QDomElement writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    /**
     * Sets the layout's state from a DOM element. \a layoutElement is the DOM node corresponding to the layout.
     * \see writeLayoutXml()
     */
    virtual bool readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context ) = 0;

    /**
     * Refreshes the layout when global layout related options change.
     */
    virtual void updateSettings() = 0;

    /**
     * Accepts the specified style entity \a visitor, causing it to visit all style entities associated
     * with the layout.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    virtual bool layoutAccept( QgsStyleEntityVisitorInterface *visitor ) const
    {
      Q_UNUSED( visitor );
      return true;
    }
};

#endif //QGSLAYOUTINTERFACE_H
