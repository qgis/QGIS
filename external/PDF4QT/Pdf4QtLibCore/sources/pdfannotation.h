//    Copyright (C) 2020-2021 Jakub Melka
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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFANNOTATION_H
#define PDFANNOTATION_H

#include "pdfutils.h"
#include "pdfobject.h"
#include "pdfaction.h"
#include "pdffile.h"
#include "pdfcms.h"
#include "pdfmultimedia.h"
#include "pdfmeshqualitysettings.h"
#include "pdfrenderer.h"
#include "pdfblendfunction.h"
#include "pdfdocument.h"
#include "pdfcolorconvertor.h"
#include "pdftextlayout.h"

#include <QCursor>
#include <QPainterPath>

#include <array>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace pdf
{
class PDFWidget;
class PDFObjectStorage;
class PDFFontCache;
class PDFFormManager;
class PDFModifiedDocument;
class PDFOptionalContentActivity;
class PDFFormFieldWidgetEditor;
class PDFModifiedDocument;

using TextAlignment = Qt::Alignment;
using Polygons = std::vector<QPolygonF>;

enum class AnnotationType
{
    Invalid,
    Text,
    Link,
    FreeText,
    Line,
    Square,
    Circle,
    Polygon,
    Polyline,
    Highlight,
    Underline,
    Squiggly,
    StrikeOut,
    Stamp,
    Caret,
    Ink,
    Popup,
    FileAttachment,
    Sound,
    Movie,
    Widget,
    Screen,
    PrinterMark,
    TrapNet,
    Watermark,
    Redact,
    Projection,
    _3D,
    RichMedia
};

enum class AnnotationLineEnding
{
    None,
    Square,
    Circle,
    Diamond,
    OpenArrow,
    ClosedArrow,
    Butt,
    ROpenArrow,
    RClosedArrow,
    Slash
};

/// Represents annotation's border. Two main definition exists, one is older,
/// called \p Simple, the other one is defined in BS dictionary of the annotation.
class PDFAnnotationBorder
{
public:
    explicit inline PDFAnnotationBorder() = default;

    enum class Definition
    {
        Invalid,
        Simple,
        BorderStyle
    };

    enum class Style
    {
        Solid,
        Dashed,
        Beveled,
        Inset,
        Underline
    };

    /// Parses the annotation border from the array. If object contains invalid annotation border,
    /// then default annotation border is returned. If object is empty, empty annotation border is returned.
    /// \param storage Object storage
    /// \param object Border object
    static PDFAnnotationBorder parseBorder(const PDFObjectStorage* storage, PDFObject object);

    /// Parses the annotation border from the BS dictionary. If object contains invalid annotation border,
    /// then default annotation border is returned. If object is empty, empty annotation border is returned.
    /// \param storage Object storage
    /// \param object Border object
    static PDFAnnotationBorder parseBS(const PDFObjectStorage* storage, PDFObject object);

    /// Returns true, if object is correctly defined
    bool isValid() const { return m_definition != Definition::Invalid; }

    Definition getDefinition() const { return m_definition; }
    Style getStyle() const { return m_style; }
    PDFReal getHorizontalCornerRadius() const { return m_hCornerRadius; }
    PDFReal getVerticalCornerRadius() const { return m_vCornerRadius; }
    PDFReal getWidth() const { return m_width; }
    const std::vector<PDFReal>& getDashPattern() const { return m_dashPattern; }

private:
    Definition m_definition = Definition::Invalid;
    Style m_style = Style::Solid;
    PDFReal m_hCornerRadius = 0.0;
    PDFReal m_vCornerRadius = 0.0;
    PDFReal m_width = 1.0;
    std::vector<PDFReal> m_dashPattern;
};

using AnnotationBorderStyle = PDFAnnotationBorder::Style;

/// Annotation border effect
class PDFAnnotationBorderEffect
{
public:
    explicit inline PDFAnnotationBorderEffect() = default;

    enum class Effect
    {
        None,
        Cloudy
    };

    /// Parses the annotation border effect from the object. If object contains invalid annotation border effect,
    /// then default annotation border effect is returned. If object is empty, also default annotation border effect is returned.
    /// \param storage Object storage
    /// \param object Border effect object
    static PDFAnnotationBorderEffect parse(const PDFObjectStorage* storage, PDFObject object);

private:
    Effect m_effect = Effect::None;
    PDFReal m_intensity = 0.0;
};

/// Storage which handles appearance streams of annotations. Appeareance streams are divided
/// to three main categories - normal, rollower and down. Each category can have different
/// states, for example, checkbox can have on/off state. This container can also resolve
/// queries to obtain appropriate appearance stream.
class PDFAppeareanceStreams
{
public:
    explicit inline PDFAppeareanceStreams() = default;

    enum class Appearance
    {
        Normal,
        Rollover,
        Down
    };

    using Key = std::pair<Appearance, QByteArray>;

    /// Parses annotation appearance streams from the object. If object is invalid, then
    /// empty appearance stream is constructed.
    /// \param storage Object storage
    /// \param object Appearance streams object
    static PDFAppeareanceStreams parse(const PDFObjectStorage* storage, PDFObject object);

    /// Tries to search for appearance stream for given appearance. If no appearance is found,
    /// then null object is returned.
    /// \param appearance Appearance type
    PDFObject getAppearance(Appearance appearance = Appearance::Normal) const { return getAppearance(appearance, QByteArray()); }

    /// Tries to resolve appearance stream for given appearance and state. If no appearance is found,
    /// then null object is returned.
    /// \param appearance Appearance type
    /// \param state State name
    PDFObject getAppearance(Appearance appearance, const QByteArray& state) const;

    /// Returns list of appearance states for given appearance
    /// \param appearance Appearance
    QByteArrayList getAppearanceStates(Appearance appearance) const;

    /// Returns list of appearance keys
    std::vector<Key> getAppearanceKeys() const;

private:
    std::map<Key, PDFObject> m_appearanceStreams;
};

/// Represents annotation's active region, it is used also to
/// determine underline lines.
class PDFAnnotationQuadrilaterals
{
public:
    using Quadrilateral = std::array<QPointF, 4>;
    using Quadrilaterals = std::vector<Quadrilateral>;

    inline explicit PDFAnnotationQuadrilaterals() = default;
    inline explicit PDFAnnotationQuadrilaterals(QPainterPath&& path, Quadrilaterals&& quadrilaterals) :
        m_path(qMove(path)),
        m_quadrilaterals(qMove(quadrilaterals))
    {

    }

    const QPainterPath& getPath() const { return m_path; }
    const Quadrilaterals& getQuadrilaterals() const { return m_quadrilaterals; }
    bool isEmpty() const { return m_path.isEmpty(); }

private:
    QPainterPath m_path;
    Quadrilaterals m_quadrilaterals;
};

/// Represents callout line (line from annotation to some point)
class PDFAnnotationCalloutLine
{
public:

    enum class Type
    {
        Invalid,
        StartEnd,
        StartKneeEnd
    };

    inline explicit PDFAnnotationCalloutLine() = default;
    inline explicit PDFAnnotationCalloutLine(QPointF start, QPointF end) :
        m_type(Type::StartEnd),
        m_points({start, end})
    {

    }

    inline explicit PDFAnnotationCalloutLine(QPointF start, QPointF knee, QPointF end) :
        m_type(Type::StartKneeEnd),
        m_points({start, knee, end})
    {

    }

    /// Parses annotation callout line from the object. If object is invalid, then
    /// invalid callout line is constructed.
    /// \param storage Object storage
    /// \param object Callout line object
    static PDFAnnotationCalloutLine parse(const PDFObjectStorage* storage, PDFObject object);

    bool isValid() const { return m_type != Type::Invalid; }
    Type getType() const { return m_type; }

    QPointF getPoint(int index) const { return m_points.at(index); }

private:
    Type m_type = Type::Invalid;
    std::array<QPointF, 3> m_points;
};

/// Information about annotation icon fitting (in the widget)
class PDFAnnotationIconFitInfo
{
public:
    inline explicit PDFAnnotationIconFitInfo() = default;

    enum class ScaleCondition
    {
        Always,
        ScaleBigger,
        ScaleSmaller,
        Never
    };

    enum class ScaleType
    {
        Anamorphic,  ///< Do not keep aspect ratio, fit whole annotation rectangle
        Proportional ///< Keep aspect ratio, annotation rectangle may not be filled fully with icon
    };

    /// Parses annotation appearance icon fit info from the object. If object is invalid, then
    /// default appearance icon fit info is constructed.
    /// \param storage Object storage
    /// \param object Appearance icon fit info object
    static PDFAnnotationIconFitInfo parse(const PDFObjectStorage* storage, PDFObject object);

private:
    ScaleCondition m_scaleCondition = ScaleCondition::Always;
    ScaleType m_scaleType = ScaleType::Proportional;
    QPointF m_relativeProportionalPosition = QPointF(0.5, 0.5);
    bool m_fullBox = false;
};

/// Additional appearance characteristics used for constructing of appearance
/// stream to display annotation on the screen (or just paint it).
class PDFAnnotationAppearanceCharacteristics
{
public:
    inline explicit PDFAnnotationAppearanceCharacteristics() = default;

    enum class PushButtonMode
    {
        NoIcon,
        NoCaption,
        IconWithCaptionBelow,
        IconWithCaptionAbove,
        IconWithCaptionRight,
        IconWithCaptionLeft,
        IconWithCaptionOverlaid
    };

    /// Number of degrees by which the widget annotation is rotated
    /// counterclockwise relative to the page.
    PDFInteger getRotation() const { return m_rotation; }
    const std::vector<PDFReal>& getBorderColor() const { return m_borderColor; }
    const std::vector<PDFReal>& getBackgroundColor() const { return m_backgroundColor; }
    const QString& getNormalCaption() const { return m_normalCaption; }
    const QString& getRolloverCaption() const { return m_rolloverCaption; }
    const QString& getDownCaption() const { return m_downCaption; }
    const PDFObject& getNormalIcon() const { return m_normalIcon; }
    const PDFObject& getRolloverIcon() const { return m_rolloverIcon; }
    const PDFObject& getDownIcon() const { return m_downIcon; }
    const PDFAnnotationIconFitInfo& getIconFit() const { return m_iconFit; }
    PushButtonMode getPushButtonMode() const { return m_pushButtonMode; }

    /// Parses annotation appearance characteristics from the object. If object is invalid, then
    /// default appearance characteristics is constructed.
    /// \param storage Object storage
    /// \param object Appearance characteristics object
    static PDFAnnotationAppearanceCharacteristics parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFInteger m_rotation = 0;
    std::vector<PDFReal> m_borderColor;
    std::vector<PDFReal> m_backgroundColor;
    QString m_normalCaption;
    QString m_rolloverCaption;
    QString m_downCaption;
    PDFObject m_normalIcon;
    PDFObject m_rolloverIcon;
    PDFObject m_downIcon;
    PDFAnnotationIconFitInfo m_iconFit;
    PushButtonMode m_pushButtonMode = PushButtonMode::NoIcon;
};

/// Storage for annotation additional actions
class PDFAnnotationAdditionalActions
{
public:

    enum Action
    {
        CursorEnter,
        CursorLeave,
        MousePressed,
        MouseReleased,
        FocusIn,
        FocusOut,
        PageOpened,
        PageClosed,
        PageShow,
        PageHide,
        FormFieldModified,
        FormFieldFormatted,
        FormFieldValidated,
        FormFieldCalculated,
        Default,
        End
    };

    inline explicit PDFAnnotationAdditionalActions() = default;

    /// Returns action for given type. If action is invalid,
    /// or not present, nullptr is returned.
    /// \param action Action type
    const PDFAction* getAction(Action action) const { return m_actions.at(action).get(); }

    /// Returns array with all actions
    const std::array<PDFActionPtr, End>& getActions() const { return m_actions; }

    /// Parses annotation additional actions from the object. If object is invalid, then
    /// empty additional actions is constructed.
    /// \param storage Object storage
    /// \param object Additional actions object
    /// \param defaultAction Default action of object (defined in "A" entry of annotation dictionary)
    static PDFAnnotationAdditionalActions parse(const PDFObjectStorage* storage, PDFObject object, PDFObject defaultAction);

private:
    std::array<PDFActionPtr, End> m_actions;
};

/// Annotation default appearance
class PDF4QTLIBCORESHARED_EXPORT PDFAnnotationDefaultAppearance
{
public:
    explicit inline PDFAnnotationDefaultAppearance() = default;

    /// Parses appearance string. If error occurs, then default appearance
    /// string is constructed.
    static PDFAnnotationDefaultAppearance parse(const QByteArray& string);

    const QByteArray& getFontName() const { return m_fontName; }
    PDFReal getFontSize() const { return m_fontSize; }
    QColor getFontColor() const { return m_fontColor; }

private:
    QByteArray m_fontName;
    PDFReal m_fontSize = 8.0;
    QColor m_fontColor = Qt::black;
};

class PDFAnnotation;
class PDFMarkupAnnotation;
class PDFTextAnnotation;

using PDFAnnotationPtr = QSharedPointer<PDFAnnotation>;

struct AnnotationDrawParameters
{
    /// Painter, onto which is annotation graphics drawn
    QPainter* painter = nullptr;

    /// Pointer to annotation (if draw is delegated to other objects,
    /// for example, form manager, then maybe pointer to annotation
    /// is needed).
    PDFAnnotation* annotation = nullptr;

    /// Pointer to form manager (if forms are drawn)
    const PDFFormManager* formManager = nullptr;

    /// Output parameter. Marks annotation's graphics bounding
    /// rectangle (it can be different/adjusted from original
    /// annotation bounding rectangle, in that case, it must be adjusted).
    /// If this rectangle is invalid, then appearance content stream
    /// is assumed to be empty.
    QRectF boundingRectangle;

    /// Appeareance mode (normal/rollover/down, and appearance state)
    PDFAppeareanceStreams::Key key;

    /// Color convertor
    PDFColorConvertor colorConvertor;
};

/// Base class for all annotation types. Represents PDF annotation object.
/// Annotations are various enhancements to pages graphical representation,
/// such as graphics, text, highlight or multimedia content, such as sounds,
/// videos and 3D annotations.
class PDF4QTLIBCORESHARED_EXPORT PDFAnnotation
{
public:
    explicit PDFAnnotation();
    virtual ~PDFAnnotation() = default;

    enum Flag : uint
    {
        None            = 0x0000,
        Invisible       = 0x0001, ///< If set, do not display unknown annotations using their AP dictionary
        Hidden          = 0x0002, ///< If set, do not display annotation and do not show popup windows (completely hidden)
        Print           = 0x0004, ///< If set, print annotation
        NoZoom          = 0x0008, ///< Do not apply page zoom while displaying annotation rectangle
        NoRotate        = 0x0010, ///< Do not rotate annotation's appearance to match orientation of the page
        NoView          = 0x0020, ///< Do not display annotation on the screen (it still can be printed)
        ReadOnly        = 0x0040, ///< Do not allow interacting with the user (and disallow also mouse interaction)
        Locked          = 0x0080, ///< Do not allow to delete/modify annotation by user
        ToggleNoView    = 0x0100, ///< If set, invert the interpretation of NoView flag
        LockedContents  = 0x0200, ///< Do not allow to modify contents of the annotation
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    virtual AnnotationType getType() const = 0;

    virtual PDFMarkupAnnotation* asMarkupAnnotation() { return nullptr; }
    virtual const PDFMarkupAnnotation* asMarkupAnnotation() const { return nullptr; }
    virtual bool isReplyTo() const { return false; }
    virtual QString getGUICaption() const;

    /// Draws the annotation using parameters. Annotation is drawn onto painter,
    /// but actual graphics can be drawn outside of annotation's rectangle.
    /// In that case, adjusted annotation's rectangle is passed to the parameters.
    /// Painter must use annotation's coordinate system (for example, line points
    /// must match in both in painter and this annotation).
    /// \param parameters Graphics parameters
    virtual void draw(AnnotationDrawParameters& parameters) const;

    /// Returns a list of appearance states, which must be created for this annotation
    virtual std::vector<PDFAppeareanceStreams::Key> getDrawKeys(const PDFFormManager* formManager) const;

    /// Returns effective flags (some annotations can behave as they have always
    /// set some flags, such as NoZoom and NoRotate)
    virtual Flags getEffectiveFlags() const { return getFlags(); }

    PDFObjectReference getSelfReference() const { return m_selfReference; }
    const QRectF& getRectangle() const { return m_rectangle; }
    const QString& getContents() const { return m_contents; }
    PDFObjectReference getPageReference() const { return m_pageReference; }
    const QString& getName() const { return m_name; }
    const QDateTime& getLastModifiedDateTime() const { return m_lastModified; }
    const QString& getLastModifiedString() const { return m_lastModifiedString; }
    Flags getFlags() const { return m_flags; }
    const PDFAppeareanceStreams& getAppearanceStreams() const { return m_appearanceStreams; }
    const QByteArray& getAppearanceState() const { return m_appearanceState; }
    const PDFAnnotationBorder& getBorder() const { return m_annotationBorder; }
    const std::vector<PDFReal>& getColor() const { return m_color; }
    PDFInteger getStructuralParent() const { return m_structParent; }
    PDFObjectReference getOptionalContent() const { return m_optionalContentReference; }
    const PDFObject& getAssociatedFiles() const { return m_associatedFiles; }
    PDFReal getFillOpacity() const { return m_fillingOpacity; }
    PDFReal getStrokeOpacity() const { return m_strokingOpacity; }
    BlendMode getBlendMode() const { return m_blendMode; }
    const QString& getLanguage() const { return m_language; }

    void setSelfReference(const PDFObjectReference& selfReference);
    void setRectangle(const QRectF& rectangle);
    void setContents(const QString& contents);
    void setPageReference(const PDFObjectReference& pageReference);
    void setName(const QString& name);
    void setLastModified(const QDateTime& lastModified);
    void setLastModifiedString(const QString& lastModifiedString);
    void setFlags(const Flags& flags);
    void setAppearanceStreams(const PDFAppeareanceStreams& appearanceStreams);
    void setAppearanceState(const QByteArray& appearanceState);
    void setAnnotationBorder(const PDFAnnotationBorder& annotationBorder);
    void setColor(const std::vector<PDFReal>& color);
    void setStructParent(const PDFInteger& structParent);
    void setOptionalContentReference(const PDFObjectReference& optionalContentReference);
    void setAssociatedFiles(const PDFObject& associatedFiles);
    void setFillingOpacity(const PDFReal& fillingOpacity);
    void setStrokingOpacity(const PDFReal& strokingOpacity);
    void setBlendMode(const BlendMode& blendMode);
    void setLanguage(const QString& language);

    /// Returns current composition mode. If blend mode is not supported by Qt,
    /// then normal composition mode is returned.
    QPainter::CompositionMode getCompositionMode() const;

    /// Parses annotation from the object. If error occurs, then nullptr is returned.
    /// \param storage Object storage
    /// \param reference Annotation object reference
    static PDFAnnotationPtr parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    /// Parses quadrilaterals and fills them in the painter path. If no quadrilaterals are defined,
    /// then annotation rectangle is used. If annotation rectangle is also invalid,
    /// then empty painter path is used.
    /// \param storage Object storage
    /// \param quadrilateralsObject Object with quadrilaterals definition
    /// \param annotationRect Annotation rectangle
    static PDFAnnotationQuadrilaterals parseQuadrilaterals(const PDFObjectStorage* storage, PDFObject quadrilateralsObject, const QRectF annotationRect);

    /// Converts name to line ending. If appropriate line ending for name is not found,
    /// then None line ending is returned.
    /// \param name Name of the line ending
    static AnnotationLineEnding convertNameToLineEnding(const QByteArray& name);

    /// Converts line ending to name.
    /// \param lineEnding Line ending
    static QByteArray convertLineEndingToName(AnnotationLineEnding lineEnding);

    /// Returns draw color from defined annotation color. If color is incorrectly
    /// defined, then black color is returned.
    /// \param color Color (can have 1, 3 and 4 components)
    /// \param opacity Opacity
    static QColor getDrawColorFromAnnotationColor(const std::vector<PDFReal>& color, PDFReal opacity);

    /// Returns true, if annotation is editable
    /// \param type Annotation type
    static bool isTypeEditable(AnnotationType type);

protected:
    virtual QColor getStrokeColor() const;
    virtual QColor getFillColor() const;

    struct LineGeometryInfo
    {
        /// Original line
        QLineF originalLine;

        /// Transformed line
        QLineF transformedLine;

        /// Matrix LCStoGCS is local coordinate system of line originalLine. It transforms
        /// points on the line to the global coordinate system. So, point (0, 0) will
        /// map onto p1 and point (originalLine.length(), 0) will map onto p2.
        QTransform LCStoGCS;

        /// Inverted matrix of LCStoGCS. It maps global coordinate system to local
        /// coordinate system of the original line.
        QTransform GCStoLCS;

        static LineGeometryInfo create(QLineF line);
    };

    /// Returns pen from border settings and annotation color
    QPen getPen() const;

    /// Returns brush from interior color. If annotation doesn't have
    /// a brush, then empty brush is returned.
    QBrush getBrush() const;

    /// Draw line ending at given point, using parameters. Line ending appearance
    /// is constructed given parameters \p lineEndingSize, \p arrowAxisLength
    /// and \p ending. Parameter \p flipAxis controls, if we are drawing at the
    /// start (false), or at the end (true) of the line. User can specify matrix,
    /// which maps from local coordinate system of the line to the global coordinate
    /// system. Also, bouding path is updated.
    /// \param painter Painter
    /// \param point Point, at which line ending is being drawn
    /// \param lineEndingSize Line ending size
    /// \param arrowAxisLength Length of the arrow
    /// \param ending Type of ending
    /// \param flipAxis Flip axis to draw end of line?
    /// \param LCStoGCS Transformation from local coordinate system of line to global coordinate system
    /// \pram boundingPath Bounding path to be updated
    void drawLineEnding(QPainter* painter,
                        QPointF point,
                        PDFReal lineEndingSize,
                        PDFReal arrowAxisLength,
                        AnnotationLineEnding ending,
                        bool flipAxis,
                        const QTransform& LCStoGCS,
                        QPainterPath& boundingPath) const;

    /// Draw line using given parameters and painter. Line is specified
    /// by its geometry information. Painter must be set to global coordinates.
    /// Bounding path is also updated, it is specified in global coordinates,
    /// not in line local coordinate system. We consider p1 as start point of
    /// the line, and p2 as the end point. The painter must have proper QPen
    /// and QBrush setted, this function uses current pen/brush to paint the line.
    /// \param info Line geometry info
    /// \param painter Painter
    /// \param lineEndingSize Line ending size
    /// \param p1Ending Line endpoint graphics at p1
    /// \param p2Ending Line endpoint graphics at p2
    /// \param boundingPath Bounding path in global coordinate system
    /// \param textOffset Additional text offset
    /// \param text Text, which should be printed along the line
    /// \param textIsAboveLine Text should be printed above line
    void drawLine(const LineGeometryInfo& info,
                  QPainter& painter,
                  PDFReal lineEndingSize,
                  AnnotationLineEnding p1Ending,
                  AnnotationLineEnding p2Ending,
                  QPainterPath& boundingPath,
                  QPointF textOffset,
                  const QString& text,
                  bool textIsAboveLine) const;

    /// Draws character unicode symbol using text
    /// \param text Text to be drawn
    /// \param opacity Opacity
    /// \param parameters Draw parameters
    void drawCharacterSymbol(QString text, PDFReal opacity, AnnotationDrawParameters& parameters) const;

    /// Parses path. If path is incorrect, empty path is returned.
    /// \param storage Storage
    /// \param dictionary Annotation's dictionary
    /// \param closePath Close path when finishing?
    static QPainterPath parsePath(const PDFObjectStorage* storage, const PDFDictionary* dictionary, bool closePath);

private:
    PDFObjectReference m_selfReference; ///< Reference to self
    QRectF m_rectangle; ///< Annotation rectangle, in page coordinates, "Rect" entry
    QString m_contents; ///< Text to be displayed to the user (or alternate text), "Content" entry
    PDFObjectReference m_pageReference; ///< Reference to annotation's page, "P" entry
    QString m_name; ///< Unique name (in page context) for the annotation, "NM" entry
    QDateTime m_lastModified; ///< Date and time, when annotation was last modified, "M" entry
    QString m_lastModifiedString; ///< Date and time, in text format
    Flags m_flags; ///< Annotation flags
    PDFAppeareanceStreams m_appearanceStreams; ///< Appearance streams, "AP" entry
    QByteArray m_appearanceState; ///< Appearance state, "AS" entry
    PDFAnnotationBorder m_annotationBorder; ///< Annotation border, "Border" entry
    std::vector<PDFReal> m_color; ///< Color (for example, title bar of popup window), "C" entry
    PDFInteger m_structParent; ///< Structural parent identifier, "StructParent" entry
    PDFObjectReference m_optionalContentReference; ///< Reference to optional content, "OC" entry
    PDFObject m_associatedFiles;
    PDFReal m_fillingOpacity = 1.0;
    PDFReal m_strokingOpacity = 1.0;
    BlendMode m_blendMode = BlendMode::Normal;
    QString m_language;
};

/// Markup annotation object, used to mark up contents of PDF documents. Markup annotations
/// can have various types, as free text (just text displayed on page), annotations with popup
/// windows, and special annotations, such as multimedia annotations.
class PDFMarkupAnnotation : public PDFAnnotation
{
public:
    explicit inline PDFMarkupAnnotation() = default;

    virtual PDFMarkupAnnotation* asMarkupAnnotation() override { return this; }
    virtual const PDFMarkupAnnotation* asMarkupAnnotation() const override { return this; }
    virtual bool isReplyTo() const override;

    enum class ReplyType
    {
        Reply,
        Group
    };

    const QString& getWindowTitle() const { return m_windowTitle; }
    PDFObjectReference getPopupAnnotation() const { return m_popupAnnotation; }
    const QString& getRichTextString() const { return m_richTextString; }
    const QDateTime& getCreationDate() const { return m_creationDate; }
    PDFObjectReference getInReplyTo() const { return m_inReplyTo; }
    const QString& getSubject() const { return m_subject; }
    ReplyType getReplyType() const { return m_replyType; }
    const QByteArray& getIntent() const { return m_intent; }
    const PDFObject& getExternalData() const { return m_externalData; }

    void setWindowTitle(const QString& windowTitle);
    void setPopupAnnotation(const PDFObjectReference& popupAnnotation);
    void setRichTextString(const QString& richTextString);
    void setCreationDate(const QDateTime& creationDate);
    void setInReplyTo(PDFObjectReference inReplyTo);
    void setSubject(const QString& subject);
    void setReplyType(ReplyType replyType);
    void setIntent(const QByteArray& intent);
    void setExternalData(const PDFObject& externalData);

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QString m_windowTitle;
    PDFObjectReference m_popupAnnotation;
    QString m_richTextString;
    QDateTime m_creationDate;
    PDFObjectReference m_inReplyTo;
    QString m_subject;
    ReplyType m_replyType = ReplyType::Reply;
    QByteArray m_intent;
    PDFObject m_externalData;
};

enum class TextAnnotationIcon
{
    Comment,
    Help,
    Insert,
    Key,
    NewParagraph,
    Note,
    Paragraph
};

/// Text annotation represents note attached to a specific point in the PDF
/// document. It appears as icon, and it is not zoomed, or rotated (behaves
/// as if flag NoZoom and NoRotate were set). When this annotation is opened,
/// it displays popup window containing the text of the note, font and size
/// is implementation dependent by viewer application.
class PDF4QTLIBCORESHARED_EXPORT PDFTextAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFTextAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Text; }
    virtual std::vector<PDFAppeareanceStreams::Key> getDrawKeys(const PDFFormManager* formManager) const override;
    virtual void draw(AnnotationDrawParameters& parameters) const override;
    virtual Flags getEffectiveFlags() const override;

    bool isOpen() const { return m_open; }
    const QByteArray& getIconName() const { return m_iconName; }
    const QString& getState() const { return m_state; }
    const QString& getStateModel() const { return m_stateModel; }

    static QIcon createIcon(QString key, QSize size);

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    static QString getTextForIcon(const QString& key);

    bool m_open = false;
    QByteArray m_iconName;
    QString m_state;
    QString m_stateModel;
};

enum class LinkHighlightMode
{
    None,
    Invert,
    Outline,
    Push
};

/// Link annotation represents hypertext link to a destination to elsewhere
/// in the document, or action to be performed.
class PDFLinkAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFLinkAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Link; }
    virtual std::vector<PDFAppeareanceStreams::Key> getDrawKeys(const PDFFormManager* formManager) const override;
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const PDFAction* getAction() const { return m_action.data(); }
    LinkHighlightMode getHighlightMode() const { return m_highlightMode; }
    const PDFAction* getURIAction() const { return m_previousAction.data(); }
    const PDFAnnotationQuadrilaterals& getActivationRegion() const { return m_activationRegion; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDFActionPtr m_action;
    LinkHighlightMode m_highlightMode = LinkHighlightMode::Invert;
    PDFActionPtr m_previousAction;
    PDFAnnotationQuadrilaterals m_activationRegion;
};

/// Free text annotation displays text directly on the page. Free text doesn't have
/// open/close state, text is always visible.
class PDFFreeTextAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFFreeTextAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::FreeText; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    enum class Justification
    {
        Left,
        Centered,
        Right
    };

    enum class Intent
    {
        None,
        Callout,
        TypeWriter
    };

    const QByteArray& getDefaultAppearance() const { return m_defaultAppearance; }
    Justification getJustification() const { return m_justification; }
    const QString& getDefaultStyle() const { return m_defaultStyleString; }
    const PDFAnnotationCalloutLine& getCalloutLine() const { return m_calloutLine; }
    Intent getIntent() const { return m_intent; }
    const QRectF& getTextRectangle() const { return m_textRectangle; }
    const PDFAnnotationBorderEffect& getBorderEffect() const { return m_effect; }
    AnnotationLineEnding getStartLineEnding() const { return m_startLineEnding; }
    AnnotationLineEnding getEndLineEnding() const { return m_endLineEnding; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QByteArray m_defaultAppearance;
    Justification m_justification = Justification::Left;
    QString m_defaultStyleString;
    PDFAnnotationCalloutLine m_calloutLine;
    Intent m_intent = Intent::None;
    QRectF m_textRectangle;
    PDFAnnotationBorderEffect m_effect;
    AnnotationLineEnding m_startLineEnding = AnnotationLineEnding::None;
    AnnotationLineEnding m_endLineEnding = AnnotationLineEnding::None;
};

/// Line annotation, draws straight line on the page (in most simple form), or
/// it can display, for example, dimensions with perpendicular lines at the line
/// endings. Caption text can also be displayed.
class PDFLineAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFLineAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Line; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    enum class Intent
    {
        Arrow,
        Dimension
    };

    enum class CaptionPosition
    {
        Inline,
        Top
    };

    const QLineF& getLine() const { return m_line; }
    AnnotationLineEnding getStartLineEnding() const { return m_startLineEnding; }
    AnnotationLineEnding getEndLineEnding() const { return m_endLineEnding; }
    const std::vector<PDFReal>& getInteriorColor() const { return m_interiorColor; }
    PDFReal getLeaderLineLength() const { return m_leaderLineLength; }
    PDFReal getLeaderLineExtension() const { return m_leaderLineExtension; }
    PDFReal getLeaderLineOffset() const { return m_leaderLineOffset; }
    bool isCaptionRendered() const { return m_captionRendered; }
    Intent getIntent() const { return m_intent; }
    CaptionPosition getCaptionPosition() const { return m_captionPosition; }
    const PDFObject& getMeasureDictionary() const { return m_measureDictionary; }
    const QPointF& getCaptionOffset() const { return m_captionOffset; }

protected:
    virtual QColor getFillColor() const override;

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QLineF m_line;
    AnnotationLineEnding m_startLineEnding = AnnotationLineEnding::None;
    AnnotationLineEnding m_endLineEnding = AnnotationLineEnding::None;
    std::vector<PDFReal> m_interiorColor;
    PDFReal m_leaderLineLength = 0.0;
    PDFReal m_leaderLineExtension = 0.0;
    PDFReal m_leaderLineOffset = 0.0;
    bool m_captionRendered = false;
    Intent m_intent = Intent::Arrow;
    CaptionPosition m_captionPosition = CaptionPosition::Inline;
    PDFObject m_measureDictionary;
    QPointF m_captionOffset;
};

/// Simple geometry annotation.
/// Square and circle annotations displays rectangle or ellipse on the page.
/// Name is a bit strange (because rectangle may not be a square or circle is not ellipse),
/// but it is defined in PDF specification, so we will use these terms.
class PDFSimpleGeometryAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFSimpleGeometryAnnotation(AnnotationType type) :
        m_type(type)
    {

    }

    virtual AnnotationType getType() const override { return m_type; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const std::vector<PDFReal>& getInteriorColor() const { return m_interiorColor; }
    const PDFAnnotationBorderEffect& getBorderEffect() const { return m_effect; }
    const QRectF& getGeometryRectangle() const { return m_geometryRectangle; }

protected:
    virtual QColor getFillColor() const override;

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    AnnotationType m_type;
    std::vector<PDFReal> m_interiorColor;
    PDFAnnotationBorderEffect m_effect;
    QRectF m_geometryRectangle;
};

/// Polygonal geometry, consists of polygon or polyline geometry. Polygon annotation
/// displays closed polygon (potentially filled), polyline annotation displays
/// polyline, which is not closed.
class PDFPolygonalGeometryAnnotation : public PDFMarkupAnnotation
{
public:
    enum class Intent
    {
        None,
        Cloud,
        Dimension
    };

    inline explicit PDFPolygonalGeometryAnnotation(AnnotationType type) :
        m_type(type),
        m_intent(Intent::None)
    {

    }

    virtual AnnotationType getType() const override { return m_type; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const std::vector<QPointF>& getVertices() const { return m_vertices; }
    AnnotationLineEnding getStartLineEnding() const { return m_startLineEnding; }
    AnnotationLineEnding getEndLineEnding() const { return m_endLineEnding; }
    const std::vector<PDFReal>& getInteriorColor() const { return m_interiorColor; }
    const PDFAnnotationBorderEffect& getBorderEffect() const { return m_effect; }
    Intent getIntent() const { return m_intent; }
    const PDFObject& getMeasure() const { return m_measure; }
    const QPainterPath& getPath() const { return m_path; }

protected:
    virtual QColor getFillColor() const override;

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    AnnotationType m_type;
    std::vector<QPointF> m_vertices;
    AnnotationLineEnding m_startLineEnding = AnnotationLineEnding::None;
    AnnotationLineEnding m_endLineEnding = AnnotationLineEnding::None;
    std::vector<PDFReal> m_interiorColor;
    PDFAnnotationBorderEffect m_effect;
    Intent m_intent;
    PDFObject m_measure;
    QPainterPath m_path;
};

/// Annotation for text highlighting. Can highlight, underline, strikeout,
/// or squiggly underline the text.
class PDFHighlightAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFHighlightAnnotation(AnnotationType type) :
        m_type(type)
    {

    }

    virtual AnnotationType getType() const override { return m_type; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const PDFAnnotationQuadrilaterals& getHiglightArea() const { return m_highlightArea; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    AnnotationType m_type;
    PDFAnnotationQuadrilaterals m_highlightArea;
};

/// Annotation for visual symbol that indicates presence of text edits.
class PDFCaretAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFCaretAnnotation() = default;

    enum class Symbol
    {
        None,
        Paragraph
    };

    virtual AnnotationType getType() const override { return AnnotationType::Caret; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const QRectF& getCaretRectangle() const { return m_caretRectangle; }
    Symbol getSymbol() const { return m_symbol; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QRectF m_caretRectangle;
    Symbol m_symbol = Symbol::None;
};

enum class Stamp
{
    Approved,
    AsIs,
    Confidential,
    Departmental,
    Draft,
    Experimental,
    Expired,
    Final,
    ForComment,
    ForPublicRelease,
    NotApproved,
    NotForPublicRelease,
    Sold,
    TopSecret
};

enum class StampIntent
{
    Stamp,
    StampImage,
    StampSnapshot
};

/// Annotation for stamps. Displays text or graphics intended to look
/// as if they were stamped on the paper.
class PDF4QTLIBCORESHARED_EXPORT PDFStampAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFStampAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Stamp; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    Stamp getStamp() const { return m_stamp; }
    StampIntent getIntent() const { return m_intent; }

    void setStamp(const Stamp& stamp);
    void setIntent(const StampIntent& intent);

    static QString getText(Stamp stamp);

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    Stamp m_stamp = Stamp::Draft;
    StampIntent m_intent = StampIntent::Stamp;
};

/// Ink annotation. Represents a path composed of disjoint polygons.
class PDFInkAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFInkAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Ink; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const QPainterPath& getInkPath() const { return m_inkPath; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QPainterPath m_inkPath;
};

/// Popup annotation. Displays text in popup window for markup annotations.
/// This annotation contains field to associated annotation, for which
/// is window displayed, and window state (open/closed).
class PDFPopupAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFPopupAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Popup; }

    bool isOpened() const { return m_opened; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    bool m_opened = false;
};

enum class FileAttachmentIcon
{
    Graph,
    Paperclip,
    PushPin,
    Tag
};

/// File attachment annotation contains reference to (embedded or external) file.
/// So it is a link to specified file. Activating annotation enables user to view
/// or store attached file in the filesystem.
class PDFFileAttachmentAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFFileAttachmentAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::FileAttachment; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const PDFFileSpecification& getFileSpecification() const { return m_fileSpecification; }
    FileAttachmentIcon getIcon() const { return m_icon; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDFFileSpecification m_fileSpecification;
    FileAttachmentIcon m_icon = FileAttachmentIcon::PushPin;
};

/// Sound annotation contains sound, which is played, when
/// annotation is activated.
class PDFSoundAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFSoundAnnotation() = default;

    enum class Icon
    {
        Speaker,
        Microphone
    };

    virtual AnnotationType getType() const override { return AnnotationType::Sound; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const PDFSound& getSound() const { return m_sound; }
    Icon getIcon() const { return m_icon; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDFSound m_sound;
    Icon m_icon = Icon::Speaker;
};

/// Movie annotation contains movie or sound, which is played, when
/// annotation is activated.
class PDFMovieAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFMovieAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Movie; }

    const QString& getMovieTitle() const { return m_movieTitle; }
    bool isMovieToBePlayed() const { return m_playMovie; }
    const PDFMovie& getMovie() const { return m_movie; }
    const PDFMovieActivation& getMovieActivation() const { return m_movieActivation; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QString m_movieTitle;
    bool m_playMovie = true;
    PDFMovie m_movie;
    PDFMovieActivation m_movieActivation;
};

/// Screen action represents area of page in which is media played.
/// See also Rendition actions and their relationship to this annotation.
class PDFScreenAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFScreenAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Screen; }

    const QString& getScreenTitle() const { return m_screenTitle; }
    const PDFAnnotationAppearanceCharacteristics& getAppearanceCharacteristics() const { return m_appearanceCharacteristics; }
    const PDFAction* getAction() const { return m_action.get(); }
    const PDFAnnotationAdditionalActions& getAdditionalActions() const { return m_additionalActions; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QString m_screenTitle;
    PDFAnnotationAppearanceCharacteristics m_appearanceCharacteristics;
    PDFActionPtr m_action;
    PDFAnnotationAdditionalActions m_additionalActions;
};

/// Widget annotation represents form fields for interactive forms.
/// Annotation's dictionary is merged with form field dictionary,
/// it can be done, because dictionaries doesn't overlap.
class PDFWidgetAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFWidgetAnnotation() = default;

    enum class HighlightMode
    {
        None,
        Invert,
        Outline,
        Push,
        Toggle
    };

    virtual AnnotationType getType() const override { return AnnotationType::Widget; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;
    virtual std::vector<PDFAppeareanceStreams::Key> getDrawKeys(const PDFFormManager* formManager) const override;

    HighlightMode getHighlightMode() const { return m_highlightMode; }
    const PDFAnnotationAppearanceCharacteristics& getAppearanceCharacteristics() const { return m_appearanceCharacteristics; }
    const PDFAction* getAction() const { return m_action.get(); }
    const PDFAnnotationAdditionalActions& getAdditionalActions() const { return m_additionalActions; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    HighlightMode m_highlightMode = HighlightMode::Invert;
    PDFAnnotationAppearanceCharacteristics m_appearanceCharacteristics;
    PDFActionPtr m_action;
    PDFAnnotationAdditionalActions m_additionalActions;
};

/// Printer mark annotation represents graphics symbol, mark, or other
/// graphic feature to assist printing production.
class PDFPrinterMarkAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFPrinterMarkAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::PrinterMark; }
};

/// Trapping characteristics for the page
class PDFTrapNetworkAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFTrapNetworkAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::TrapNet; }
};

/// Watermark annotation represents watermark displayed on the page,
/// for example, if it is printed. Watermarks are displayed at fixed
/// position and size on the page.
class PDFWatermarkAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFWatermarkAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Watermark; }

    const QTransform& getMatrix() const { return m_matrix; }
    PDFReal getRelativeHorizontalOffset() const { return m_relativeHorizontalOffset; }
    PDFReal getRelativeVerticalOffset() const { return m_relativeVerticalOffset; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    QTransform m_matrix;
    PDFReal m_relativeHorizontalOffset = 0.0;
    PDFReal m_relativeVerticalOffset = 0.0;
};

/// Redaction annotation represents content selection, which should
/// be removed from the document.
class PDFRedactAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFRedactAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Redact; }
    virtual void draw(AnnotationDrawParameters& parameters) const override;

    const PDFAnnotationQuadrilaterals& getRedactionRegion() const { return m_redactionRegion; }
    const std::vector<PDFReal>& getInteriorColor() const { return m_interiorColor; }
    const PDFObject& getOverlay() const { return m_overlayForm; }
    const QString& getOverlayText() const { return m_overlayText; }
    bool isRepeat() const { return m_repeat; }
    const QByteArray& getDefaultAppearance() const { return m_defaultAppearance; }
    PDFInteger getJustification() const { return m_justification; }

protected:
    virtual QColor getFillColor() const override;

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDFAnnotationQuadrilaterals m_redactionRegion;
    std::vector<PDFReal> m_interiorColor;
    PDFObject m_overlayForm; ///< Overlay form object
    QString m_overlayText;
    bool m_repeat = false;
    QByteArray m_defaultAppearance;
    PDFInteger m_justification = 0;
};

class PDFProjectionAnnotation : public PDFMarkupAnnotation
{
public:
    inline explicit PDFProjectionAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::Projection; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);
};

/// 3D annotations represents 3D scene, which can be viewed in the application.
class PDF3DAnnotation : public PDFAnnotation
{
public:
    inline explicit PDF3DAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::_3D; }

    const PDF3DStream& getStream() const { return m_stream; }
    const std::optional<PDF3DView>& getDefaultView() const { return m_defaultView; }
    const PDF3DActivation& getActivation() const { return m_activation; }
    bool isInteractive() const { return m_interactive; }
    QRectF getViewBox() const { return m_viewBox; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDF3DStream m_stream;
    std::optional<PDF3DView> m_defaultView;
    PDF3DActivation m_activation;
    bool m_interactive = true;
    QRectF m_viewBox;
};

/// Rich media annotations can be video, audio, or other multimedia presentations.
/// The application should provide additional functionality to control rich media,
/// such as buttons to play/pause/stop video etc. This annotation consists
/// of contents and settings, settings are optional.
class PDFRichMediaAnnotation : public PDFAnnotation
{
public:
    inline explicit PDFRichMediaAnnotation() = default;

    virtual AnnotationType getType() const override { return AnnotationType::RichMedia; }

    const PDFRichMediaContent* getContent() const { return &m_content; }
    const PDFRichMediaSettings* getSettings() const { return &m_settings; }

private:
    friend PDFAnnotationPtr PDFAnnotation::parse(const PDFObjectStorage* storage, PDFObjectReference reference);

    PDFRichMediaContent m_content;
    PDFRichMediaSettings m_settings;
};

/// Annotation manager manages annotations for document's pages. Each page
/// can have multiple annotations, and this object caches them. Also,
/// this object builds annotation's appearance streams, if necessary. This
/// manager is intended to non-gui rendering. If widget annotation manager is used,
/// then this object is not thread safe.
class PDF4QTLIBCORESHARED_EXPORT PDFAnnotationManager : public QObject
{
    Q_OBJECT

private:
    using BaseClass = QObject;

    void drawWidgetAnnotationHighlight(QRectF annotationRectangle, PDFObjectReference annotationReference, QPainter* painter, QTransform userSpaceToDeviceSpace) const;

public:

    enum class Target
    {
        View,
        Print
    };

    explicit PDFAnnotationManager(PDFFontCache* fontCache,
                                  const PDFCMSManager* cmsManager,
                                  const PDFOptionalContentActivity* optionalActivity,
                                  PDFMeshQualitySettings meshQualitySettings,
                                  PDFRenderer::Features features,
                                  Target target,
                                  QObject* parent);
    virtual ~PDFAnnotationManager() override;

    virtual void drawPage(QPainter* painter,
                          PDFInteger pageIndex,
                          const PDFPrecompiledPage* compiledPage,
                          PDFTextLayoutGetter& layoutGetter,
                          const QTransform& pagePointToDevicePointMatrix,
                          QList<PDFRenderError>& errors) const;

    /// Set document
    /// \param document New document
    virtual void setDocument(const PDFModifiedDocument& document);

    Target getTarget() const;
    void setTarget(Target target);

    const PDFOptionalContentActivity* getOptionalActivity() const;
    void setOptionalActivity(const PDFOptionalContentActivity* optionalActivity);

    PDFFontCache* getFontCache() const;
    void setFontCache(PDFFontCache* fontCache);

    PDFMeshQualitySettings getMeshQualitySettings() const;
    void setMeshQualitySettings(const PDFMeshQualitySettings& meshQualitySettings);

    PDFRenderer::Features getFeatures() const;
    void setFeatures(PDFRenderer::Features features);

    PDFFormManager* getFormManager() const;
    void setFormManager(PDFFormManager* formManager);

    struct PageAnnotation
    {
        PDFAppeareanceStreams::Appearance appearance = PDFAppeareanceStreams::Appearance::Normal;
        PDFAnnotationPtr annotation;
        bool isHovered = false;

        /// This mutable appearance stream is protected by main mutex
        mutable PDFCachedItem<PDFObject> appearanceStream;
    };

    struct PDF4QTLIBCORESHARED_EXPORT PageAnnotations
    {
        bool isEmpty() const { return annotations.empty(); }

        /// Returns popup annotation for given annotation, if annotation doesn't have
        /// popup annotation attached, then nullptr is returned.
        /// \param pageAnnotation Page annotation
        /// \returns Popup annotation or nullptr
        const PageAnnotation* getPopupAnnotation(const PageAnnotation& pageAnnotation) const;

        /// Returns a list of page annotations, which are replies to this page annotation.
        /// If page annotation doesn't have any replies, then empty list is returned. Replies
        /// are returned in order, in which they were created.
        /// \param pageAnnotation Page annotation
        /// \returns List of replies
        std::vector<const PageAnnotation*> getReplies(const PageAnnotation& pageAnnotation) const;

        std::vector<PageAnnotation> annotations;
    };

    /// Prepares annotation transformations for rendering
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param device Paint device, onto which will be annotation rendered
    /// \param annotationFlags Annotation flags
    /// \param page Page
    /// \param[in,out] annotationRectangle Input/output annotation rectangle
    QTransform prepareTransformations(const QTransform& pagePointToDevicePointMatrix,
                                      QPaintDevice* device,
                                      const PDFAnnotation::Flags annotationFlags,
                                      const PDFPage* page,
                                      QRectF& annotationRectangle) const;

    /// Returns current appearance stream for given page annotation
    /// \param pageAnnotation Page annotation
    PDFObject getAppearanceStream(const PageAnnotation& pageAnnotation) const;

    /// Returns constant reference to page annotation for given page index.
    /// This function requires, that pointer to m_document is valid.
    /// \param pageIndex Page index (must point to valid page)
    const PageAnnotations& getPageAnnotations(PDFInteger pageIndex) const;

    /// Returns reference to page annotation for given page index.
    /// This function requires, that pointer to m_document is valid.
    /// \param pageIndex Page index (must point to valid page)
    PageAnnotations& getPageAnnotations(PDFInteger pageIndex);

    /// Returns true, if given page has any annotation
    bool hasAnnotation(PDFInteger pageIndex) const;

    /// Returns true, if any page in the given indices has annotation
    bool hasAnyPageAnnotation(const std::vector<PDFInteger>& pageIndices) const;

    /// Returns true, if any page in the document has annotation
    bool hasAnyPageAnnotation() const;

protected:
    void drawWidgetAnnotationHighlight(QRectF annotationRectangle,
                                       const PDFAnnotation* annotation,
                                       QPainter* painter,
                                       QTransform userSpaceToDeviceSpace) const;

    /// Returns true, if given annotation should be drawn
    /// \param annotation Annotation
    bool isAnnotationDrawEnabled(const PageAnnotation& annotation) const;

    /// Returns true, if annotation is drawn by editor (from form widget)
    /// \param annotation Annotation
    bool isAnnotationDrawnByEditor(const PageAnnotation& annotation) const;

    /// Draws annotation
    /// \param pageAnnotation Page annotation
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param page Page
    /// \param cms Color management system
    /// \param isEditorDrawEnabled Is editor draw enabled?
    /// \param errors Errors list (where draw errors are stored)
    /// \param painter Painter
    void drawAnnotation(const PageAnnotation& annotation,
                        const QTransform& pagePointToDevicePointMatrix,
                        const PDFPage* page,
                        const PDFCMS* cms,
                        bool isEditorDrawEnabled,
                        QList<PDFRenderError>& errors,
                        QPainter* painter) const;

    /// Draws annotation by direct drawing, not using annotation's
    /// appearance stream.
    /// \param pageAnnotation Page annotation
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param page Page
    /// \param cms Color management system
    /// \param isEditorDrawEnabled Is annotation drawn by form widget editor?
    /// \param painter Painter
    void drawAnnotationDirect(const PageAnnotation& annotation,
                              const QTransform& pagePointToDevicePointMatrix,
                              const PDFPage* page,
                              const PDFCMS* cms,
                              bool isEditorDrawEnabled,
                              QPainter* painter) const;

    /// Draws annotation using annotation's appearance stream.
    /// \param pageAnnotation Page annotation
    /// \param appearanceStreamObject Object with appearance stream
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param page Page
    /// \param cms Color management system
    /// \param painter Painter
    void drawAnnotationUsingAppearanceStream(const PageAnnotation& annotation,
                                             const PDFObject& appearanceStreamObject,
                                             const QTransform& pagePointToDevicePointMatrix,
                                             const PDFPage* page,
                                             const PDFCMS* cms,
                                             QPainter* painter) const;

    const PDFDocument* m_document;

    PDFFontCache* m_fontCache;
    const PDFCMSManager* m_cmsManager;
    const PDFOptionalContentActivity* m_optionalActivity;
    PDFFormManager* m_formManager;
    PDFMeshQualitySettings m_meshQualitySettings;
    PDFRenderer::Features m_features;

    mutable QMutex m_mutex;
    mutable std::map<PDFInteger, PageAnnotations> m_pageAnnotations;
    Target m_target = Target::View;
};

}   // namespace pdf

#endif // PDFANNOTATION_H
