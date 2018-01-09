/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractreportsection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsReportSectionContext
{
%Docstring
 Current context for a report section.

.. warning::

   This is not considered stable API, and may change in future QGIS releases. It is
exposed to the Python bindings for unit testing purposes only.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractreportsection.h"
%End
  public:

    QgsFeature feature;

    QgsVectorLayer *currentLayer;

};

class QgsAbstractReportSection : QgsAbstractLayoutIterator
{
%Docstring
 An abstract base class for QgsReport subsections.

.. warning::

   This is not considered stable API, and may change in future QGIS releases. It is
exposed to the Python bindings for unit testing purposes only.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractreportsection.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast< QgsReportSectionFieldGroup * >( sipCpp ) )
      sipType = sipType_QgsReportSectionFieldGroup;
    else if ( dynamic_cast< QgsReportSectionLayout * >( sipCpp ) )
      sipType = sipType_QgsReportSectionLayout;
    else
      sipType = NULL;
%End
  public:

    QgsAbstractReportSection( QgsAbstractReportSection *parentSection = 0 );
%Docstring
Constructor for QgsAbstractReportSection, attached to the specified ``parent`` section.
Note that ownership is not transferred to ``parent``.
%End

    ~QgsAbstractReportSection();



    virtual QString type() const = 0;
%Docstring
Returns the section subclass type.
%End

    virtual QString description() const = 0;
%Docstring
Returns a user-visible, translated description of the section.
%End

    virtual QIcon icon() const = 0;
%Docstring
Returns an icon representing the section.
%End

    virtual QgsAbstractReportSection *clone() const = 0 /Factory/;
%Docstring
Clones the report section. Ownership of the returned section is
transferred to the caller.

Subclasses should call copyCommonProperties() in their clone()
implementations.
%End

    QgsAbstractReportSection *parentSection();
%Docstring
Returns the parent report section.
%End

    QgsProject *project();
%Docstring
Returns the associated project.
%End

    virtual int count();

    virtual QString filePath( const QString &baseFilePath, const QString &extension );

    virtual QgsLayout *layout();

    virtual bool beginRender();

    virtual bool next();

    virtual bool endRender();


    virtual void reset();
%Docstring
Resets the section, ready for a new iteration.
%End

    virtual void prepareHeader();
%Docstring
Called just before rendering the section's header.

.. seealso:: :py:func:`prepareFooter`
%End

    virtual void prepareFooter();
%Docstring
Called just before rendering the section's footer.

.. seealso:: :py:func:`prepareHeader`
%End

    virtual QgsLayout *nextBody( bool &ok /Out/ );
%Docstring
Returns the next body layout to export, or a None if
no body layout is required this iteration.

``ok`` will be set to false if no bodies remain for this section.
%End

    bool headerEnabled() const;
%Docstring
Returns true if the header for the section is enabled.

.. seealso:: :py:func:`setHeaderEnabled`

.. seealso:: :py:func:`header`

.. seealso:: :py:func:`setHeader`
%End

    void setHeaderEnabled( bool enabled );
%Docstring
Sets whether the header for the section is ``enabled``.

.. seealso:: :py:func:`headerEnabled`

.. seealso:: :py:func:`header`

.. seealso:: :py:func:`setHeader`
%End

    QgsLayout *header();
%Docstring
Returns the header for the section. Note that the header is only
included if headerEnabled() is true.

.. seealso:: :py:func:`setHeaderEnabled`

.. seealso:: :py:func:`headerEnabled`

.. seealso:: :py:func:`setHeader`
%End

    void setHeader( QgsLayout *header /Transfer/ );
%Docstring
Sets the ``header`` for the section. Note that the header is only
included if headerEnabled() is true. Ownership of ``header``
is transferred to the report section.

.. seealso:: :py:func:`setHeaderEnabled`

.. seealso:: :py:func:`headerEnabled`

.. seealso:: :py:func:`header`
%End

    bool footerEnabled() const;
%Docstring
Returns true if the footer for the section is enabled.

.. seealso:: :py:func:`setFooterEnabled`

.. seealso:: :py:func:`footer`

.. seealso:: :py:func:`setFooter`
%End

    void setFooterEnabled( bool enabled );
%Docstring
Sets whether the footer for the section is ``enabled``.

.. seealso:: :py:func:`footerEnabled`

.. seealso:: :py:func:`footer`

.. seealso:: :py:func:`setFooter`
%End

    QgsLayout *footer();
%Docstring
Returns the footer for the section. Note that the footer is only
included if footerEnabled() is true.

.. seealso:: :py:func:`setFooterEnabled`

.. seealso:: :py:func:`footerEnabled`

.. seealso:: :py:func:`setFooter`
%End

    void setFooter( QgsLayout *footer /Transfer/ );
%Docstring
Sets the ``footer`` for the section. Note that the footer is only
included if footerEnabled() is true. Ownership of ``footer``
is transferred to the report section.

.. seealso:: :py:func:`setFooterEnabled`

.. seealso:: :py:func:`footerEnabled`

.. seealso:: :py:func:`footer`
%End

    int childCount() const;
%Docstring
Return the number of child sections for this report section. The child
sections form the body of the report section.

.. seealso:: :py:func:`children`
%End

    int row() const;
%Docstring
Returns the row number of the section within it's parent section.
%End

    QList< QgsAbstractReportSection * > childSections() const;
%Docstring
Return all child sections for this report section. The child
sections form the body of the report section.

.. seealso:: :py:func:`childCount`

.. seealso:: :py:func:`child`

.. seealso:: :py:func:`appendChild`

.. seealso:: :py:func:`insertChild`

.. seealso:: :py:func:`removeChild`
%End

    QgsAbstractReportSection *childSection( int index );
%Docstring
Returns the child section at the specified ``index``.

.. seealso:: :py:func:`children`
%End

    void appendChild( QgsAbstractReportSection *section /Transfer/ );
%Docstring
Adds a child ``section``, transferring ownership of the section to this section.

.. seealso:: :py:func:`children`

.. seealso:: :py:func:`insertChild`
%End

    void insertChild( int index, QgsAbstractReportSection *section /Transfer/ );
%Docstring
Inserts a child ``section`` at the specified ``index``, transferring ownership of the section to this section.

.. seealso:: :py:func:`children`

.. seealso:: :py:func:`appendChild`
%End

    void removeChild( QgsAbstractReportSection *section );
%Docstring
Removes a child ``section``, deleting it.

.. seealso:: :py:func:`children`
%End

    void removeChildAt( int index );
%Docstring
Removes the child section at the specified ``index``, deleting it.

.. seealso:: :py:func:`children`
%End

    void setContext( const QgsReportSectionContext &context );
%Docstring
Sets the current ``context`` for this section.

.. seealso:: :py:func:`context`
%End

    const QgsReportSectionContext &context() const;
%Docstring
Returns the current context for this section.

.. seealso:: :py:func:`setContext`
%End

    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores the section state in a DOM element.

.. seealso:: :py:func:`readXml`
%End

    bool readXml( const QDomElement &sectionElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets the item state from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

  protected:

    enum SubSection
    {
      Header,
      Body,
      Children,
      Footer,
      End,
    };

    void copyCommonProperties( QgsAbstractReportSection *destination ) const;
%Docstring
Copies the common properties of a report section to a ``destination`` section.
This method should be called from clone() implementations.
%End

    virtual void setParentSection( QgsAbstractReportSection *parent );
%Docstring
Sets the ``parent`` report section.
%End

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores section state within an XML DOM element.

.. seealso:: :py:func:`writeXml`

.. seealso:: :py:func:`readPropertiesFromElement`
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets section state from a DOM element.

.. seealso:: :py:func:`writePropertiesToElement`

.. seealso:: :py:func:`readXml`
%End

  private:
    QgsAbstractReportSection( const QgsAbstractReportSection &other );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractreportsection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
