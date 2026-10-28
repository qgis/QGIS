/***************************************************************************
                             qgsacademicreference.h
                             -----------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSACADEMICREFERENCE_H
#define QGSACADEMICREFERENCE_H

#include "qgis.h"
#include "qgis_core.h"

/**
 * \class QgsAcademicReference
 * \ingroup core
 * \brief Encapsulates an academic reference and formats it according to style guidelines.
 * \since QGIS 4.4
*/
class CORE_EXPORT QgsAcademicReference
{
  public:
    /**
   * Constructor for an invalid QgsAcademicReference.
   */
    QgsAcademicReference() = default;

    /**
   * Returns FALSE if this is a default constructed, invalid reference.
   */
    bool isValid() const { return mType != Qgis::AcademicReferenceType::Unknown; }

    /**
   * Creates a book reference.
   *
   * \param authors list of authors
   * \param year publication year
   * \param title book title
   * \param publisher publisher name
   */
    static QgsAcademicReference createBook( const QStringList &authors, int year, const QString &title, const QString &publisher );

    /**
   * Creates a journal article reference.
   *
   * \param authors list of authors
   * \param year publication year
   * \param title article title
   * \param journal journal name
   * \param volume volume identifier
   * \param issue issue identifier
   * \param pages page range or number
   */
    static QgsAcademicReference createJournalArticle(
      const QStringList &authors, int year, const QString &title, const QString &journal, const QString &volume = QString(), const QString &issue = QString(), const QString &pages = QString()
    );

    /**
   * Creates a conference paper or presentation reference.
   *
   * \param authors list of authors
   * \param year presentation year
   * \param title paper title
   * \param meeting conference or meeting title
   * \param publisher publisher name
   * \param pages page range or number
   */
    static QgsAcademicReference createPresentation(
      const QStringList &authors, int year, const QString &title, const QString &meeting, const QString &publisher = QString(), const QString &pages = QString()
    );

    /**
   * Creates a web page or online resource reference.
   *
   * \param authors list of authors
   * \param year publication or access year
   * \param title web page title
   * \param url target address
   */
    static QgsAcademicReference createWebPage( const QStringList &authors, int year, const QString &title, const QString &url );

    /**
   * Creates a preprint reference.
   *
   * \param authors list of authors
   * \param year publication year
   * \param title paper title
   * \param repository repository or archive name
   * \param url repository target address
   */
    static QgsAcademicReference createPreprint( const QStringList &authors, int year, const QString &title, const QString &repository, const QString &url );

    /**
   * Returns the reference type.
   *
   * \see setType()
   */
    Qgis::AcademicReferenceType type() const;

    /**
   * Sets the reference \a type.
   *
   * \see type()
   */
    void setType( Qgis::AcademicReferenceType type );

    /**
   * Returns the authors of the reference.
   *
   * \see setAuthors()
   */
    QStringList authors() const;

    /**
   * Sets the \a authors of the reference.
   *
   * Individual authors should be formatted using surname first, followed by a comma and the author's initials. E.g. "Smith, J. P.".
   *
   * \see authors()
   */
    void setAuthors( const QStringList &authors );

    /**
   * Returns the publication year.
   *
   * \see setYear()
   */
    int year() const;

    /**
   * Sets the publication \a year.
   *
   * \see year()
   */
    void setYear( int year );

    /**
   * Returns the title of the work.
   *
   * \see setTitle()
   */
    QString title() const;

    /**
   * Sets the \a title of the work.
   *
   * \see title()
   */
    void setTitle( const QString &title );

    /**
   * Returns the journal title.
   *
   * \see setJournal()
   */
    QString journal() const;

    /**
   * Sets the \a journal title.
   *
   * \see journal()
   */
    void setJournal( const QString &journal );

    /**
   * Returns the volume identifier.
   *
   * \see setVolume()
   */
    QString volume() const;

    /**
   * Sets the \a volume identifier.
   *
   * \see volume()
   */
    void setVolume( const QString &volume );

    /**
   * Returns the issue number.
   *
   * \see setIssue()
   */
    QString issue() const;

    /**
   * Sets the \a issue number.
   *
   * \see issue()
   */
    void setIssue( const QString &issue );

    /**
   * Returns the page numbers.
   *
   * \see setPages()
   */
    QString pages() const;

    /**
   * Sets the page numbers.
   *
   * \see pages()
   */
    void setPages( const QString &pages );

    /**
   * Returns the publisher name.
   *
   * \see setPublisher()
   */
    QString publisher() const;

    /**
   * Sets the \a publisher name.
   *
   * \see publisher()
   */
    void setPublisher( const QString &publisher );

    /**
   * Returns the URL.
   *
   * \see setUrl()
   */
    QString url() const;

    /**
   * Sets the \a url.
   *
   * \see url()
   */
    void setUrl( const QString &url );

    /**
   * Returns a plain text representation of the reference (in APA style).
   */
    QString asPlainText() const;

    /**
   * Returns a HTML formatted representation of the reference (in APA style).
   */
    QString asHtml() const;

  private:
    QString formatted( bool asHtml ) const;

    Qgis::AcademicReferenceType mType = Qgis::AcademicReferenceType::Unknown;
    QStringList mAuthors;
    int mYear = 0;
    QString mTitle;
    QString mJournal;
    QString mVolume;
    QString mIssue;
    QString mPages;
    QString mPublisher;
    QString mUrl;
};

#endif //QGSACADEMICREFERENCE_H
