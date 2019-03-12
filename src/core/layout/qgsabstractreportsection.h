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
#include "qgsvectorlayerref.h"


///@cond NOT_STABLE

// This is not considered stable API - it is exposed to python bindings only for unit testing!

/**
 * \ingroup core
 * \class QgsReportSectionContext
 * \brief Current context for a report section.
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings for unit testing purposes only.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReportSectionContext
{
  public:

    //! Current feature
    QgsFeature feature;

    //! Current coverage layer
    QgsVectorLayer *currentLayer = nullptr;

    //! Current field filters
    QVariantMap fieldFilters;
};

/**
 * \ingroup core
 * \class QgsAbstractReportSection
 * \brief An abstract base class for QgsReport subsections.
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings for unit testing purposes only.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstractReportSection : public QgsAbstractLayoutIterator
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsReportSectionFieldGroup * >( sipCpp ) )
      sipType = sipType_QgsReportSectionFieldGroup;
    else if ( dynamic_cast< QgsReportSectionLayout * >( sipCpp ) )
      sipType = sipType_QgsReportSectionLayout;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsAbstractReportSection, attached to the specified \a parent section.
     * Note that ownership is not transferred to \a parent.
     */
    QgsAbstractReportSection( QgsAbstractReportSection *parentSection = nullptr );

    ~QgsAbstractReportSection() override;

    //! QgsAbstractReportSection cannot be copied
    QgsAbstractReportSection( const QgsAbstractReportSection &other ) = delete;

    //! QgsAbstractReportSection cannot be copied
    QgsAbstractReportSection &operator=( const QgsAbstractReportSection &other ) = delete;

    /**
     * Returns the section subclass type.
     */
    virtual QString type() const = 0;

    /**
     * Returns a user-visible, translated description of the section.
     */
    virtual QString description() const = 0;

    /**
     * Returns an icon representing the section.
     */
    virtual QIcon icon() const = 0;

    /**
     * Clones the report section. Ownership of the returned section is
     * transferred to the caller.
     *
     * Subclasses should call copyCommonProperties() in their clone()
     * implementations.
     */
    virtual QgsAbstractReportSection *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the parent report section.
     */
    QgsAbstractReportSection *parentSection() { return mParent; }

    /**
     * Returns the associated project.
     */
    QgsProject *project();

    // TODO - how to handle this?
    int count() override { return -1; }

    QString filePath( const QString &baseFilePath, const QString &extension ) override;
    QgsLayout *layout() override;
    bool beginRender() override;
    bool next() override;
    bool endRender() override;

    /**
     * Resets the section, ready for a new iteration.
     */
    virtual void reset();

    /**
     * Called just before rendering the section's header. Should return TRUE if the header
     * is to be included for this section, or FALSE to skip the header for the current
     * section.
     * \see prepareFooter()
     */
    virtual bool prepareHeader();

    /**
     * Called just before rendering the section's footer. Should return TRUE if the footer
     * is to be included for this section, or FALSE to skip the footerfor the current
     * section.
     * \see prepareHeader()
     */
    virtual bool prepareFooter();

    /**
     * Returns the next body layout to export, or NULLPTR if
     * no body layout is required this iteration.
     *
     * \a ok will be set to FALSE if no bodies remain for this section.
     */
    virtual QgsLayout *nextBody( bool &ok SIP_OUT ) { ok = false; return nullptr; }

    /**
     * Returns TRUE if the header for the section is enabled.
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
     * included if headerEnabled() is TRUE.
     * \see setHeaderEnabled()
     * \see headerEnabled()
     * \see setHeader()
     */
    QgsLayout *header() { return mHeader.get(); }

    /**
     * Sets the \a header for the section. Note that the header is only
     * included if headerEnabled() is TRUE. Ownership of \a header
     * is transferred to the report section.
     * \see setHeaderEnabled()
     * \see headerEnabled()
     * \see header()
     */
    void setHeader( QgsLayout *header SIP_TRANSFER );

    /**
     * Returns TRUE if the footer for the section is enabled.
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
     * included if footerEnabled() is TRUE.
     * \see setFooterEnabled()
     * \see footerEnabled()
     * \see setFooter()
     */
    QgsLayout *footer() { return mFooter.get(); }

    /**
     * Sets the \a footer for the section. Note that the footer is only
     * included if footerEnabled() is TRUE. Ownership of \a footer
     * is transferred to the report section.
     * \see setFooterEnabled()
     * \see footerEnabled()
     * \see footer()
     */
    void setFooter( QgsLayout *footer SIP_TRANSFER );

    /**
     * Returns the number of child sections for this report section. The child
     * sections form the body of the report section.
     * \see children()
     */
    int childCount() const { return mChildren.count(); }

    /**
     * Returns the row number of the section within it's parent section.
     */
    int row() const;

    /**
     * Returns all child sections for this report section. The child
     * sections form the body of the report section.
     * \see childCount()
     * \see child()
     * \see appendChild()
     * \see insertChild()
     * \see removeChild()
     */
    QList< QgsAbstractReportSection * > childSections() const { return mChildren; }

    /**
     * Returns the child section at the specified \a index.
     * \see children()
     */
    QgsAbstractReportSection *childSection( int index );

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

    /**
     * Sets the current \a context for this section.
     * \see context()
     */
    void setContext( const QgsReportSectionContext &context );

    /**
     * Returns the current context for this section.
     * \see setContext()
     */
    const QgsReportSectionContext &context() const { return mContext; }

    /**
     * Stores the section state in a DOM element.
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the item state from a DOM element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &sectionElement, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Refreshes the section when global layout related options change.
     */
    virtual void reloadSettings();

  protected:

    //! Report sub-sections
    enum SubSection
    {
      Header, //!< Header for section
      Body, //!< Body of section
      Children, //!< Child sections
      Footer, //!< Footer for section
      End, //!< End of section (i.e. past all available content)
    };

    /**
     * Copies the common properties of a report section to a \a destination section.
     * This method should be called from clone() implementations.
     */
    void copyCommonProperties( QgsAbstractReportSection *destination ) const;

    /**
     * Sets the \a parent report section.
     */
    virtual void setParentSection( QgsAbstractReportSection *parent ) { mParent = parent; }

    /**
     * Stores section state within an XML DOM element.
     * \see writeXml()
     * \see readPropertiesFromElement()
     */
    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets section state from a DOM element.
     * \see writePropertiesToElement()
     * \see readXml()
     */
    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

  private:

    QgsAbstractReportSection *mParent = nullptr;

    int mSectionNumber = 0;
    SubSection mNextSection = Header;
    int mNextChild = 0;
    QgsLayout *mCurrentLayout = nullptr;

    bool mHeaderEnabled = false;
    bool mFooterEnabled = false;
    std::unique_ptr< QgsLayout > mHeader;
    std::unique_ptr< QgsLayout > mFooter;

    QList< QgsAbstractReportSection * > mChildren;

    QgsReportSectionContext mContext;

#ifdef SIP_RUN
    QgsAbstractReportSection( const QgsAbstractReportSection &other );
#endif
};

///@endcond

#endif //QGSABSTRACTREPORTSECTION_H
