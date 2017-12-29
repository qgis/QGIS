/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractreportsection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsAbstractReportSection : QgsAbstractLayoutIterator
{
%Docstring
 An abstract base class for QgsReport subsections.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractreportsection.h"
%End
  public:

    QgsAbstractReportSection();
%Docstring
Constructor for QgsAbstractReportSection
%End

    ~QgsAbstractReportSection();



    virtual QgsAbstractReportSection *clone() const = 0 /Factory/;
%Docstring
Clones the report section. Ownership of the returned section is
transferred to the caller.

Subclasses should call copyCommonProperties() in their clone()
implementations.
%End

    virtual int count();

    virtual QString filePath( const QString &baseFilePath, const QString &extension );




    virtual QgsLayout *layout();

    virtual bool beginRender();

    virtual bool next();

    virtual bool endRender();


    virtual QgsLayout *nextBody();
%Docstring
Returns the next body layout to export, or a None if
no body layouts remain for this section.
%End

    bool headerEnabled() const;
%Docstring
Returns true if the header for the section is enabled.

.. seealso:: :py:func:`setHeaderEnabled()`

.. seealso:: :py:func:`header()`

.. seealso:: :py:func:`setHeader()`
%End

    void setHeaderEnabled( bool enabled );
%Docstring
Sets whether the header for the section is ``enabled``.

.. seealso:: :py:func:`headerEnabled()`

.. seealso:: :py:func:`header()`

.. seealso:: :py:func:`setHeader()`
%End

    QgsLayout *header();
%Docstring
Returns the header for the section. Note that the header is only
included if headerEnabled() is true.

.. seealso:: :py:func:`setHeaderEnabled()`

.. seealso:: :py:func:`headerEnabled()`

.. seealso:: :py:func:`setHeader()`
%End

    void setHeader( QgsLayout *header /Transfer/ );
%Docstring
Sets the ``header`` for the section. Note that the header is only
included if headerEnabled() is true. Ownership of ``header``
is transferred to the report section.

.. seealso:: :py:func:`setHeaderEnabled()`

.. seealso:: :py:func:`headerEnabled()`

.. seealso:: :py:func:`header()`
%End

    bool footerEnabled() const;
%Docstring
Returns true if the footer for the section is enabled.

.. seealso:: :py:func:`setFooterEnabled()`

.. seealso:: :py:func:`footer()`

.. seealso:: :py:func:`setFooter()`
%End

    void setFooterEnabled( bool enabled );
%Docstring
Sets whether the footer for the section is ``enabled``.

.. seealso:: :py:func:`footerEnabled()`

.. seealso:: :py:func:`footer()`

.. seealso:: :py:func:`setFooter()`
%End

    QgsLayout *footer();
%Docstring
Returns the footer for the section. Note that the footer is only
included if footerEnabled() is true.

.. seealso:: :py:func:`setFooterEnabled()`

.. seealso:: :py:func:`footerEnabled()`

.. seealso:: :py:func:`setFooter()`
%End

    void setFooter( QgsLayout *footer /Transfer/ );
%Docstring
Sets the ``footer`` for the section. Note that the footer is only
included if footerEnabled() is true. Ownership of ``footer``
is transferred to the report section.

.. seealso:: :py:func:`setFooterEnabled()`

.. seealso:: :py:func:`footerEnabled()`

.. seealso:: :py:func:`footer()`
%End

    int childCount() const;
%Docstring
Return the number of child sections for this report section. The child
sections form the body of the report section.

.. seealso:: :py:func:`children()`
%End

    QList< QgsAbstractReportSection * > children();
%Docstring
Return all child sections for this report section. The child
sections form the body of the report section.

.. seealso:: :py:func:`childCount()`

.. seealso:: :py:func:`child()`

.. seealso:: :py:func:`appendChild()`

.. seealso:: :py:func:`insertChild()`

.. seealso:: :py:func:`removeChild()`
%End

    QgsAbstractReportSection *child( int index );
%Docstring
Returns the child section at the specified ``index``.

.. seealso:: :py:func:`children()`
%End

    void appendChild( QgsAbstractReportSection *section /Transfer/ );
%Docstring
Adds a child ``section``, transferring ownership of the section to this section.

.. seealso:: :py:func:`children()`

.. seealso:: :py:func:`insertChild()`
%End

    void insertChild( int index, QgsAbstractReportSection *section /Transfer/ );
%Docstring
Inserts a child ``section`` at the specified ``index``, transferring ownership of the section to this section.

.. seealso:: :py:func:`children()`

.. seealso:: :py:func:`appendChild()`
%End

    void removeChild( QgsAbstractReportSection *section );
%Docstring
Removes a child ``section``, deleting it.

.. seealso:: :py:func:`children()`
%End

    void removeChildAt( int index );
%Docstring
Removes the child section at the specified ``index``, deleting it.

.. seealso:: :py:func:`children()`
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

  private:
    QgsAbstractReportSection( const QgsAbstractReportSection &other );
};

class QgsReportSectionLayout : QgsAbstractReportSection
{
%Docstring
 A report section consisting of a single QgsLayout body.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractreportsection.h"
%End
  public:

    QgsLayout *body();
%Docstring
Returns the body layout for the section.

.. seealso:: :py:func:`setBody()`
%End

    void setBody( QgsLayout *body /Transfer/ );
%Docstring
Sets the ``body`` layout for the section. Ownership of ``body``
is transferred to the report section.

.. seealso:: :py:func:`body()`
%End

    virtual QgsReportSectionLayout *clone() const /Factory/;

    virtual bool beginRender();

    virtual QgsLayout *nextBody();


};


class QgsReport : QgsAbstractReportSection
{
%Docstring
 Represents a report for use with the QgsLayout engine.

Reports consist of multiple sections, represented by :py:class:`QgsAbstractReportSection`
subclasses.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractreportsection.h"
%End
  public:

    QgsReport();
%Docstring
Constructor for QgsReport.
%End

    virtual QgsReport *clone() const /Factory/;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsabstractreportsection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
