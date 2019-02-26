/***************************************************************************
                             qgsreportsectionlayout.h
                             ---------------------------
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
#ifndef QGSREPORTSECTIONLAYOUT_H
#define QGSREPORTSECTIONLAYOUT_H

#include "qgis_core.h"
#include "qgsabstractreportsection.h"

///@cond NOT_STABLE

// This is not considered stable API - it is exposed to python bindings only for unit testing!

/**
 * \ingroup core
 * \class QgsReportSectionLayout
 * \brief A report section consisting of a single QgsLayout body.
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings for unit testing purposes only.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReportSectionLayout : public QgsAbstractReportSection
{
  public:

    /**
     * Constructor for QgsReportSectionLayout, attached to the specified \a parent section.
     * Note that ownership is not transferred to \a parent.
     */
    QgsReportSectionLayout( QgsAbstractReportSection *parentSection = nullptr );

    QString type() const override { return QStringLiteral( "SectionLayout" ); }
    QString description() const override { return QObject::tr( "Section" ); }
    QIcon icon() const override;

    /**
     * Returns the body layout for the section.
     * \see setBody()
     * \see bodyEnabled()
     * \see setBodyEnabled()
     */
    QgsLayout *body() { return mBody.get(); }

    /**
     * Sets the \a body layout for the section. Ownership of \a body
     * is transferred to the report section.
     * \see body()
     * \see bodyEnabled()
     * \see setBodyEnabled()
     */
    void setBody( QgsLayout *body SIP_TRANSFER ) { mBody.reset( body ); }

    /**
     * Returns TRUE if the body for the section is enabled.
     * \see setBodyEnabled()
     * \see body()
     * \see setBody()
     */
    bool bodyEnabled() const { return mBodyEnabled; }

    /**
     * Sets whether the body for the section is \a enabled.
     * \see bodyEnabled()
     * \see body()
     * \see setBody()
     */
    void setBodyEnabled( bool enabled ) { mBodyEnabled = enabled; }

    QgsReportSectionLayout *clone() const override SIP_FACTORY;
    bool beginRender() override;
    QgsLayout *nextBody( bool &ok ) override;
    void reloadSettings() override;

  protected:

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    bool mExportedBody = false;
    std::unique_ptr< QgsLayout > mBody;
    bool mBodyEnabled = true;
};

///@endcond

#endif //QGSREPORTSECTIONLAYOUT_H
