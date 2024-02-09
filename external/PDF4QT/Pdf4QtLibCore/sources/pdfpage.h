//    Copyright (C) 2018-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFPAGE_H
#define PDFPAGE_H

#include "pdfobject.h"

#include <QRectF>
#include <QDateTime>

#include <set>
#include <optional>

namespace pdf
{
class PDFDocument;
class PDFObjectStorage;

/// This enum represents number of degree, which should be page rotated CLOCKWISE,
/// when being displayed or printed.
enum class PageRotation
{
    None,
    Rotate90,
    Rotate180,
    Rotate270
};

enum class PageTabOrder
{
    Invalid,
    Row,
    Column,
    Structure,
    Array,
    Widget
};

constexpr PageRotation getPageRotationRotatedRight(PageRotation rotation)
{
    switch (rotation)
    {
        case PageRotation::None:
            return PageRotation::Rotate90;
        case PageRotation::Rotate90:
            return PageRotation::Rotate180;
        case PageRotation::Rotate180:
            return PageRotation::Rotate270;
        case PageRotation::Rotate270:
            return PageRotation::None;
    }

    return PageRotation::None;
}

constexpr PageRotation getPageRotationRotatedLeft(PageRotation rotation)
{
    switch (rotation)
    {
        case PageRotation::None:
            return PageRotation::Rotate270;
        case PageRotation::Rotate90:
            return PageRotation::None;
        case PageRotation::Rotate180:
            return PageRotation::Rotate90;
        case PageRotation::Rotate270:
            return PageRotation::Rotate180;
    }

    return PageRotation::None;
}

constexpr PageRotation getPageRotationCombined(PageRotation r1, PageRotation r2)
{
    while (r1 != PageRotation::None)
    {
        r2 = getPageRotationRotatedRight(r2);
        r1 = getPageRotationRotatedLeft(r1);
    }

    return r2;
}

constexpr PageRotation getPageRotationInversed(PageRotation rotation)
{
    switch (rotation)
    {
        case PageRotation::None:
            return PageRotation::None;
        case PageRotation::Rotate90:
            return PageRotation::Rotate270;
        case PageRotation::Rotate180:
            return PageRotation::Rotate180;
        case PageRotation::Rotate270:
            return PageRotation::Rotate90;
    }

    return PageRotation::None;
}

/// This class represents attributes, which are inheritable. Also allows merging from
/// parents.
class PDFPageInheritableAttributes
{
public:
    explicit inline PDFPageInheritableAttributes() = default;

    /// Parses inheritable attributes from the page tree node
    /// \param templateAttributes Template attributes
    /// \param dictionary Dictionary, from which the data will be read
    /// \param storage Storage owning this data
    static PDFPageInheritableAttributes parse(const PDFPageInheritableAttributes& templateAttributes, const PDFObject& dictionary, const PDFObjectStorage* storage);

    const QRectF& getMediaBox() const { return m_mediaBox; }
    const QRectF& getCropBox() const { return m_cropBox; }
    PageRotation getPageRotation() const;
    const PDFObject& getResources() const { return m_resources; }

private:
    QRectF m_mediaBox;
    QRectF m_cropBox;
    std::optional<PageRotation> m_pageRotation;
    PDFObject m_resources;
};

/// Object representing page in PDF document. Contains different page properties, such as
/// media box, crop box, rotation, etc. and also page content, resources.
class PDF4QTLIBCORESHARED_EXPORT PDFPage
{
public:
    explicit PDFPage() = default;

    /// Parses the page tree. If error occurs, then exception is thrown.
    /// \param storage Storage owning this tree
    /// \param root Root object of page tree
    static std::vector<PDFPage> parse(const PDFObjectStorage* storage, const PDFObject& root);

    inline const QRectF& getMediaBox() const { return m_mediaBox; }
    inline const QRectF& getCropBox() const { return m_cropBox; }
    inline const QRectF& getBleedBox() const { return m_bleedBox; }
    inline const QRectF& getTrimBox() const { return m_trimBox; }
    inline const QRectF& getArtBox() const { return m_artBox; }
    inline PageRotation getPageRotation() const { return m_pageRotation; }

    inline const PDFObject& getResources() const { return m_resources; }
    inline const PDFObject& getContents() const { return m_contents; }

    QRectF getRectMM(const QRectF& rect) const;

    inline QRectF getMediaBoxMM() const { return getRectMM(m_mediaBox); }
    inline QRectF getCropBoxMM() const { return getRectMM(m_cropBox); }
    inline QRectF getBleedBoxMM() const { return getRectMM(m_bleedBox); }
    inline QRectF getTrimBoxMM() const { return getRectMM(m_trimBox); }
    inline QRectF getArtBoxMM() const { return getRectMM(m_artBox); }

    inline PDFObjectReference getPageReference() const { return m_pageReference; }
    inline PDFObjectReference getThumbnailReference() const { return m_thumbnailReference; }
    inline PDFObjectReference getDocumentPart() const { return m_documentPart; }

    QRectF getRotatedMediaBox() const;
    QRectF getRotatedMediaBoxMM() const;
    QRectF getRotatedCropBox() const;

    inline const std::vector<PDFObjectReference>& getAnnotations() const { return m_annots; }
    inline const std::vector<PDFObjectReference>& getBeads() const { return m_beads; }
    inline const QDateTime& getLastModifiedDateTime() const { return m_lastModified; }

    /// Returns box color info dictionary, if it is present. This dictionary
    /// describes appearance of page boundaries. Empty object can be returned,
    /// if dictionary doesn't exist.
    /// \param storage Storage
    PDFObject getBoxColorInfo(const PDFObjectStorage* storage) const;

    /// Returns page transparency group (attributes for the
    /// transparent imaging model). Empty object can be returned,
    /// if dictionary doesn't exist.
    /// \param storage Storage
    PDFObject getTransparencyGroup(const PDFObjectStorage* storage) const;

    /// Returns page thumbnail. Empty object can be returned,
    /// if thumbnail doesn't exist.
    /// \param storage Storage
    PDFObject getThumbnail(const PDFObjectStorage* storage) const;

    /// Returns page transition. Page transition object defines,
    /// what should be done during presentations, how pages are switched etc.
    /// Empty object can be returned, if thumbnail doesn't exist.
    /// \param storage Storage
    PDFObject getTransition(const PDFObjectStorage* storage) const;

    /// Returns page additional actions dictionary. If no additional
    /// actions are defined, then empty object is returned.
    /// \param storage Storage
    PDFObject getAdditionalActions(const PDFObjectStorage* storage) const;

    /// Returns page metadata stream. If no metadata stream is defined,
    /// then empty object is returned.
    /// \param storage Storage
    PDFObject getMetadata(const PDFObjectStorage* storage) const;

    /// Returns page piece dictionary associated with the page.
    /// Empty object can be returned, if no piece dictionary is found.
    /// \param storage Storage
    PDFObject getPieceDictionary(const PDFObjectStorage* storage) const;

    /// Returns color separation info. This information is required
    /// to generate color separations for the page.
    /// \param storage Storage
    PDFObject getColorSeparationInfo(const PDFObjectStorage* storage) const;

    /// Returns first navigation node on the page, or null object,
    /// if no navigation nodes are present.
    /// \param storage Storage
    PDFObject getFirstSubpageNavigationNode(const PDFObjectStorage* storage) const;

    /// Returns array of viewport dictionaries, that shall specify
    /// rectangular regions on the page.
    /// \param storage Storage
    PDFObject getViewports(const PDFObjectStorage* storage) const;

    /// Returns array of associated files.
    /// \param storage Storage
    PDFObject getAssociatedFiles(const PDFObjectStorage* storage) const;

    /// Returns array of output intents. Output intents define color
    /// characteristics of output devices on which this page
    /// will be rendered.
    /// \param storage Storage
    PDFObject getOutputIntents(const PDFObjectStorage* storage) const;

    /// Returns page transition time. During presentations, this specifies a time window,
    /// in which is page displayed, until it is advanced to the next page. Time
    /// is specified in seconds.
    inline PDFInteger getDuration() const { return m_duration; }

    /// Returns integer key of structure parent of the page, in the structure tree.
    /// If no structure tree exists, or page doesn't define it, then zero is returned.
    inline PDFInteger getStructureParentKey() const { return m_structParent; }

    /// Returns web capture content set id.
    inline const QByteArray& getWebCaptureContentSetId() const { return m_webCaptureContentSetId; }

    /// Returns preferred zoom. If zero is returned, no preferred zoom is returned.
    inline PDFReal getPreferredZoom() const { return m_preferredZoom; }

    /// Returns page tab order (if it is defined). Page tab order defines
    /// sequence of tab stops for annotations. If no tab order is defined,
    /// then Invalid is returned.
    inline PageTabOrder getTabOrder() const { return m_pageTabOrder; }

    /// Returns name of template, from which this page was generated
    inline const QByteArray& getTemplateName() const { return m_templateName; }

    /// Returns page user space unit. User space units are multiplies
    /// of 1 / 72 inch. Default value is 1.0.
    inline PDFReal getUserUnit() const { return m_userUnit; }

    static QSizeF getRotatedSize(const QSizeF& size, PageRotation rotation);
    static QRectF getRotatedBox(const QRectF& rect, PageRotation rotation);

private:
    /// Parses the page tree (implementation). If error occurs, then exception is thrown.
    /// \param pages Page array. Pages are inserted into this array
    /// \param visitedReferences Visited references (to check cycles in page tree and avoid hangup)
    /// \param templateAttributes Template attributes (inheritable attributes defined in parent)
    /// \param root Root object of page tree
    /// \param storage Storage owning this tree
    static void parseImpl(std::vector<PDFPage>& pages,
                          std::set<PDFObjectReference>& visitedReferences,
                          const PDFPageInheritableAttributes& templateAttributes,
                          const PDFObject& root,
                          const PDFObjectStorage* storage);

    /// Returns object from page dictionary. This function requires,
    /// that storage of object is present, for object fetching. Objects
    /// are not stored in this class, because it will have too large
    /// memory requirements.
    /// \param storage Storage
    /// \param key Page dictionary key
    PDFObject getObjectFromPageDictionary(const PDFObjectStorage* storage, const char* key) const;

    PDFObject m_pageObject;
    QRectF m_mediaBox;
    QRectF m_cropBox;
    QRectF m_bleedBox;
    QRectF m_trimBox;
    QRectF m_artBox;
    PageRotation m_pageRotation = PageRotation::None;
    PDFObject m_resources;
    PDFObject m_contents;
    PDFObjectReference m_pageReference;
    PDFObjectReference m_thumbnailReference;
    PDFObjectReference m_documentPart;
    std::vector<PDFObjectReference> m_annots;
    std::vector<PDFObjectReference> m_beads;
    QDateTime m_lastModified;
    PDFInteger m_duration = 0;
    PDFInteger m_structParent = 0;
    PDFReal m_preferredZoom = 0.0;
    PDFReal m_userUnit = 1.0;
    PageTabOrder m_pageTabOrder = PageTabOrder::Invalid;
    QByteArray m_webCaptureContentSetId;
    QByteArray m_templateName;
};

}   // namespace pdf

#endif // PDFPAGE_H
