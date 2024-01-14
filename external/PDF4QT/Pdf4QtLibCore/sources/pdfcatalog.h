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

#ifndef PDFCATALOG_H
#define PDFCATALOG_H

#include "pdfobject.h"
#include "pdfpage.h"
#include "pdfoptionalcontent.h"
#include "pdfoutline.h"
#include "pdfaction.h"

#include <array>
#include <vector>
#include <utility>

namespace pdf
{

class PDFDocument;

/// Defines page layout. Default value is SinglePage. This enum specifies the page layout
/// to be used in viewer application.
enum class PageLayout
{
    SinglePage,         ///< Display one page at time (single page on screen)
    OneColumn,          ///< Displays pages in one column (continuous mode)
    TwoColumnLeft,      ///< Display pages in two continuous columns, odd numbered pages are on the left
    TwoColumnRight,     ///< Display pages in two continuous columns, even numbered pages are on the left
    TwoPagesLeft,       ///< Display two pages on the screen, odd numbered pages are on the left
    TwoPagesRight,      ///< Display two pages on the screen, even numbered pages are on the left
    Custom              ///< Custom layout, multiple columns can be used, -1 as page index means page is omitted
};

/// Specifies, how the document should be displayed in the viewer application.
enum class PageMode
{
    UseNone,            ///< Default value, neither document outline or thumbnails are visible
    UseOutlines,        ///< Document outline window is selected and visible
    UseThumbnails,      ///< Document thumbnails window is selected and visible
    Fullscreen,         ///< Use fullscreen mode, no menu bars, window controls, or any other window visible (presentation mode)
    UseOptionalContent, ///< Optional content group window is selected and visible
    UseAttachments,     ///< Attachments window is selected and visible
};

/// Represents page numbering definition object
class PDFPageLabel
{
public:

    enum class NumberingStyle
    {
        None,               ///< This means, only prefix is used, no numbering
        DecimalArabic,
        UppercaseRoman,
        LowercaseRoman,
        UppercaseLetters,
        LowercaseLetters
    };

    explicit inline PDFPageLabel() :
        m_numberingType(NumberingStyle::None),
        m_prefix(),
        m_pageIndex(0),
        m_startNumber(0)
    {

    }

    explicit inline PDFPageLabel(NumberingStyle numberingType, const QString& prefix, PDFInteger pageIndex, PDFInteger startNumber) :
        m_numberingType(numberingType),
        m_prefix(prefix),
        m_pageIndex(pageIndex),
        m_startNumber(startNumber)
    {

    }

    /// Comparison operator, works only with page indices (because they should be unique)
    bool operator<(const PDFPageLabel& other) const { return m_pageIndex < other.m_pageIndex; }

    NumberingStyle getNumberingStyle() const { return m_numberingType; }
    const QString& getPrefix() const { return m_prefix; }
    PDFInteger getPageIndex() const { return m_pageIndex; }
    PDFInteger getPageStartNumber() const { return m_startNumber; }

    /// Parses page label object from PDF object, according to PDF Reference 1.7, Table 8.10
    static PDFPageLabel parse(PDFInteger pageIndex, const PDFObjectStorage* storage, const PDFObject& object);

private:
    NumberingStyle m_numberingType;
    QString m_prefix;
    PDFInteger m_pageIndex;
    PDFInteger m_startNumber;
};

/// Info about the document. Title, Author, Keywords... It also stores "extra"
/// values, which are in info dictionary. They can be either strings, or date
/// time (QString or QDateTime).
struct PDFDocumentInfo
{
    /// Indicates, that document was modified that it includes trapping information.
    /// See PDF Reference 1.7, Section 10.10.5 "Trapping Support".
    enum class Trapped
    {
        True,       ///< Fully trapped
        False,      ///< Not yet trapped
        Unknown     ///< Either unknown, or it has been trapped partly, not fully
    };

    /// Parses info from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then exception is thrown. This function may throw exceptions, if error occured.
    /// \param object Object containing info dictionary
    /// \param storage Storage of objects
    static PDFDocumentInfo parse(const PDFObject& object, const PDFObjectStorage* storage);

    QString title;
    QString author;
    QString subject;
    QString keywords;
    QString creator;
    QString producer;
    QDateTime creationDate;
    QDateTime modifiedDate;
    Trapped trapped = Trapped::Unknown;
    PDFVersion version;
    std::map<QByteArray, QVariant> extra;
};

class PDFViewerPreferences
{
public:

    enum OptionFlag
    {
        None                = 0x0000,   ///< Empty flag
        HideToolbar         = 0x0001,   ///< Hide toolbar
        HideMenubar         = 0x0002,   ///< Hide menubar
        HideWindowUI        = 0x0004,   ///< Hide window UI (for example scrollbars, navigation controls, etc.)
        FitWindow           = 0x0008,   ///< Resize window to fit first displayed page
        CenterWindow        = 0x0010,   ///< Position of the document's window should be centered on the screen
        DisplayDocTitle     = 0x0020,   ///< Display documents title instead of file name (introduced in PDF 1.4)
        PickTrayByPDFSize   = 0x0040,   ///< Pick tray by PDF size (printing option)
        EnforcePrintScaling = 0x0080    ///< Enforce print scaling settings
    };

    Q_DECLARE_FLAGS(OptionFlags, OptionFlag)

    /// This enum specifies, how to display document, when exiting full screen mode.
    enum class NonFullScreenPageMode
    {
        UseNone,
        UseOutline,
        UseThumbnails,
        UseOptionalContent
    };

    /// Predominant reading order of text.
    enum class Direction
    {
        LeftToRight,    ///< Default
        RightToLeft     ///< Reading order is right to left. Also used for vertical writing systems (Chinese/Japan etc.)
    };

    /// Printer settings - paper handling option to use when printing the document.
    enum class Duplex
    {
        None,
        Simplex,
        DuplexFlipShortEdge,
        DuplexFlipLongEdge
    };

    enum class PrintScaling
    {
        None,
        AppDefault
    };

    enum Properties
    {
        ViewArea,
        ViewClip,
        PrintArea,
        PrintClip,
        EndProperties
    };

    inline PDFViewerPreferences() = default;

    inline PDFViewerPreferences(const PDFViewerPreferences&) = default;
    inline PDFViewerPreferences(PDFViewerPreferences&&) = default;

    inline PDFViewerPreferences& operator=(const PDFViewerPreferences&) = default;
    inline PDFViewerPreferences& operator=(PDFViewerPreferences&&) = default;

    /// Parses viewer preferences from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then exception is thrown.
    static PDFViewerPreferences parse(const PDFObject& catalogDictionary, const PDFDocument* document);

    OptionFlags getOptions() const { return m_optionFlags; }
    const QByteArray& getProperty(Properties property) const { return m_properties.at(property); }
    NonFullScreenPageMode getNonFullScreenPageMode() const { return m_nonFullScreenPageMode; }
    Direction getDirection() const { return m_direction; }
    Duplex getDuplex() const { return m_duplex; }
    PrintScaling getPrintScaling() const { return m_printScaling; }
    const std::vector<std::pair<PDFInteger, PDFInteger>>& getPrintPageRanges() const { return m_printPageRanges; }
    PDFInteger getNumberOfCopies() const { return m_numberOfCopies; }

private:
    OptionFlags m_optionFlags = None;
    std::array<QByteArray, EndProperties> m_properties;
    NonFullScreenPageMode m_nonFullScreenPageMode = NonFullScreenPageMode::UseNone;
    Direction m_direction = Direction::LeftToRight;
    Duplex m_duplex = Duplex::None;
    PrintScaling m_printScaling = PrintScaling::AppDefault;
    std::vector<std::pair<PDFInteger, PDFInteger>> m_printPageRanges;
    PDFInteger m_numberOfCopies = 1;
};

/// Document security store. Contains certificates, CRLs, OCSPs, and
/// other data for signature validation.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentSecurityStore
{
public:
    explicit inline PDFDocumentSecurityStore() = default;

    struct SecurityStoreItem
    {
        std::vector<QByteArray> Cert;
        std::vector<QByteArray> CRL;
        std::vector<QByteArray> OCSP;
        QDateTime created;
        QByteArray timestamp;
    };

    /// Returns master item. Return value is never nullptr.
    const SecurityStoreItem* getMasterItem() const { return &m_master; }

    /// Get item using hash. If item is not found, master item is returned.
    const SecurityStoreItem* getItem(const QByteArray& hash) const;

    /// Parses document security store from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then empty object is returned.
    static PDFDocumentSecurityStore parse(const PDFObject& object, const PDFDocument* document);

private:
    SecurityStoreItem m_master;
    std::map<QByteArray, SecurityStoreItem> m_VRI;
};

/// Article thread. Each thread contains beads, which can be across multiple pages.
class PDFArticleThread
{
public:
    explicit inline PDFArticleThread() = default;

    struct Bead
    {
        PDFObjectReference self;
        PDFObjectReference thread;
        PDFObjectReference next;
        PDFObjectReference previous;
        PDFObjectReference page;
        QRectF rect;
    };
    using Beads = std::vector<Bead>;
    using Information = PDFDocumentInfo;

    const Beads& getBeads() const { return m_beads; }
    const Information& getInformation() const { return m_information; }
    const PDFObjectReference& getMetadata() const { return m_metadata; }

    /// Parses article thread from object. If object cannot be parsed, or error occurs,
    /// then empty object is returned.
    /// \param storage Storage
    /// \param object Object
    static PDFArticleThread parse(const PDFObjectStorage* storage, const PDFObject& object);

private:
    Beads m_beads;
    Information m_information;
    PDFObjectReference m_metadata;
};

/// Document extensions. Contains information about developer's extensions
/// used in document.
class PDF4QTLIBCORESHARED_EXPORT PDFDeveloperExtensions
{
public:
    explicit PDFDeveloperExtensions() = default;

    struct Extension
    {
        QByteArray name;
        QByteArray baseVersion;
        PDFInteger extensionLevel = 0;
        QByteArray url;
    };

    using Extensions = std::vector<Extension>;

    /// Returns list of extensions
    const Extensions& getExtensions() const { return m_extensions; }

    /// Parses extensions from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then empty object is returned, no exception is thrown.
    /// \param object Extensions dictionary
    /// \param document Document
    static PDFDeveloperExtensions parse(const PDFObject& object, const PDFDocument* document);

private:
    Extensions m_extensions;
};

/// Web capture info
class PDF4QTLIBCORESHARED_EXPORT PDFWebCaptureInfo
{
public:
    explicit PDFWebCaptureInfo() = default;

    const QByteArray& getVersion() const { return m_version; }
    const std::vector<PDFObjectReference>& getCommands() const { return m_commands; }

    /// Parses web capture info from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then empty object is returned, no exception is thrown.
    /// \param object Spider info dictionary
    /// \param storage Storage
    static PDFWebCaptureInfo parse(const PDFObject& object, const PDFObjectStorage* storage);

private:
    QByteArray m_version;
    std::vector<PDFObjectReference> m_commands;
};

class PDF4QTLIBCORESHARED_EXPORT PDFOutputIntentICCProfileInfo
{
public:
    explicit PDFOutputIntentICCProfileInfo() = default;

    const QByteArray& getChecksum() const { return m_checkSum; }
    const std::vector<QByteArray>& getColorants() const { return m_colorants; }
    const QByteArray& getIccVersion() const { return m_iccVersion; }
    const QByteArray& getSignature() const { return m_signature; }
    const QString& getProfileName() const { return m_profileName; }
    const PDFObject& getUrls() const { return m_urls; }

    /// Parses icc profile info from object. If object cannot be parsed, or error occurs,
    /// then empty object is returned, no exception is thrown.
    /// \param object Output intent dictionary
    /// \param storage Storage
    static PDFOutputIntentICCProfileInfo parse(const PDFObject& object, const PDFObjectStorage* storage);

private:
    QByteArray m_checkSum;
    std::vector<QByteArray> m_colorants;
    QByteArray m_iccVersion;
    QByteArray m_signature;
    QString m_profileName;
    PDFObject m_urls;
};

/// Output intent
class PDF4QTLIBCORESHARED_EXPORT PDFOutputIntent
{
public:
    explicit PDFOutputIntent() = default;

    const QByteArray& getSubtype() const { return m_subtype; }
    const QString& getOutputCondition() const { return m_outputCondition; }
    const QString& getOutputConditionIdentifier() const { return m_outputConditionIdentifier; }
    const QString& getRegistryName() const { return m_registryName; }
    const QString& getInfo() const { return m_info; }
    const PDFObject& getOutputProfile() const { return m_destOutputProfile; }
    const PDFOutputIntentICCProfileInfo& getOutputProfileInfo() const { return m_destOutputProfileRef; }
    const PDFObject& getMixingHints() const { return m_mixingHints; }
    const PDFObject& getSpectralData() const { return m_spectralData; }

    /// Parses output intent from object. If object cannot be parsed, or error occurs,
    /// then empty object is returned, no exception is thrown.
    /// \param object Output intent dictionary
    /// \param storage Storage
    static PDFOutputIntent parse(const PDFObjectStorage* storage, const PDFObject& object);

private:
    QByteArray m_subtype;
    QString m_outputCondition;
    QString m_outputConditionIdentifier;
    QString m_registryName;
    QString m_info;
    PDFObject m_destOutputProfile;
    PDFOutputIntentICCProfileInfo m_destOutputProfileRef;
    PDFObject m_mixingHints;
    PDFObject m_spectralData;
};

/// Legal attestations
class PDF4QTLIBCORESHARED_EXPORT PDFLegalAttestation
{
public:
    explicit inline PDFLegalAttestation() = default;

    enum Entry
    {
        JavaScriptActions,
        LaunchActions,
        URIActions,
        MovieActions,
        SoundActions,
        HideAnnotationActions,
        GoToRemoteActions,
        AlternateImages,
        ExternalStreams,
        TrueTypeFonts,
        ExternalRefXobjects,
        ExternalOPIdicts,
        NonEmbeddedFonts,
        DevDepGS_OP,
        DevDepGS_HT,
        DevDepGS_TR,
        DevDepGS_UCR,
        DevDepGS_BG,
        DevDepGS_FL,
        LastEntry
    };

    PDFInteger getEntry(Entry entry) const { return m_entries.at(entry); }
    bool hasOptionalContent() const { return m_hasOptionalContent; }
    const QString& getAttestationText() const { return m_attestation; }

    /// Parses legal attestation from object. If object cannot be parsed, or error occurs,
    /// then no object is returned, no exception is thrown.
    /// \param object Legal attestation dictionary
    /// \param storage Storage
    static std::optional<PDFLegalAttestation> parse(const PDFObjectStorage* storage, const PDFObject& object);

private:
    std::array<PDFInteger, LastEntry> m_entries = { };
    bool m_hasOptionalContent = false;
    QString m_attestation;
};

/// Document can contain requirements for viewer application. This class
/// verifies, if this library and viewer application satisfies these requirements
/// and returns result.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentRequirements
{
public:

    enum Requirement
    {
        None                    = 0x00000000,
        OCInteract              = 0x00000001,
        OCAutoStates            = 0x00000002,
        AcroFormInteract        = 0x00000004,
        Navigation              = 0x00000008,
        Markup                  = 0x00000010,
        _3DMarkup               = 0x00000020,
        Multimedia              = 0x00000040,
        U3D                     = 0x00000080,
        PRC                     = 0x00000100,
        Action                  = 0x00000200,
        EnableJavaScripts       = 0x00000400,
        Attachment              = 0x00000800,
        AttachmentEditing       = 0x00001000,
        Collection              = 0x00002000,
        CollectionEditing       = 0x00004000,
        DigSigValidation        = 0x00008000,
        DigSig                  = 0x00010000,
        DigSigMDP               = 0x00020000,
        RichMedia               = 0x00040000,
        Geospatial2D            = 0x00080000,
        Geospatial3D            = 0x00100000,
        DPartInteract           = 0x00200000,
        SeparationSimulation    = 0x00400000,
        Transitions             = 0x00800000,
        Encryption              = 0x01000000
    };

    Q_DECLARE_FLAGS(Requirements, Requirement)

    struct RequirementEntry
    {
        Requirement requirement = None;
        PDFInteger penalty = 100;
        QByteArray version;
        PDFObject handler;

        static RequirementEntry parse(const PDFObjectStorage* storage, const PDFObject& object);
    };

    struct ValidationResult
    {
        Requirements unsatisfied = None;
        PDFInteger penalty = 0;
        QString message;

        bool isOk() const { return penalty < 100; }
        bool isError() const { return !isOk(); }
        bool isWarning() const { return isOk() && !message.isEmpty(); }
    };

    /// Validates requirements against supported requirements
    ValidationResult validate(Requirements supported) const;

    /// Returns string version of requirement
    static QString getRequirementName(Requirement requirement);

    /// Parses document requirements. If error occurs, empty
    /// document requirements are returned.
    /// \param storage Storage
    /// \param object Object
    static PDFDocumentRequirements parse(const PDFObjectStorage* storage, const PDFObject& object);

private:
    std::vector<RequirementEntry> m_requirements;
};

/// Storage for page additional actions
class PDFPageAdditionalActions
{
public:

    enum Action
    {
        Open,
        Close,
        End
    };

    inline explicit PDFPageAdditionalActions() = default;

    /// Returns action for given type. If action is invalid,
    /// or not present, nullptr is returned.
    /// \param action Action type
    const PDFAction* getAction(Action action) const { return m_actions.at(action).get(); }

    /// Returns array with all actions
    const std::array<PDFActionPtr, End>& getActions() const { return m_actions; }

    /// Parses page additional actions from the object. If object is invalid, then
    /// empty additional actions is constructed.
    /// \param storage Object storage
    /// \param object Additional actions object
    static PDFPageAdditionalActions parse(const PDFObjectStorage* storage, PDFObject object);

private:
    std::array<PDFActionPtr, End> m_actions;
};

class PDF4QTLIBCORESHARED_EXPORT PDFCatalog
{
public:
    inline PDFCatalog() = default;

    inline PDFCatalog(const PDFCatalog&) = default;
    inline PDFCatalog(PDFCatalog&&) = default;

    inline PDFCatalog& operator=(const PDFCatalog&) = default;
    inline PDFCatalog& operator=(PDFCatalog&&) = default;

    static constexpr const size_t INVALID_PAGE_INDEX = std::numeric_limits<size_t>::max();

    enum DocumentAction
    {
        WillClose,
        WillSave,
        DidSave,
        WillPrint,
        DidPrint,
        LastDocumentAction
    };

    /// Returns viewer preferences of the application
    const PDFViewerPreferences* getViewerPreferences() const { return &m_viewerPreferences; }

    /// Returns the page count
    size_t getPageCount() const { return m_pages.size(); }

    /// Returns the page
    const PDFPage* getPage(size_t index) const { return &m_pages.at(index); }

    /// Returns page index. If page is not found, then INVALID_PAGE_INDEX is returned.
    size_t getPageIndexFromPageReference(PDFObjectReference reference) const;

    /// Returns optional content properties
    const PDFOptionalContentProperties* getOptionalContentProperties() const { return &m_optionalContentProperties; }

    /// Returns root pointer for outline items
    QSharedPointer<PDFOutlineItem> getOutlineRootPtr() const { return m_outlineRoot; }

    /// Returns action, which should be performed
    const PDFAction* getOpenAction() const { return m_openAction.data(); }

    /// Returns version of the PDF specification, to which the document conforms.
    const QByteArray& getVersion() const { return m_version; }

    PageLayout getPageLayout() const { return m_pageLayout; }
    PageMode getPageMode() const { return m_pageMode; }
    const QByteArray& getBaseURI() const { return m_baseURI; }
    const std::map<QByteArray, PDFFileSpecification>& getEmbeddedFiles() const { return m_namedEmbeddedFiles; }
    const PDFObject& getFormObject() const { return m_formObject; }
    const PDFDeveloperExtensions& getExtensions() const { return m_extensions; }
    const PDFDocumentSecurityStore& getDocumentSecurityStore() const { return m_documentSecurityStore; }
    const std::vector<PDFArticleThread>& getArticleThreads() const { return m_threads; }
    const PDFAction* getDocumentAction(DocumentAction action) const { return m_documentActions.at(action).get(); }
    const auto& getDocumentActions() const { return m_documentActions; }
    const PDFObject& getMetadata() const { return m_metadata; }
    const PDFObject& getStructureTreeRoot() const { return m_structureTreeRoot; }
    const QString& getLanguage() const { return m_language; }
    const PDFWebCaptureInfo& getWebCaptureInfo() const { return m_webCaptureInfo; }
    const std::vector<PDFOutputIntent>& getOutputIntents() const { return m_outputIntents; }
    const PDFObject& getPieceInfo() const { return m_pieceInfo; }
    const PDFObject& getPerms() const { return m_perms; }
    const PDFLegalAttestation* getLegalAttestation() const { return m_legalAttestation.has_value() ? &m_legalAttestation.value() : nullptr; }
    const PDFObject& getRequirements() const { return m_requirements; }
    const PDFObject& getCollection() const { return m_collection; }
    bool isXFANeedsRendering() const { return m_xfaNeedsRendering; }
    const PDFObject& getAssociatedFiles() const { return m_associatedFiles; }
    const PDFObject& getDocumentPartRoot() const { return m_documentPartRoot; }
    const std::map<QByteArray, PDFDestination>& getNamedDestinations() const { return m_namedDestinations; }

    /// Is document marked to have structure tree conforming to tagged document convention?
    bool isLogicalStructureMarked() const { return m_markInfoFlags.testFlag(MarkInfo_Marked); }

    /// Is document marked to have structure tree with user attributes?
    bool isLogicalStructureUserPropertiesUsed() const { return m_markInfoFlags.testFlag(MarkInfo_UserProperties); }

    /// Is document marked to have structure tree not completely conforming to standard?
    bool isLogicalStructureSuspects() const { return m_markInfoFlags.testFlag(MarkInfo_Suspects); }

    /// Returns destination using the key. If destination with the key is not found,
    /// then nullptr is returned.
    /// \param key Destination key
    /// \returns Pointer to the destination, or nullptr
    const PDFDestination* getNamedDestination(const QByteArray& key) const;

    /// Returns javascript action using the key. If javascript action is not found,
    /// then nullptr is returned.
    /// \param key Action key
    /// \returns Javascript action, or nullptr
    PDFActionPtr getNamedJavaScriptAction(const QByteArray& key) const;

    /// Returns appearance stream using the key. If appearance stream is not found,
    /// then empty object is returned.
    /// \param key Appearance stream key
    /// \returns Appearance, or nullptr
    PDFObject getNamedAppearanceStream(const QByteArray& key) const;

    /// Returns named page using the key. If named page is not found,
    /// then empty object is returned.
    /// \param key Page key
    /// \returns Page, or nullptr
    PDFObject getNamedPage(const QByteArray& key) const;

    /// Returns named template using the key. If named template is not found,
    /// then empty object is returned.
    /// \param key Template key
    /// \returns Template, or nullptr
    PDFObject getNamedTemplate(const QByteArray& key) const;

    /// Returns named digital identifier using the key. If named digital identifier is not found,
    /// then empty object is returned. Digital identifiers are used in Web Capture functionality.
    /// See also PDF 2.0 specification.
    /// \param key Digital identifier key
    /// \returns Digital identifier, or nullptr
    PDFObject getNamedDigitalIdentifier(const QByteArray& key) const;

    /// Returns named url using the key. If named url is not found,
    /// then empty object is returned. Urls are used in Web Capture functionality.
    /// See also PDF 2.0 specification.
    /// \param key Url key
    /// \returns Url, or nullptr
    PDFObject getNamedUrl(const QByteArray& key) const;

    /// Returns named alternate representation using the key. If named alternate representation is not found,
    /// then empty object is returned.
    /// \param key Alternate representation key
    /// \returns Alternate representation, or nullptr
    PDFObject getNamedAlternateRepresentation(const QByteArray& key) const;

    /// Returns named rendition using the key. If named rendition is not found,
    /// then empty object is returned.
    /// \param key Rendition key
    /// \returns Rendition, or nullptr
    PDFObject getNamedRendition(const QByteArray& key) const;

    /// Returns all named JavaScript actions
    const std::map<QByteArray, PDFActionPtr>& getNamedJavaScriptActions() const { return m_namedJavaScriptActions; }

    /// Parses catalog from catalog dictionary. If object cannot be parsed, or error occurs,
    /// then exception is thrown.
    static PDFCatalog parse(const PDFObject& catalog, const PDFDocument* document);

private:

    enum MarkInfoFlag : uint8_t
    {
        MarkInfo_None           = 0x0000,
        MarkInfo_Marked         = 0x0001,   ///< Document conforms to tagged PDF convention
        MarkInfo_UserProperties = 0x0002,   ///< Structure tree contains user properties
        MarkInfo_Suspects       = 0x0004,   ///< Suspects
    };
    Q_DECLARE_FLAGS(MarkInfoFlags, MarkInfoFlag)

    QByteArray m_version;
    PDFViewerPreferences m_viewerPreferences;
    std::vector<PDFPage> m_pages;
    std::vector<PDFPageLabel> m_pageLabels;
    PDFOptionalContentProperties m_optionalContentProperties;
    QSharedPointer<PDFOutlineItem> m_outlineRoot;
    PDFActionPtr m_openAction;
    std::array<PDFActionPtr, LastDocumentAction> m_documentActions;
    PageLayout m_pageLayout = PageLayout::SinglePage;
    PageMode m_pageMode = PageMode::UseNone;
    QByteArray m_baseURI;
    PDFObject m_formObject;
    PDFObject m_structureTreeRoot;
    PDFDeveloperExtensions m_extensions;
    PDFDocumentSecurityStore m_documentSecurityStore;
    std::vector<PDFArticleThread> m_threads;
    PDFObject m_metadata;
    MarkInfoFlags m_markInfoFlags = MarkInfo_None;
    QString m_language;
    PDFWebCaptureInfo m_webCaptureInfo;
    std::vector<PDFOutputIntent> m_outputIntents;
    PDFObject m_pieceInfo;
    PDFObject m_perms;
    std::optional<PDFLegalAttestation> m_legalAttestation;
    PDFObject m_requirements;
    PDFObject m_collection;
    bool m_xfaNeedsRendering = false;
    PDFObject m_associatedFiles;
    PDFObject m_documentPartRoot;

    // Maps from Names dictionary
    std::map<QByteArray, PDFDestination> m_namedDestinations;
    std::map<QByteArray, PDFObject> m_namedAppearanceStreams;
    std::map<QByteArray, PDFActionPtr> m_namedJavaScriptActions;
    std::map<QByteArray, PDFObject> m_namedPages;
    std::map<QByteArray, PDFObject> m_namedTemplates;
    std::map<QByteArray, PDFObject> m_namedDigitalIdentifiers;
    std::map<QByteArray, PDFObject> m_namedUniformResourceLocators;
    std::map<QByteArray, PDFFileSpecification> m_namedEmbeddedFiles;
    std::map<QByteArray, PDFObject> m_namedAlternateRepresentations;
    std::map<QByteArray, PDFObject> m_namedRenditions;
};

}   // namespace pdf

#endif // PDFCATALOG_H
