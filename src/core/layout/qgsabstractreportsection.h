/***************************************************************************
                             qgsabstractreportsection.h
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
#ifndef QGSABSTRACTREPORTSECTION_H
#define QGSABSTRACTREPORTSECTION_H

#include "qgis_core.h"
#include "qgsabstractlayoutiterator.h"
#include "qgslayoutreportcontext.h"

/**
 * \ingroup core
 * \class QgsAbstractReportSection
 * \brief An abstract base class for QgsReport subsections.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstractReportSection : public QgsAbstractLayoutIterator
{

  public:

    //! Constructor for QgsAbstractReportSection
    QgsAbstractReportSection() = default;

    ~QgsAbstractReportSection() override;

    //! QgsAbstractReportSection cannot be copied
    QgsAbstractReportSection( const QgsAbstractReportSection &other ) = delete;

    //! QgsAbstractReportSection cannot be copied
    QgsAbstractReportSection &operator=( const QgsAbstractReportSection &other ) = delete;

    /**
     * Clones the report section. Ownership of the returned section is
     * transferred to the caller.
     *
     * Subclasses should call copyCommonProperties() in their clone()
     * implementations.
     */
    virtual QgsAbstractReportSection *clone() const = 0 SIP_FACTORY;

    // TODO - how to handle this?
    int count() override { return -1; }

    //TODO - baseFilePath should be a filename, not directory
    QString filePath( const QString &baseFilePath, const QString &extension ) override;


#if 0 //TODO
    virtual void setContext( const QgsLayoutReportContext &context ) = 0;
#endif

    QgsLayout *layout() override;
    bool beginRender() override;
    bool next() override;
    bool endRender() override;

    /**
     * Returns true if the header for the section is enabled.
     * \see setHeaderEnabled()
     * \see header()
     * \see setHeader()
     */
    bool headerEnabled() const { return mHeaderEnabled; }

    /**
     * Sets whether the header for the section is \a enabled.
     * \see headerEnabled()
     * \see header()
     * \see setHeader()
     */
    void setHeaderEnabled( bool enabled ) { mHeaderEnabled = enabled; }

    /**
     * Returns the header for the section. Note that the header is only
     * included if headerEnabled() is true.
     * \see setHeaderEnabled()
     * \see headerEnabled()
     * \see setHeader()
     */
    QgsLayout *header() { return mHeader.get(); }

    /**
     * Sets the \a header for the section. Note that the header is only
     * included if headerEnabled() is true. Ownership of \a header
     * is transferred to the report section.
     * \see setHeaderEnabled()
     * \see headerEnabled()
     * \see header()
     */
    void setHeader( QgsLayout *header SIP_TRANSFER ) { mHeader.reset( header ); }

    /**
     * Returns true if the footer for the section is enabled.
     * \see setFooterEnabled()
     * \see footer()
     * \see setFooter()
     */
    bool footerEnabled() const { return mFooterEnabled; }

    /**
     * Sets whether the footer for the section is \a enabled.
     * \see footerEnabled()
     * \see footer()
     * \see setFooter()
     */
    void setFooterEnabled( bool enabled ) { mFooterEnabled = enabled; }

    /**
     * Returns the footer for the section. Note that the footer is only
     * included if footerEnabled() is true.
     * \see setFooterEnabled()
     * \see footerEnabled()
     * \see setFooter()
     */
    QgsLayout *footer() { return mFooter.get(); }

    /**
     * Sets the \a footer for the section. Note that the footer is only
     * included if footerEnabled() is true. Ownership of \a footer
     * is transferred to the report section.
     * \see setFooterEnabled()
     * \see footerEnabled()
     * \see footer()
     */
    void setFooter( QgsLayout *footer SIP_TRANSFER ) { mFooter.reset( footer ); }

    /**
     * Return the number of child sections for this report section. The child
     * sections form the body of the report section.
     * \see children()
     */
    int childCount() const { return mChildren.count(); }

    /**
     * Return all child sections for this report section. The child
     * sections form the body of the report section.
     * \see childCount()
     * \see child()
     * \see appendChild()
     * \see insertChild()
     * \see removeChild()
     */
    QList< QgsAbstractReportSection * > children() { return mChildren; }

    /**
     * Returns the child section at the specified \a index.
     * \see children()
     */
    QgsAbstractReportSection *child( int index );

    /**
     * Adds a child \a section, transferring ownership of the section to this section.
     * \see children()
     * \see insertChild()
     */
    void appendChild( QgsAbstractReportSection *section SIP_TRANSFER );

    /**
     * Inserts a child \a section at the specified \a index, transferring ownership of the section to this section.
     * \see children()
     * \see appendChild()
     */
    void insertChild( int index, QgsAbstractReportSection *section SIP_TRANSFER );

    /**
     * Removes a child \a section, deleting it.
     * \see children()
     */
    void removeChild( QgsAbstractReportSection *section );

    /**
     * Removes the child section at the specified \a index, deleting it.
     * \see children()
     */
    void removeChildAt( int index );

  protected:

    //! Report sub-sections
    enum SubSection
    {
      Header, //!< Header for section
      Body, //!< Body of section
      Footer, //!< Footer for section
      End, //!< End of section (i.e. past all available content)
    };

    /**
     * Copies the common properties of a report section to a \a destination section.
     * This method should be called from clone() implementations.
     */
    void copyCommonProperties( QgsAbstractReportSection *destination ) const;

  private:

    int mSectionNumber = 0;
    SubSection mNextSection = Header;
    int mNextChild = 0;
    QgsLayout *mCurrentLayout = nullptr;

    bool mHeaderEnabled = false;
    bool mFooterEnabled = false;
    std::unique_ptr< QgsLayout > mHeader;
    std::unique_ptr< QgsLayout > mFooter;

    QList< QgsAbstractReportSection * > mChildren;

#ifdef SIP_RUN
    QgsAbstractReportSection( const QgsAbstractReportSection &other );
#endif
};

/**
 * \ingroup core
 * \class QgsReportSectionLayout
 * \brief A report section consisting of a single QgsLayout body.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReportSectionLayout : public QgsAbstractReportSection
{
  public:

    /**
     * Returns the body layout for the section.
     * \see setBody()
     */
    QgsLayout *body() { return mBody.get(); }

    /**
     * Sets the \a body layout for the section. Ownership of \a body
     * is transferred to the report section.
     * \see body()
     */
    void setBody( QgsLayout *body SIP_TRANSFER ) { mBody.reset( body ); }

    QgsReportSectionLayout *clone() const override SIP_FACTORY;
    bool beginRender() override;
    bool next() override;

  private:

    bool mExportedBody = false;
    std::unique_ptr< QgsLayout > mBody;

};


/**
 * \ingroup core
 * \class QgsReport
 * \brief Represents a report for use with the QgsLayout engine.
 *
 * Reports consist of multiple sections, represented by QgsAbstractReportSection
 * subclasses.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReport : public QgsAbstractReportSection
{

  public:

    //! Constructor for QgsReport.
    QgsReport() = default;

    QgsReport *clone() const override SIP_FACTORY;

};

#endif //QGSABSTRACTREPORTSECTION_H
