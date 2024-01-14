//    Copyright (C) 2018-2023 Jakub Melka
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

#include "pdfcatalog.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfnumbertreeloader.h"
#include "pdfnametreeloader.h"
#include "pdfencoding.h"
#include "pdfdbgheap.h"

namespace pdf
{

// Entries for "Info" entry in trailer dictionary
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_TITLE = "Title";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_AUTHOR = "Author";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_SUBJECT = "Subject";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_KEYWORDS = "Keywords";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_CREATOR = "Creator";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_PRODUCER = "Producer";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_CREATION_DATE = "CreationDate";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_MODIFIED_DATE = "ModDate";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_TRAPPED = "Trapped";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_TRAPPED_TRUE = "True";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_TRAPPED_FALSE = "False";
static constexpr const char* PDF_DOCUMENT_INFO_ENTRY_TRAPPED_UNKNOWN = "Unknown";

static constexpr const char* PDF_VIEWER_PREFERENCES_DICTIONARY = "ViewerPreferences";
static constexpr const char* PDF_VIEWER_PREFERENCES_HIDE_TOOLBAR = "HideToolbar";
static constexpr const char* PDF_VIEWER_PREFERENCES_HIDE_MENUBAR = "HideMenubar";
static constexpr const char* PDF_VIEWER_PREFERENCES_HIDE_WINDOW_UI = "HideWindowUI";
static constexpr const char* PDF_VIEWER_PREFERENCES_FIT_WINDOW = "FitWindow";
static constexpr const char* PDF_VIEWER_PREFERENCES_CENTER_WINDOW = "CenterWindow";
static constexpr const char* PDF_VIEWER_PREFERENCES_DISPLAY_DOCUMENT_TITLE = "DisplayDocTitle";
static constexpr const char* PDF_VIEWER_PREFERENCES_NON_FULLSCREEN_PAGE_MODE = "NonFullScreenPageMode";
static constexpr const char* PDF_VIEWER_PREFERENCES_DIRECTION = "Direction";
static constexpr const char* PDF_VIEWER_PREFERENCES_VIEW_AREA = "ViewArea";
static constexpr const char* PDF_VIEWER_PREFERENCES_VIEW_CLIP = "ViewClip";
static constexpr const char* PDF_VIEWER_PREFERENCES_PRINT_AREA = "PrintArea";
static constexpr const char* PDF_VIEWER_PREFERENCES_PRINT_CLIP = "PrintClip";
static constexpr const char* PDF_VIEWER_PREFERENCES_PRINT_SCALING = "PrintScaling";
static constexpr const char* PDF_VIEWER_PREFERENCES_DUPLEX = "Duplex";
static constexpr const char* PDF_VIEWER_PREFERENCES_PICK_TRAY_BY_PDF_SIZE = "PickTrayByPDFSize";
static constexpr const char* PDF_VIEWER_PREFERENCES_NUMBER_OF_COPIES = "NumCopies";
static constexpr const char* PDF_VIEWER_PREFERENCES_PRINT_PAGE_RANGE = "PrintPageRange";

size_t PDFCatalog::getPageIndexFromPageReference(PDFObjectReference reference) const
{
    auto it = std::find_if(m_pages.cbegin(), m_pages.cend(), [reference](const PDFPage& page) { return page.getPageReference() == reference; });
    if (it != m_pages.cend())
    {
        return std::distance(m_pages.cbegin(), it);
    }

    return INVALID_PAGE_INDEX;
}

const PDFDestination* PDFCatalog::getNamedDestination(const QByteArray& key) const
{
    auto it = m_namedDestinations.find(key);
    if (it != m_namedDestinations.cend())
    {
        return &it->second;
    }

    return nullptr;
}

PDFActionPtr PDFCatalog::getNamedJavaScriptAction(const QByteArray& key) const
{
    auto it = m_namedJavaScriptActions.find(key);
    if (it != m_namedJavaScriptActions.cend())
    {
        return it->second;
    }

    return nullptr;
}

PDFObject PDFCatalog::getNamedAppearanceStream(const QByteArray& key) const
{
    auto it = m_namedAppearanceStreams.find(key);
    if (it != m_namedAppearanceStreams.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedPage(const QByteArray& key) const
{
    auto it = m_namedPages.find(key);
    if (it != m_namedPages.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedTemplate(const QByteArray& key) const
{
    auto it = m_namedTemplates.find(key);
    if (it != m_namedTemplates.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedDigitalIdentifier(const QByteArray& key) const
{
    auto it = m_namedDigitalIdentifiers.find(key);
    if (it != m_namedDigitalIdentifiers.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedUrl(const QByteArray& key) const
{
    auto it = m_namedUniformResourceLocators.find(key);
    if (it != m_namedUniformResourceLocators.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedAlternateRepresentation(const QByteArray& key) const
{
    auto it = m_namedAlternateRepresentations.find(key);
    if (it != m_namedAlternateRepresentations.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFObject PDFCatalog::getNamedRendition(const QByteArray& key) const
{
    auto it = m_namedRenditions.find(key);
    if (it != m_namedRenditions.cend())
    {
        return it->second;
    }

    return PDFObject();
}

PDFCatalog PDFCatalog::parse(const PDFObject& catalog, const PDFDocument* document)
{
    if (!catalog.isDictionary())
    {
        throw PDFException(PDFTranslationContext::tr("Catalog must be a dictionary."));
    }

    const PDFDictionary* catalogDictionary = catalog.getDictionary();
    Q_ASSERT(catalogDictionary);

    PDFCatalog catalogObject;
    catalogObject.m_viewerPreferences = PDFViewerPreferences::parse(catalog, document);
    catalogObject.m_pages = PDFPage::parse(&document->getStorage(), catalogDictionary->get("Pages"));
    catalogObject.m_pageLabels = PDFNumberTreeLoader<PDFPageLabel>::parse(&document->getStorage(), catalogDictionary->get("PageLabels"));

    if (catalogDictionary->hasKey("OCProperties"))
    {
        catalogObject.m_optionalContentProperties = PDFOptionalContentProperties::create(document, catalogDictionary->get("OCProperties"));
    }

    if (catalogDictionary->hasKey("Outlines"))
    {
        catalogObject.m_outlineRoot = PDFOutlineItem::parse(&document->getStorage(), catalogDictionary->get("Outlines"));
    }

    if (catalogDictionary->hasKey("OpenAction"))
    {
        PDFObject openAction = document->getObject(catalogDictionary->get("OpenAction"));
        if (openAction.isArray())
        {
            catalogObject.m_openAction.reset(new PDFActionGoTo(PDFDestination::parse(&document->getStorage(), openAction), PDFDestination()));
        }
        if (openAction.isDictionary())
        {
            catalogObject.m_openAction = PDFAction::parse(&document->getStorage(), openAction);
        }
    }

    PDFDocumentDataLoaderDecorator loader(document);
    if (catalogDictionary->hasKey("PageLayout"))
    {
        constexpr const std::array<std::pair<const char*, PageLayout>, 6> pageLayouts = {
            std::pair<const char*, PageLayout>{ "SinglePage", PageLayout::SinglePage },
            std::pair<const char*, PageLayout>{ "OneColumn", PageLayout::OneColumn },
            std::pair<const char*, PageLayout>{ "TwoColumnLeft", PageLayout::TwoColumnLeft },
            std::pair<const char*, PageLayout>{ "TwoColumnRight", PageLayout::TwoColumnRight },
            std::pair<const char*, PageLayout>{ "TwoPageLeft", PageLayout::TwoPagesLeft },
            std::pair<const char*, PageLayout>{ "TwoPageRight", PageLayout::TwoPagesRight }
        };

        catalogObject.m_pageLayout = loader.readEnumByName(catalogDictionary->get("PageLayout"), pageLayouts.begin(), pageLayouts.end(), PageLayout::SinglePage);
    }

    if (catalogDictionary->hasKey("PageMode"))
    {
        constexpr const std::array<std::pair<const char*, PageMode>, 6> pageModes = {
            std::pair<const char*, PageMode>{ "UseNone", PageMode::UseNone },
            std::pair<const char*, PageMode>{ "UseOutlines", PageMode::UseOutlines },
            std::pair<const char*, PageMode>{ "UseThumbs", PageMode::UseThumbnails },
            std::pair<const char*, PageMode>{ "FullScreen", PageMode::Fullscreen },
            std::pair<const char*, PageMode>{ "UseOC", PageMode::UseOptionalContent },
            std::pair<const char*, PageMode>{ "UseAttachments", PageMode::UseAttachments }
        };

        catalogObject.m_pageMode = loader.readEnumByName(catalogDictionary->get("PageMode"), pageModes.begin(), pageModes.end(), PageMode::UseNone);
    }

    if (const PDFDictionary* actionDictionary = document->getDictionaryFromObject(catalogDictionary->get("AA")))
    {
        catalogObject.m_documentActions[WillClose] = PDFAction::parse(&document->getStorage(), actionDictionary->get("WC"));
        catalogObject.m_documentActions[WillSave] = PDFAction::parse(&document->getStorage(), actionDictionary->get("WS"));
        catalogObject.m_documentActions[DidSave] = PDFAction::parse(&document->getStorage(), actionDictionary->get("DS"));
        catalogObject.m_documentActions[WillPrint] = PDFAction::parse(&document->getStorage(), actionDictionary->get("WP"));
        catalogObject.m_documentActions[DidPrint] = PDFAction::parse(&document->getStorage(), actionDictionary->get("DP"));
    }

    catalogObject.m_version = loader.readNameFromDictionary(catalogDictionary, "Version");

    if (const PDFDictionary* namesDictionary = document->getDictionaryFromObject(catalogDictionary->get("Names")))
    {
        auto parseDestination = [](const PDFObjectStorage* storage, PDFObject object)
        {
            object = storage->getObject(object);
            if (object.isDictionary())
            {
                object = object.getDictionary()->get("D");
            }

            return PDFDestination::parse(storage, qMove(object));
        };

        auto getObject = [](const PDFObjectStorage*, PDFObject object)
        {
            return object;
        };

        catalogObject.m_namedDestinations = PDFNameTreeLoader<PDFDestination>::parse(&document->getStorage(), namesDictionary->get("Dests"), parseDestination);
        catalogObject.m_namedAppearanceStreams = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("AP"), getObject);
        catalogObject.m_namedJavaScriptActions = PDFNameTreeLoader<PDFActionPtr>::parse(&document->getStorage(), namesDictionary->get("JavaScript"), &PDFAction::parse);
        catalogObject.m_namedPages = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("Pages"), getObject);
        catalogObject.m_namedTemplates = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("Templates"), getObject);
        catalogObject.m_namedDigitalIdentifiers = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("IDS"), getObject);
        catalogObject.m_namedUniformResourceLocators = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("URLS"), getObject);
        catalogObject.m_namedEmbeddedFiles = PDFNameTreeLoader<PDFFileSpecification>::parse(&document->getStorage(), namesDictionary->get("EmbeddedFiles"), &PDFFileSpecification::parse);
        catalogObject.m_namedAlternateRepresentations = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("AlternatePresentations"), getObject);
        catalogObject.m_namedRenditions = PDFNameTreeLoader<PDFObject>::parse(&document->getStorage(), namesDictionary->get("Renditions"), getObject);
    }

    // Examine "Dests" dictionary
    if (const PDFDictionary* destsDictionary = document->getDictionaryFromObject(catalogDictionary->get("Dests")))
    {
        const size_t count = destsDictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            catalogObject.m_namedDestinations[destsDictionary->getKey(i).getString()] = PDFDestination::parse(&document->getStorage(), destsDictionary->getValue(i));
        }
    }

    // Examine "URI" dictionary
    if (const PDFDictionary* URIDictionary = document->getDictionaryFromObject(catalogDictionary->get("URI")))
    {
        catalogObject.m_baseURI = loader.readStringFromDictionary(URIDictionary, "Base");
    }

    catalogObject.m_formObject = catalogDictionary->get("AcroForm");
    catalogObject.m_extensions = PDFDeveloperExtensions::parse(catalogDictionary->get("Extensions"), document);
    catalogObject.m_documentSecurityStore = PDFDocumentSecurityStore::parse(catalogDictionary->get("DSS"), document);
    catalogObject.m_threads = loader.readObjectList<PDFArticleThread>(catalogDictionary->get("Threads"));
    catalogObject.m_metadata = catalogDictionary->get("Metadata");

    // Examine mark info dictionary
    catalogObject.m_markInfoFlags = MarkInfo_None;
    if (const PDFDictionary* markInfoDictionary = document->getDictionaryFromObject(catalogDictionary->get("MarkInfo")))
    {
        catalogObject.m_markInfoFlags.setFlag(MarkInfo_Marked, loader.readBooleanFromDictionary(markInfoDictionary, "Marked", false));
        catalogObject.m_markInfoFlags.setFlag(MarkInfo_UserProperties, loader.readBooleanFromDictionary(markInfoDictionary, "UserProperties", false));
        catalogObject.m_markInfoFlags.setFlag(MarkInfo_Suspects, loader.readBooleanFromDictionary(markInfoDictionary, "Suspects", false));
    }

    catalogObject.m_structureTreeRoot = catalogDictionary->get("StructTreeRoot");
    catalogObject.m_language = loader.readTextStringFromDictionary(catalogDictionary, "Lang", QString());
    catalogObject.m_webCaptureInfo = PDFWebCaptureInfo::parse(catalogDictionary->get("SpiderInfo"), &document->getStorage());
    catalogObject.m_outputIntents = loader.readObjectList<PDFOutputIntent>(catalogDictionary->get("OutputIntents"));
    catalogObject.m_pieceInfo = catalogDictionary->get("PieceInfo");
    catalogObject.m_perms = catalogDictionary->get("Perms");
    catalogObject.m_legalAttestation = PDFLegalAttestation::parse(&document->getStorage(), catalogDictionary->get("Legal"));
    catalogObject.m_requirements = catalogDictionary->get("Requirements");
    catalogObject.m_collection = catalogDictionary->get("Collection");
    catalogObject.m_xfaNeedsRendering = loader.readBooleanFromDictionary(catalogDictionary, "NeedsRendering", false);
    catalogObject.m_associatedFiles = catalogDictionary->get("AF");
    catalogObject.m_documentPartRoot = catalogDictionary->get("DPartRoot");

    return catalogObject;
}

PDFViewerPreferences PDFViewerPreferences::parse(const PDFObject& catalogDictionary, const PDFDocument* document)
{
    PDFViewerPreferences result;

    if (!catalogDictionary.isDictionary())
    {
        throw PDFException(PDFTranslationContext::tr("Catalog must be a dictionary."));
    }

    const PDFDictionary* dictionary = catalogDictionary.getDictionary();
    if (dictionary->hasKey(PDF_VIEWER_PREFERENCES_DICTIONARY))
    {
        const PDFObject& viewerPreferencesObject = document->getObject(dictionary->get(PDF_VIEWER_PREFERENCES_DICTIONARY));
        if (viewerPreferencesObject.isDictionary())
        {
            // Load the viewer preferences object
            const PDFDictionary* viewerPreferencesDictionary = viewerPreferencesObject.getDictionary();

            auto addFlag = [&result, viewerPreferencesDictionary, document] (const char* name, OptionFlag flag)
            {
                const PDFObject& flagObject = document->getObject(viewerPreferencesDictionary->get(name));
                if (!flagObject.isNull())
                {
                    if (flagObject.isBool())
                    {
                        result.m_optionFlags.setFlag(flag, flagObject.getBool());
                    }
                }
            };
            addFlag(PDF_VIEWER_PREFERENCES_HIDE_TOOLBAR, HideToolbar);
            addFlag(PDF_VIEWER_PREFERENCES_HIDE_MENUBAR, HideMenubar);
            addFlag(PDF_VIEWER_PREFERENCES_HIDE_WINDOW_UI, HideWindowUI);
            addFlag(PDF_VIEWER_PREFERENCES_FIT_WINDOW, FitWindow);
            addFlag(PDF_VIEWER_PREFERENCES_CENTER_WINDOW, CenterWindow);
            addFlag(PDF_VIEWER_PREFERENCES_DISPLAY_DOCUMENT_TITLE, DisplayDocTitle);
            addFlag(PDF_VIEWER_PREFERENCES_PICK_TRAY_BY_PDF_SIZE, PickTrayByPDFSize);

            // Non-fullscreen page mode
            const PDFObject& nonFullscreenPageMode = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_NON_FULLSCREEN_PAGE_MODE));
            if (nonFullscreenPageMode.isName())
            {
                QByteArray enumName = nonFullscreenPageMode.getString();
                if (enumName == "UseNone")
                {
                    result.m_nonFullScreenPageMode = NonFullScreenPageMode::UseNone;
                }
                else if (enumName == "UseOutlines")
                {
                    result.m_nonFullScreenPageMode = NonFullScreenPageMode::UseOutline;
                }
                else if (enumName == "UseThumbs")
                {
                    result.m_nonFullScreenPageMode = NonFullScreenPageMode::UseThumbnails;
                }
                else if (enumName == "UseOC")
                {
                    result.m_nonFullScreenPageMode = NonFullScreenPageMode::UseOptionalContent;
                }
            }

            // Direction
            const PDFObject& direction = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_DIRECTION));
            if (direction.isName())
            {
                QByteArray enumName = direction.getString();
                if (enumName == "L2R")
                {
                    result.m_direction = Direction::LeftToRight;
                }
                else if (enumName == "R2L")
                {
                    result.m_direction = Direction::RightToLeft;
                }
            }

            auto addProperty = [&result, viewerPreferencesDictionary, document] (const char* name, Properties property)
            {
                const PDFObject& propertyObject = document->getObject(viewerPreferencesDictionary->get(name));
                if (!propertyObject.isNull())
                {
                    if (propertyObject.isName())
                    {
                        result.m_properties[property] = propertyObject.getString();
                    }
                }
            };
            addProperty(PDF_VIEWER_PREFERENCES_VIEW_AREA, ViewArea);
            addProperty(PDF_VIEWER_PREFERENCES_VIEW_CLIP, ViewClip);
            addProperty(PDF_VIEWER_PREFERENCES_PRINT_AREA, PrintArea);
            addProperty(PDF_VIEWER_PREFERENCES_PRINT_CLIP, PrintClip);

            // Print scaling
            const PDFObject& printScaling = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_PRINT_SCALING));
            if (printScaling.isName())
            {
                QByteArray enumName = printScaling.getString();
                if (enumName == "None")
                {
                    result.m_printScaling = PrintScaling::None;
                }
                else if (enumName == "AppDefault")
                {
                    result.m_printScaling = PrintScaling::AppDefault;
                }
            }

            // Duplex
            const PDFObject& duplex = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_DUPLEX));
            if (duplex.isName())
            {
                QByteArray enumName = duplex.getString();
                if (enumName == "Simplex")
                {
                    result.m_duplex = Duplex::Simplex;
                }
                else if (enumName == "DuplexFlipShortEdge")
                {
                    result.m_duplex = Duplex::DuplexFlipShortEdge;
                }
                else if (enumName == "DuplexFlipLongEdge")
                {
                    result.m_duplex = Duplex::DuplexFlipLongEdge;
                }
            }

            // Print page range
            const PDFObject& printPageRange = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_PRINT_PAGE_RANGE));
            if (printPageRange.isArray())
            {
                // According to PDF Reference 1.7, this entry is ignored in following cases:
                //  1) Array size is odd
                //  2) Array contains negative numbers
                //
                // But what should we do, if we get 0? Pages in the PDF file are numbered from 1.
                // So if this situation occur, we also ignore the entry.
                const PDFArray* array = duplex.getArray();

                const size_t count = array->getCount();
                if (count % 2 == 0 && count > 0)
                {
                    bool badPageNumber = false;
                    int scanned = 0;
                    PDFInteger start = -1;

                    for (size_t i = 0; i < count; ++i)
                    {
                        const PDFObject& number = document->getObject(array->getItem(i));
                        if (number.isInt())
                        {
                            PDFInteger current = number.getInteger();

                            if (current <= 0)
                            {
                                badPageNumber = true;
                                break;
                            }

                            switch (scanned++)
                            {
                                case 0:
                                {
                                    start = current;
                                    break;
                                }

                                case 1:
                                {
                                    scanned = 0;
                                    result.m_printPageRanges.emplace_back(start, current);
                                    break;
                                }

                                default:
                                    Q_ASSERT(false);
                            }
                        }
                    }

                    // Did we get negative or zero value? If yes, clear the range.
                    if (badPageNumber)
                    {
                        result.m_printPageRanges.clear();
                    }
                }
            }

            // Number of copies
            const PDFObject& numberOfCopies = document->getObject(viewerPreferencesDictionary->get(PDF_VIEWER_PREFERENCES_NUMBER_OF_COPIES));
            if (!numberOfCopies.isNull())
            {
                if (numberOfCopies.isInt())
                {
                    result.m_numberOfCopies = numberOfCopies.getInteger();
                }
            }

            // Enforce
            PDFDocumentDataLoaderDecorator loader(document);
            std::vector<QByteArray> enforce = loader.readNameArrayFromDictionary(viewerPreferencesDictionary, "Enforce");
            result.m_optionFlags.setFlag(EnforcePrintScaling, std::find(enforce.cbegin(), enforce.cend(), "PrintScaling") != enforce.cend());
        }
    }

    return result;
}

PDFPageLabel PDFPageLabel::parse(PDFInteger pageIndex, const PDFObjectStorage* storage, const PDFObject& object)
{
    const PDFObject& dereferencedObject = storage->getObject(object);
    if (dereferencedObject.isDictionary())
    {
        std::array<std::pair<const char*, NumberingStyle>, 5> numberingStyles = { std::pair<const char*, NumberingStyle>{ "D", NumberingStyle::DecimalArabic},
                                                                                  std::pair<const char*, NumberingStyle>{ "R", NumberingStyle::UppercaseRoman },
                                                                                  std::pair<const char*, NumberingStyle>{ "r", NumberingStyle::LowercaseRoman },
                                                                                  std::pair<const char*, NumberingStyle>{ "A", NumberingStyle::UppercaseLetters},
                                                                                  std::pair<const char*, NumberingStyle>{ "a", NumberingStyle::LowercaseLetters} };

        const PDFDictionary* dictionary = dereferencedObject.getDictionary();
        const PDFDocumentDataLoaderDecorator loader(storage);
        const NumberingStyle numberingStyle = loader.readEnumByName(dictionary->get("S"), numberingStyles.cbegin(), numberingStyles.cend(), NumberingStyle::None);
        const QString prefix = loader.readTextString(dictionary->get("P"), QString());
        const PDFInteger startNumber = loader.readInteger(dictionary->get("St"), 1);
        return PDFPageLabel(numberingStyle, prefix, pageIndex, startNumber);
    }

    return PDFPageLabel();
}

const PDFDocumentSecurityStore::SecurityStoreItem* PDFDocumentSecurityStore::getItem(const QByteArray& hash) const
{
    auto it = m_VRI.find(hash);
    if (it != m_VRI.cend())
    {
        return &it->second;
    }

    return getMasterItem();
}

PDFDocumentSecurityStore PDFDocumentSecurityStore::parse(const PDFObject& object, const PDFDocument* document)
{
    PDFDocumentSecurityStore store;

    try
    {
        if (const PDFDictionary* dssDictionary = document->getDictionaryFromObject(object))
        {
            PDFDocumentDataLoaderDecorator loader(document);

            auto getDecodedStreams = [document, &loader](const PDFObject& object) -> std::vector<QByteArray>
            {
                std::vector<QByteArray> result;

                std::vector<PDFObjectReference> references = loader.readReferenceArray(object);
                result.reserve(references.size());
                for (const PDFObjectReference& reference : references)
                {
                    PDFObject objectWithStream = document->getObjectByReference(reference);
                    if (objectWithStream.isStream())
                    {
                        result.emplace_back(document->getDecodedStream(objectWithStream.getStream()));
                    }
                }

                return result;
            };

            store.m_master.Cert = getDecodedStreams(dssDictionary->get("Certs"));
            store.m_master.OCSP = getDecodedStreams(dssDictionary->get("OCSPs"));
            store.m_master.CRL = getDecodedStreams(dssDictionary->get("CRLs"));

            if (const PDFDictionary* vriDictionary = document->getDictionaryFromObject(dssDictionary->get("VRI")))
            {
                for (size_t i = 0, count = vriDictionary->getCount(); i < count; ++i)
                {
                    const PDFObject& vriItemObject = vriDictionary->getValue(i);
                    if (const PDFDictionary* vriItemDictionary = document->getDictionaryFromObject(vriItemObject))
                    {
                        QByteArray key = vriDictionary->getKey(i).getString();

                        SecurityStoreItem& item = store.m_VRI[key];
                        item.Cert = getDecodedStreams(vriItemDictionary->get("Cert"));
                        item.CRL = getDecodedStreams(vriItemDictionary->get("CRL"));
                        item.OCSP = getDecodedStreams(vriItemDictionary->get("OCSP"));
                        item.created = PDFEncoding::convertToDateTime(loader.readStringFromDictionary(vriDictionary, "TU"));

                        PDFObject timestampObject = document->getObject(vriItemDictionary->get("TS"));
                        if (timestampObject.isStream())
                        {
                            item.timestamp = document->getDecodedStream(timestampObject.getStream());
                        }
                    }
                }
            }
        }
    }
    catch (const PDFException&)
    {
        return PDFDocumentSecurityStore();
    }

    return store;
}

PDFDeveloperExtensions PDFDeveloperExtensions::parse(const PDFObject& object, const PDFDocument* document)
{
    PDFDeveloperExtensions extensions;

    if (const PDFDictionary* dictionary = document->getDictionaryFromObject(object))
    {
        const size_t extensionsCount = dictionary->getCount();
        extensions.m_extensions.reserve(extensionsCount);
        for (size_t i = 0; i < extensionsCount; ++i)
        {
            // Skip type entry
            if (dictionary->getKey(i) == "Type")
            {
                continue;
            }

            if (const PDFDictionary* extensionsDictionary = document->getDictionaryFromObject(dictionary->getValue(i)))
            {
                PDFDocumentDataLoaderDecorator loader(document);

                Extension extension;
                extension.name = dictionary->getKey(i).getString();
                extension.baseVersion = loader.readNameFromDictionary(extensionsDictionary, "BaseName");
                extension.extensionLevel = loader.readIntegerFromDictionary(extensionsDictionary, "ExtensionLevel", 0);
                extension.url = loader.readStringFromDictionary(extensionsDictionary, "URL");
                extensions.m_extensions.emplace_back(qMove(extension));
            }
        }
    }

    return extensions;
}

PDFDocumentInfo PDFDocumentInfo::parse(const PDFObject& object, const PDFObjectStorage* storage)
{
    PDFDocumentInfo info;

    if (const PDFDictionary* infoDictionary = storage->getDictionaryFromObject(object))
    {
        auto readTextString = [storage, infoDictionary](const char* entry, QString& fillEntry)
        {
            if (infoDictionary->hasKey(entry))
            {
                const PDFObject& stringObject = storage->getObject(infoDictionary->get(entry));
                if (stringObject.isString())
                {
                    // We have succesfully read the string, convert it according to encoding
                    fillEntry = PDFEncoding::convertTextString(stringObject.getString());
                }
            }
        };
        readTextString(PDF_DOCUMENT_INFO_ENTRY_TITLE, info.title);
        readTextString(PDF_DOCUMENT_INFO_ENTRY_AUTHOR, info.author);
        readTextString(PDF_DOCUMENT_INFO_ENTRY_SUBJECT, info.subject);
        readTextString(PDF_DOCUMENT_INFO_ENTRY_KEYWORDS, info.keywords);
        readTextString(PDF_DOCUMENT_INFO_ENTRY_CREATOR, info.creator);
        readTextString(PDF_DOCUMENT_INFO_ENTRY_PRODUCER, info.producer);

        auto readDate= [storage, infoDictionary](const char* entry, QDateTime& fillEntry)
        {
            if (infoDictionary->hasKey(entry))
            {
                const PDFObject& stringObject = storage->getObject(infoDictionary->get(entry));
                if (stringObject.isString())
                {
                    // We have succesfully read the string, convert it to date time
                    fillEntry = PDFEncoding::convertToDateTime(stringObject.getString());
                }
            }
        };
        readDate(PDF_DOCUMENT_INFO_ENTRY_CREATION_DATE, info.creationDate);
        readDate(PDF_DOCUMENT_INFO_ENTRY_MODIFIED_DATE, info.modifiedDate);

        if (infoDictionary->hasKey(PDF_DOCUMENT_INFO_ENTRY_TRAPPED))
        {
            const PDFObject& nameObject = storage->getObject(infoDictionary->get(PDF_DOCUMENT_INFO_ENTRY_TRAPPED));
            if (nameObject.isName())
            {
                const QByteArray& name = nameObject.getString();
                if (name == PDF_DOCUMENT_INFO_ENTRY_TRAPPED_TRUE)
                {
                    info.trapped = Trapped::True;
                }
                else if (name == PDF_DOCUMENT_INFO_ENTRY_TRAPPED_FALSE)
                {
                    info.trapped = Trapped::False;
                }
                else if (name == PDF_DOCUMENT_INFO_ENTRY_TRAPPED_UNKNOWN)
                {
                    info.trapped = Trapped::Unknown;
                }
            }
            else if (nameObject.isBool())
            {
                info.trapped = nameObject.getBool() ? Trapped::True : Trapped::False;
            }
        }

        // Scan for extra items
        constexpr const char* PREDEFINED_ITEMS[] = { PDF_DOCUMENT_INFO_ENTRY_TITLE, PDF_DOCUMENT_INFO_ENTRY_AUTHOR, PDF_DOCUMENT_INFO_ENTRY_SUBJECT,
                                                     PDF_DOCUMENT_INFO_ENTRY_KEYWORDS, PDF_DOCUMENT_INFO_ENTRY_CREATOR, PDF_DOCUMENT_INFO_ENTRY_PRODUCER,
                                                     PDF_DOCUMENT_INFO_ENTRY_CREATION_DATE, PDF_DOCUMENT_INFO_ENTRY_MODIFIED_DATE, PDF_DOCUMENT_INFO_ENTRY_TRAPPED };
        for (size_t i = 0; i < infoDictionary->getCount(); ++i)
        {
            QByteArray key = infoDictionary->getKey(i).getString();
            if (std::none_of(std::begin(PREDEFINED_ITEMS), std::end(PREDEFINED_ITEMS), [&key](const char* item) { return item == key; }))
            {
                const PDFObject& value = storage->getObject(infoDictionary->getValue(i));
                if (value.isString())
                {
                    const QByteArray& stringValue = value.getString();
                    QDateTime dateTime = PDFEncoding::convertToDateTime(stringValue);
                    if (dateTime.isValid())
                    {
                        info.extra[key] = dateTime;
                    }
                    else
                    {
                        info.extra[key] = PDFEncoding::convertTextString(stringValue);
                    }
                }
            }
        }
    }

    return info;
}

PDFArticleThread PDFArticleThread::parse(const PDFObjectStorage* storage, const PDFObject& object)
{
    PDFArticleThread result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        PDFObjectReference firstBeadReference = loader.readReferenceFromDictionary(dictionary, "F");

        std::set<PDFObjectReference> visitedBeads;
        PDFObjectReference currentBead = firstBeadReference;

        while (!visitedBeads.count(currentBead))
        {
            visitedBeads.insert(currentBead);

            // Read bead
            if (const PDFDictionary* beadDictionary = storage->getDictionaryFromObject(storage->getObjectByReference(currentBead)))
            {
                Bead bead;
                bead.self = currentBead;
                bead.thread = loader.readReferenceFromDictionary(beadDictionary, "T");
                bead.next = loader.readReferenceFromDictionary(beadDictionary, "N");
                bead.previous = loader.readReferenceFromDictionary(beadDictionary, "V");
                bead.page = loader.readReferenceFromDictionary(beadDictionary, "P");
                bead.rect = loader.readRectangle(beadDictionary->get("R"), QRectF());

                currentBead = bead.next;
                result.m_beads.push_back(bead);
            }
            else
            {
                // current bead will be the same, the cycle will break
            }
        }

        result.m_information = PDFDocumentInfo::parse(dictionary->get("I"), storage);
        result.m_metadata = loader.readReferenceFromDictionary(dictionary, "Metadata");
    }

    return result;
}

PDFWebCaptureInfo PDFWebCaptureInfo::parse(const PDFObject& object, const PDFObjectStorage* storage)
{
    PDFWebCaptureInfo result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_version = loader.readNameFromDictionary(dictionary, "V");
        result.m_commands = loader.readReferenceArrayFromDictionary(dictionary, "C");
    }

    return result;
}

PDFOutputIntent PDFOutputIntent::parse(const PDFObjectStorage* storage, const PDFObject& object)
{
    PDFOutputIntent result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_subtype = loader.readNameFromDictionary(dictionary, "S");
        result.m_outputCondition = loader.readTextStringFromDictionary(dictionary, "OutputCondition", QString());
        result.m_outputConditionIdentifier = loader.readTextStringFromDictionary(dictionary, "OutputConditionIdentifier", QString());
        result.m_registryName = loader.readTextStringFromDictionary(dictionary, "RegistryName", QString());
        result.m_info = loader.readTextStringFromDictionary(dictionary, "Info", QString());
        result.m_destOutputProfile = dictionary->get("DestOutputProfile");
        result.m_destOutputProfileRef = PDFOutputIntentICCProfileInfo::parse(dictionary->get("DestOutputProfileRef"), storage);
        result.m_mixingHints = dictionary->get("MixingHints");
        result.m_spectralData = dictionary->get("SpectralData");
    }

    return result;
}

PDFOutputIntentICCProfileInfo PDFOutputIntentICCProfileInfo::parse(const PDFObject& object, const PDFObjectStorage* storage)
{
    PDFOutputIntentICCProfileInfo result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);
        result.m_checkSum = loader.readStringFromDictionary(dictionary, "CheckSum");
        result.m_colorants = loader.readNameArrayFromDictionary(dictionary, "ColorantTable");
        result.m_iccVersion = loader.readStringFromDictionary(dictionary, "ICCVersion");
        result.m_signature = loader.readStringFromDictionary(dictionary, "ProfileCS");
        result.m_profileName = loader.readTextStringFromDictionary(dictionary, "ProfileName", QString());
        result.m_urls = dictionary->get("URLs");
    }

    return result;
}

std::optional<PDFLegalAttestation> PDFLegalAttestation::parse(const PDFObjectStorage* storage, const PDFObject& object)
{
    std::optional<PDFLegalAttestation> result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        result = PDFLegalAttestation();

        result->m_entries[JavaScriptActions] = loader.readIntegerFromDictionary(dictionary, "JavaScriptActions", 0);
        result->m_entries[LaunchActions] = loader.readIntegerFromDictionary(dictionary, "LaunchActions", 0);
        result->m_entries[URIActions] = loader.readIntegerFromDictionary(dictionary, "URIActions", 0);
        result->m_entries[MovieActions] = loader.readIntegerFromDictionary(dictionary, "MovieActions", 0);
        result->m_entries[SoundActions] = loader.readIntegerFromDictionary(dictionary, "SoundActions", 0);
        result->m_entries[HideAnnotationActions] = loader.readIntegerFromDictionary(dictionary, "HideAnnotationActions", 0);
        result->m_entries[GoToRemoteActions] = loader.readIntegerFromDictionary(dictionary, "GoToRemoteActions", 0);
        result->m_entries[AlternateImages] = loader.readIntegerFromDictionary(dictionary, "AlternateImages", 0);
        result->m_entries[ExternalStreams] = loader.readIntegerFromDictionary(dictionary, "ExternalStreams", 0);
        result->m_entries[TrueTypeFonts] = loader.readIntegerFromDictionary(dictionary, "TrueTypeFonts", 0);
        result->m_entries[ExternalRefXobjects] = loader.readIntegerFromDictionary(dictionary, "ExternalRefXobjects", 0);
        result->m_entries[ExternalOPIdicts] = loader.readIntegerFromDictionary(dictionary, "ExternalOPIdicts", 0);
        result->m_entries[NonEmbeddedFonts] = loader.readIntegerFromDictionary(dictionary, "NonEmbeddedFonts", 0);
        result->m_entries[DevDepGS_OP] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_OP", 0);
        result->m_entries[DevDepGS_HT] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_HT", 0);
        result->m_entries[DevDepGS_TR] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_TR", 0);
        result->m_entries[DevDepGS_UCR] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_UCR", 0);
        result->m_entries[DevDepGS_BG] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_BG", 0);
        result->m_entries[DevDepGS_FL] = loader.readIntegerFromDictionary(dictionary, "DevDepGS_FL", 0);
        result->m_hasOptionalContent = loader.readBooleanFromDictionary(dictionary, "OptionalContent", false);
        result->m_attestation = loader.readTextStringFromDictionary(dictionary, "Attestation", QString());
    }

    return result;
}

PDFDocumentRequirements::ValidationResult PDFDocumentRequirements::validate(Requirements supported) const
{
    ValidationResult result;

    QStringList unsatisfiedRequirements;
    for (const RequirementEntry& entry : m_requirements)
    {
        if (entry.requirement == None)
        {
            // Unrecognized entry, just add the penalty
            result.penalty += entry.penalty;
            continue;
        }

        if (!supported.testFlag(entry.requirement))
        {
            result.penalty += entry.penalty;
            unsatisfiedRequirements << getRequirementName(entry.requirement);
        }
    }

    if (!unsatisfiedRequirements.isEmpty())
    {
        result.message = PDFTranslationContext::tr("Required features %1 are unsupported. Document processing can be limited.").arg(unsatisfiedRequirements.join(", "));
    }

    return result;
}

QString PDFDocumentRequirements::getRequirementName(Requirement requirement)
{
    switch (requirement)
    {
       case OCInteract:
            return PDFTranslationContext::tr("Optional Content User Interaction");
       case OCAutoStates:
            return PDFTranslationContext::tr("Optional Content Usage");
       case AcroFormInteract:
            return PDFTranslationContext::tr("Acrobat Forms");
       case Navigation:
            return PDFTranslationContext::tr("Navigation");
       case Markup:
            return PDFTranslationContext::tr("Markup Annotations");
       case _3DMarkup:
            return PDFTranslationContext::tr("Markup of 3D Content");
       case Multimedia:
            return PDFTranslationContext::tr("Multimedia");
       case U3D:
            return PDFTranslationContext::tr("U3D Format of PDF 3D");
       case PRC:
            return PDFTranslationContext::tr("PRC Format of PDF 3D");
       case Action:
            return PDFTranslationContext::tr("Actions");
       case EnableJavaScripts:
            return PDFTranslationContext::tr("JavaScript");
       case Attachment:
            return PDFTranslationContext::tr("Attached Files");
       case AttachmentEditing:
            return PDFTranslationContext::tr("Attached Files Modification");
       case Collection:
            return PDFTranslationContext::tr("Collections of Attached Files");
       case CollectionEditing:
            return PDFTranslationContext::tr("Collections of Attached Files (editation)");
       case DigSigValidation:
            return PDFTranslationContext::tr("Digital Signature Validation");
       case DigSig:
            return PDFTranslationContext::tr("Apply Digital Signature");
       case DigSigMDP:
            return PDFTranslationContext::tr("Digital Signature Validation (with MDP)");
       case RichMedia:
            return PDFTranslationContext::tr("Rich Media");
       case Geospatial2D:
            return PDFTranslationContext::tr("Geospatial 2D Features");
       case Geospatial3D:
            return PDFTranslationContext::tr("Geospatial 3D Features");
       case DPartInteract:
            return PDFTranslationContext::tr("Navigation for Document Parts");
       case SeparationSimulation:
            return PDFTranslationContext::tr("Separation Simulation");
       case Transitions:
            return PDFTranslationContext::tr("Transitions/Presentations");
       case Encryption:
            return PDFTranslationContext::tr("Encryption");

        default:
            Q_ASSERT(false);
            break;
    }

    return QString();
}

PDFDocumentRequirements PDFDocumentRequirements::parse(const PDFObjectStorage* storage, const PDFObject& object)
{
    PDFDocumentRequirements requirements;

    PDFDocumentDataLoaderDecorator loader(storage);
    requirements.m_requirements = loader.readObjectList<RequirementEntry>(object);

    return requirements;
}

PDFDocumentRequirements::RequirementEntry PDFDocumentRequirements::RequirementEntry::parse(const PDFObjectStorage* storage, const PDFObject& object)
{
    RequirementEntry entry;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array requirementTypes = {
            std::pair<const char*, Requirement>{ "OCInteract", OCInteract },
            std::pair<const char*, Requirement>{ "OCAutoStates", OCAutoStates },
            std::pair<const char*, Requirement>{ "AcroFormInteract", AcroFormInteract },
            std::pair<const char*, Requirement>{ "Navigation", Navigation },
            std::pair<const char*, Requirement>{ "Markup", Markup },
            std::pair<const char*, Requirement>{ "3DMarkup", _3DMarkup },
            std::pair<const char*, Requirement>{ "Multimedia", Multimedia },
            std::pair<const char*, Requirement>{ "U3D", U3D },
            std::pair<const char*, Requirement>{ "PRC", PRC },
            std::pair<const char*, Requirement>{ "Action", Action },
            std::pair<const char*, Requirement>{ "EnableJavaScripts", EnableJavaScripts },
            std::pair<const char*, Requirement>{ "Attachment", Attachment },
            std::pair<const char*, Requirement>{ "AttachmentEditing", AttachmentEditing },
            std::pair<const char*, Requirement>{ "Collection", Collection },
            std::pair<const char*, Requirement>{ "CollectionEditing", CollectionEditing },
            std::pair<const char*, Requirement>{ "DigSigValidation", DigSigValidation },
            std::pair<const char*, Requirement>{ "DigSig", DigSig },
            std::pair<const char*, Requirement>{ "DigSigMDP", DigSigMDP },
            std::pair<const char*, Requirement>{ "RichMedia", RichMedia },
            std::pair<const char*, Requirement>{ "Geospatial2D", Geospatial2D },
            std::pair<const char*, Requirement>{ "Geospatial3D", Geospatial3D },
            std::pair<const char*, Requirement>{ "DPartInteract", DPartInteract },
            std::pair<const char*, Requirement>{ "SeparationSimulation", SeparationSimulation },
            std::pair<const char*, Requirement>{ "Transitions", Transitions },
            std::pair<const char*, Requirement>{ "Encryption", Encryption }
        };

        entry.requirement = loader.readEnumByName(dictionary->get("S"), requirementTypes.begin(), requirementTypes.end(), None);
        entry.handler = dictionary->get("RH");
        entry.version = loader.readNameFromDictionary(dictionary, "V");
        entry.penalty = loader.readIntegerFromDictionary(dictionary, "Penalty", 100);
    }

    return entry;
}

PDFPageAdditionalActions PDFPageAdditionalActions::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFPageAdditionalActions result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        result.m_actions[Open] = PDFAction::parse(storage, dictionary->get("O"));
        result.m_actions[Close] = PDFAction::parse(storage, dictionary->get("C"));
    }

    return result;
}

}   // namespace pdf
