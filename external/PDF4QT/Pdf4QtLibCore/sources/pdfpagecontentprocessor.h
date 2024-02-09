//    Copyright (C) 2019-2021 Jakub Melka
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

#ifndef PDFPAGECONTENTPROCESSOR_H
#define PDFPAGECONTENTPROCESSOR_H

#include "pdfrenderer.h"
#include "pdfcolorspaces.h"
#include "pdfparser.h"
#include "pdffont.h"
#include "pdfutils.h"
#include "pdfmeshqualitysettings.h"
#include "pdfblendfunction.h"
#include "pdftextlayout.h"
#include "pdfoperationcontrol.h"

#include <QVector>
#include <QTransform>
#include <QPainterPath>
#include <QSharedPointer>

#include <stack>
#include <tuple>
#include <type_traits>

namespace pdf
{
class PDFCMS;
class PDFMesh;
class PDFImage;
class PDFTilingPattern;
class PDFShadingPattern;
class PDFOptionalContentActivity;

static constexpr const char* PDF_RESOURCE_EXTGSTATE = "ExtGState";

class PDFLineDashPattern
{
public:
    explicit inline PDFLineDashPattern() = default;
    explicit PDFLineDashPattern(const std::vector<PDFReal>& dashArray, PDFReal dashOffset);

    inline const std::vector<PDFReal>& getDashArray() const { return m_dashArray; }
    inline void setDashArray(const std::vector<PDFReal>& dashArray) { m_dashArray = dashArray; }

    inline PDFReal getDashOffset() const { return m_dashOffset; }
    inline void setDashOffset(PDFReal dashOffset) { m_dashOffset = dashOffset; }

    inline bool operator==(const PDFLineDashPattern& other) const { return m_dashArray == other.m_dashArray && m_dashOffset == other.m_dashOffset; }
    inline bool operator!=(const PDFLineDashPattern& other) const { return !(*this == other); }

    /// Is line solid? Function returns true, if yes.
    bool isSolid() const { return m_dashArray.empty(); }

    /// Fix line dash pattern according to the specification
    void fix();

    /// Create dash pattern for QPen.
    /// \param penWidthF Width of the pen.
    /// \note Dash pattern in QPen is in pen width units
    QVector<qreal> createForQPen(qreal penWidthF) const;

private:
    std::vector<PDFReal> m_dashArray;
    PDFReal m_dashOffset = 0.0;
};

/// Process the contents of the page.
class PDF4QTLIBCORESHARED_EXPORT PDFPageContentProcessor : public PDFRenderErrorReporter
{
public:
    explicit PDFPageContentProcessor(const PDFPage* page,
                                     const PDFDocument* document,
                                     const PDFFontCache* fontCache,
                                     const PDFCMS* CMS,
                                     const PDFOptionalContentActivity* optionalContentActivity,
                                     QTransform pagePointToDevicePointMatrix,
                                     const PDFMeshQualitySettings& meshQualitySettings);
    virtual ~PDFPageContentProcessor();

    enum class Operator
    {
        // General graphic state        w, J, j, M, d, ri, i, gs
        SetLineWidth,                       ///< w, sets the line width
        SetLineCap,                         ///< J, sets the line cap
        SetLineJoin,                        ///< j, sets the line join
        SetMitterLimit,                     ///< M, sets the mitter limit
        SetLineDashPattern,                 ///< d, sets the line dash pattern
        SetRenderingIntent,                 ///< ri, sets the rendering intent
        SetFlatness,                        ///< i, sets the flattness (number in range from 0 to 100)
        SetGraphicState,                    ///< gs, sets the whole graphic state (stored in resource dictionary)

        // Special graphic state:       q, Q, cm
        SaveGraphicState,                   ///< q, saves the graphic state
        RestoreGraphicState,                ///< Q, restores the graphic state
        AdjustCurrentTransformationMatrix,  ///< cm, modify the current transformation matrix by matrix multiplication

        // Path construction:           m, l, c, v, y, h, re
        MoveCurrentPoint,                   ///< m, begin a new subpath by moving to the desired point
        LineTo,                             ///< l, appends a straight line segment to the subpath
        Bezier123To,                        ///< c, appends a Bézier curve with control points 1, 2, 3
        Bezier23To,                         ///< v, appends a Bézier curve with control points 2, 3
        Bezier13To,                         ///< y, appends a Bézier curve with control points 1, 3
        EndSubpath,                         ///< h, ends current subpath by adding straight line segment from the last point to the beginning
        Rectangle,                          ///< re, adds rectangle

        // Path painting:               S, s, f, F, f*, B, B*, b, b*, n
        PathStroke,                         ///< S, Stroke
        PathCloseStroke,                    ///< s, Close, Stroke (equivalent of operators h S)
        PathFillWinding,                    ///< f, Fill, Winding
        PathFillWinding2,                   ///< F, same as previous, see PDF Reference 1.7, Table 4.10
        PathFillEvenOdd,                    ///< f*, Fill, Even-Odd
        PathFillStrokeWinding,              ///< B, Fill, Stroke, Winding
        PathFillStrokeEvenOdd,              ///< B*, Fill, Stroke, Even-Odd
        PathCloseFillStrokeWinding,         ///< b, Close, Fill, Stroke, Winding (equivalent of operators h B)
        PathCloseFillStrokeEvenOdd,         ///< b*, Close, Fill, Stroke, Even-Odd (equivalent of operators h B*)
        PathClear,                          ///< n, clear path (close current) path, "no-operation", used with clipping

        // Clipping paths:             W, W*
        ClipWinding,                        ///< W, modify current clipping path by intersecting it with current path using "Non zero winding number rule"
        ClipEvenOdd,                        ///< W*, modify current clipping path by intersecting it with current path using "Even-odd rule"

        // Text object:                BT, ET
        TextBegin,                          ///< BT, begin text object, initialize text matrices, cannot be nested
        TextEnd,                            ///< ET, end text object, cannot be nested

        // Text state:                 Tc, Tw, Tz, TL, Tf, Tr, Ts
        TextSetCharacterSpacing,            ///< Tc, set text character spacing
        TextSetWordSpacing,                 ///< Tw, set text word spacing
        TextSetHorizontalScale,             ///< Tz, set text horizontal scaling (in percents, 100% = normal scaling)
        TextSetLeading,                     ///< TL, set text leading
        TextSetFontAndFontSize,             ///< Tf, set text font (name from dictionary) and its size
        TextSetRenderMode,                  ///< Tr, set text render mode
        TextSetRise,                        ///< Ts, set text rise

        // Text positioning:           Td, TD, Tm, T*
        TextMoveByOffset,                   ///< Td, move by offset
        TextSetLeadingAndMoveByOffset,      ///< TD, sets text leading and moves by offset, x y TD is equivalent to sequence -y TL x y Td
        TextSetMatrix,                      ///< Tm, set text matrix
        TextMoveByLeading,                  ///< T*, moves text by leading, equivalent to 0 leading Td

        // Text showing:               Tj, TJ, ', "
        TextShowTextString,                 ///< Tj, show text string
        TextShowTextIndividualSpacing,      ///< TJ, show text, allow individual text spacing
        TextNextLineShowText,               ///< ', move to the next line and show text ("string '" is equivalent to "T* string Tj")
        TextSetSpacingAndShowText,          ///< ", move to the next line, set spacing and show text (equivalent to sequence "w1 Tw w2 Tc string '")

        // Type 3 font:                d0, d1
        Type3FontSetOffset,                 ///< d0, set width information, see PDF 1.7 Reference, Table 5.10
        Type3FontSetOffsetAndBB,            ///< d1, set offset and glyph bounding box

        // Color:                      CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k
        ColorSetStrokingColorSpace,         ///< CS, set current color space for stroking operations
        ColorSetFillingColorSpace,          ///< cs, set current color space for filling operations
        ColorSetStrokingColor,              ///< SC, set current stroking color
        ColorSetStrokingColorN,             ///< SCN, same as SC, but also supports Pattern, Separation, DeviceN and ICCBased color spaces
        ColorSetFillingColor,               ///< sc, set current filling color
        ColorSetFillingColorN,              ///< scn, same as sc, but also supports Pattern, Separation, DeviceN and ICCBased color spaces
        ColorSetDeviceGrayStroking,         ///< G, set DeviceGray color space for stroking color and set color
        ColorSetDeviceGrayFilling,          ///< g, set DeviceGray color space for filling color and set color
        ColorSetDeviceRGBStroking,          ///< RG, set DeviceRGB color space for stroking color and set color
        ColorSetDeviceRGBFilling,           ///< rg, set DeviceRGB color space for filling color and set color
        ColorSetDeviceCMYKStroking,         ///< K, set DeviceCMYK color space for stroking color and set color
        ColorSetDeviceCMYKFilling,          ///< k, set DeviceCMYK color space for filling color and set color

        // Shading pattern:            sh
        ShadingPaintShape,                  ///< sh, paint shape

        // Inline images:              BI, ID, EI
        InlineImageBegin,                   ///< BI, begin inline image
        InlineImageData,                    ///< ID, inline image data
        InlineImageEnd,                     ///< EI, end of inline image

        // XObject:                    Do
        PaintXObject,                       ///< Do, paint the X Object (image, form, ...)

        // Marked content:             MP, DP, BMC, BDC, EMC
        MarkedContentPoint,                 ///< MP, marked content point
        MarkedContentPointWithProperties,   ///< DP, marked content point with properties
        MarkedContentBegin,                 ///< BMC, begin of sequence of marked content
        MarkedContentBeginWithProperties,   ///< BDC, begin of sequence of marked content with properties
        MarkedContentEnd,                   ///< EMC, end of marked content sequence

        // Compatibility:              BX, EX
        CompatibilityBegin,                 ///< BX, Compatibility mode begin (unrecognized operators are ignored)
        CompatibilityEnd,                   ///< EX, Compatibility mode end
        Invalid                             ///< Invalid operator, use for error reporting
    };

    enum ProcedureSet
    {
        EmptyProcSet    = 0x0000,
        NoProcSet       = 0x0001,
        PDF             = 0x0002,
        Text            = 0x0004,
        ImageB          = 0x0008,
        ImageC          = 0x0010,
        ImageI          = 0x0020
    };
    Q_DECLARE_FLAGS(ProcedureSets, ProcedureSet)

    /// Process the contents of the page
    QList<PDFRenderError> processContents();

    virtual void reportRenderError(RenderErrorType type, QString message) override;

    /// Reports render error, but only once - if same error was already reported,
    /// then no new error is reported.
    virtual void reportRenderErrorOnce(RenderErrorType type, QString message) override;

    /// Processes form (XObject of type form)
    /// \param Matrix Transformation matrix from form coordinate system to page coordinate system
    /// \param boundingBox Bounding box, to which is drawed content clipped
    /// \param resources Resources, assigned to the form
    /// \param transparencyGroup Transparency group object
    /// \param content Content stream of the form
    /// \param formStructuralParent Structural parent key for form
    void processForm(const QTransform& matrix,
                     const QRectF& boundingBox,
                     const PDFObject& resources,
                     const PDFObject& transparencyGroup,
                     const QByteArray& content,
                     PDFInteger formStructuralParent);

    /// Initialize stream processor for processing content streams. For example,
    /// graphic state is initialized to default, and default color spaces are initialized.
    void initializeProcessor();

    /// Computes visibility of OCG/OCMD - returns false, if it is not suppressed,
    /// or true, if it is suppressed.
    virtual bool isContentSuppressedByOC(PDFObjectReference ocgOrOcmd);

    /// Sets operation control object which can decide, if operation should
    /// be cancelled. If this is the case, page content processor stops
    /// processing page contents.
    /// \param newOperationControl Operation control object
    void setOperationControl(const PDFOperationControl* newOperationControl);

    /// Returns true, if page content processing is being cancelled
    bool isProcessingCancelled() const;

protected:

    struct PDFTransparencyGroup
    {
        PDFColorSpacePointer colorSpacePointer;
        bool isolated = false;
        bool knockout = false;
    };

    /// Parses transparency group
    PDFTransparencyGroup parseTransparencyGroup(const PDFObject& object);

    /// Soft mask definition
    class PDFSoftMaskDefinition
    {
    public:

        enum class Type
        {
            Invalid,
            Alpha,
            Luminosity
        };

        Type getType() const { return m_type; }
        const PDFStream* getFormStream() const { return m_formStream; }
        const PDFDictionary* getFormDictionary() const { return m_formStream ? m_formStream->getDictionary() : nullptr; }
        const PDFTransparencyGroup& getTransparencyGroup() const { return m_transparencyGroup; }
        const PDFColor& getBackdropColor() const { return m_backdropColor; }
        const PDFFunction* getTransferFunction() const { return m_transferFunction.get(); }

        static PDFSoftMaskDefinition parse(const PDFDictionary* softMask, PDFPageContentProcessor* processor);

    private:
        Type m_type = Type::Invalid;
        const PDFStream* m_formStream = nullptr;
        PDFTransparencyGroup m_transparencyGroup;
        PDFColor m_backdropColor;
        PDFFunctionPtr m_transferFunction;

        friend class PDFPageContentProcessor;
    };

    struct PDFOverprintMode
    {
        bool overprintStroking = false;
        bool overprintFilling = false;
        int overprintMode = 0;

        inline bool operator==(const PDFOverprintMode& other) const
        {
            return std::tie(overprintStroking, overprintFilling, overprintMode) == std::tie(other.overprintStroking, other.overprintFilling, other.overprintMode);
        }
        inline bool operator!=(const PDFOverprintMode& other) const
        {
            return !(*this == other);
        }
    };

    /// Represents graphic state of the PDF (holding current graphic state parameters).
    /// Please see PDF Reference 1.7, Chapter 4.3 "Graphic State"
    class PDFPageContentProcessorState
    {
    public:
        explicit PDFPageContentProcessorState();
        ~PDFPageContentProcessorState();

        PDFPageContentProcessorState(const PDFPageContentProcessorState&) = default;
        PDFPageContentProcessorState(PDFPageContentProcessorState&&) = default;

        PDFPageContentProcessorState& operator=(PDFPageContentProcessorState&&) = delete;
        PDFPageContentProcessorState& operator=(const PDFPageContentProcessorState& other);

        enum StateFlag : uint64_t
        {
            StateUnchanged                      = 0x0000000000000000,
            StateCurrentTransformationMatrix    = 0x0000000000000001,
            StateStrokeColorSpace               = 0x0000000000000002,
            StateFillColorSpace                 = 0x0000000000000004,
            StateStrokeColor                    = 0x0000000000000008,
            StateFillColor                      = 0x0000000000000010,
            StateLineWidth                      = 0x0000000000000020,
            StateLineCapStyle                   = 0x0000000000000040,
            StateLineJoinStyle                  = 0x0000000000000080,
            StateMitterLimit                    = 0x0000000000000100,
            StateLineDashPattern                = 0x0000000000000200,
            StateRenderingIntentName            = 0x0000000000000400,
            StateFlatness                       = 0x0000000000000800,
            StateSmoothness                     = 0x0000000000001000,
            StateTextMatrix                     = 0x0000000000002000,
            StateTextLineMatrix                 = 0x0000000000004000,
            StateTextCharacterSpacing           = 0x0000000000008000,
            StateTextWordSpacing                = 0x0000000000010000,
            StateTextHorizontalScaling          = 0x0000000000020000,
            StateTextLeading                    = 0x0000000000040000,
            StateTextFont                       = 0x0000000000080000,
            StateTextFontSize                   = 0x0000000000100000,
            StateTextRenderingMode              = 0x0000000000200000,
            StateTextRise                       = 0x0000000000400000,
            StateTextKnockout                   = 0x0000000000800000,
            StateAlphaStroking                  = 0x0000000001000000,
            StateAlphaFilling                   = 0x0000000002000000,
            StateBlendMode                      = 0x0000000004000000,
            StateRenderingIntent                = 0x0000000008000000,
            StateOverprint                      = 0x0000000010000000,
            StateAlphaIsShape                   = 0x0000000020000000,
            StateStrokeAdjustment               = 0x0000000040000000,
            StateSoftMask                       = 0x0000000080000000,
            StateBlackPointCompensation         = 0x0000000100000000,
            StateBlackGenerationFunction        = 0x0000000200000000,
            StateUndercolorRemovalFunction      = 0x0000000400000000,
            StateTransferFunction               = 0x0000000800000000,
            StateHalftone                       = 0x0000001000000000,
            StateHalftoneOrigin                 = 0x0000002000000000,
            StateAll                            = 0xFFFFFFFFFFFFFFFF
        };

        using StateFlags = PDFFlags<StateFlag>;

        const QTransform& getCurrentTransformationMatrix() const { return m_currentTransformationMatrix; }
        void setCurrentTransformationMatrix(const QTransform& currentTransformationMatrix);

        const PDFAbstractColorSpace* getStrokeColorSpace() const { return m_strokeColorSpace.data(); }
        void setStrokeColorSpace(const QSharedPointer<PDFAbstractColorSpace>& strokeColorSpace);

        const PDFAbstractColorSpace* getFillColorSpace() const { return m_fillColorSpace.data(); }
        void setFillColorSpace(const QSharedPointer<PDFAbstractColorSpace>& fillColorSpace);

        const QColor& getStrokeColor() const { return m_strokeColor; }
        const PDFColor& getStrokeColorOriginal() const { return m_strokeColorOriginal; }
        void setStrokeColor(const QColor& strokeColor, const PDFColor& originalColor);

        const QColor& getFillColor() const { return m_fillColor; }
        const PDFColor& getFillColorOriginal() const { return m_fillColorOriginal; }
        void setFillColor(const QColor& fillColor, const PDFColor& originalColor);

        PDFReal getLineWidth() const { return m_lineWidth; }
        void setLineWidth(PDFReal lineWidth);

        Qt::PenCapStyle getLineCapStyle() const { return m_lineCapStyle; }
        void setLineCapStyle(Qt::PenCapStyle lineCapStyle);

        Qt::PenJoinStyle getLineJoinStyle() const { return m_lineJoinStyle; }
        void setLineJoinStyle(Qt::PenJoinStyle lineJoinStyle);

        PDFReal getMitterLimit() const { return m_mitterLimit; }
        void setMitterLimit(PDFReal mitterLimit);

        const PDFLineDashPattern& getLineDashPattern() const { return m_lineDashPattern; }
        void setLineDashPattern(PDFLineDashPattern pattern);

        const QByteArray& getRenderingIntentName() const { return m_renderingIntentName; }
        void setRenderingIntentName(const QByteArray& renderingIntentName);

        PDFReal getFlatness() const { return m_flatness; }
        void setFlatness(PDFReal flatness);

        PDFReal getSmoothness() const { return m_smoothness; }
        void setSmoothness(PDFReal smoothness);

        StateFlags getStateFlags() const { return m_stateFlags; }
        void setStateFlags(StateFlags stateFlags) { m_stateFlags = stateFlags; }

        PDFReal getTextCharacterSpacing() const { return m_textCharacterSpacing; }
        void setTextCharacterSpacing(PDFReal textCharacterSpacing);

        PDFReal getTextWordSpacing() const { return m_textWordSpacing; }
        void setTextWordSpacing(PDFReal textWordSpacing);

        PDFReal getTextHorizontalScaling() const { return m_textHorizontalScaling; }
        void setTextHorizontalScaling(PDFReal textHorizontalScaling);

        PDFReal getTextLeading() const { return m_textLeading; }
        void setTextLeading(PDFReal textLeading);

        const PDFFontPointer& getTextFont() const { return m_textFont; }
        void setTextFont(const PDFFontPointer& textFont);

        PDFReal getTextFontSize() const { return m_textFontSize; }
        void setTextFontSize(PDFReal textFontSize);

        TextRenderingMode getTextRenderingMode() const { return m_textRenderingMode; }
        void setTextRenderingMode(TextRenderingMode textRenderingMode);

        PDFReal getTextRise() const { return m_textRise; }
        void setTextRise(PDFReal textRise);

        bool getTextKnockout() const { return m_textKnockout; }
        void setTextKnockout(bool textKnockout);

        const QTransform& getTextMatrix() const { return m_textMatrix; }
        void setTextMatrix(const QTransform& textMatrix);

        const QTransform& getTextLineMatrix() const { return m_textLineMatrix; }
        void setTextLineMatrix(const QTransform& textLineMatrix);

        PDFReal getAlphaStroking() const { return m_alphaStroking; }
        void setAlphaStroking(PDFReal alpha);

        PDFReal getAlphaFilling() const { return m_alphaFilling; }
        void setAlphaFilling(PDFReal alpha);

        BlendMode getBlendMode() const { return m_blendMode; }
        void setBlendMode(BlendMode mode);

        RenderingIntent getRenderingIntent() const { return m_renderingIntent; }
        void setRenderingIntent(RenderingIntent renderingIntent);

        /// Returns stroke color with alpha channel
        QColor getStrokeColorWithAlpha() const;

        /// Returns fill color with alpha channel
        QColor getFillColorWithAlpha() const;

        PDFOverprintMode getOverprintMode() const { return m_overprintMode; }
        void setOverprintMode(PDFOverprintMode overprintMode);

        bool getAlphaIsShape() const { return m_alphaIsShape; }
        void setAlphaIsShape(bool alphaIsShape);

        bool getStrokeAdjustment() const;
        void setStrokeAdjustment(bool strokeAdjustment);

        const PDFDictionary* getSoftMask() const;
        void setSoftMask(const PDFDictionary* softMask);

        BlackPointCompensationMode getBlackPointCompensationMode() const;
        void setBlackPointCompensationMode(BlackPointCompensationMode blackPointCompensationMode);

        PDFObject getBlackGenerationFunction() const;
        void setBlackGenerationFunction(const PDFObject& blackGenerationFunction);

        PDFObject getUndercolorRemovalFunction() const;
        void setUndercolorRemovalFunction(const PDFObject& undercolorRemovalFunction);

        PDFObject getTransferFunction() const;
        void setTransferFunction(const PDFObject& transferFunction);

        PDFObject getHalftone() const;
        void setHalftone(const PDFObject& halftone);

        QPointF getHalftoneOrigin() const;
        void setHalftoneOrigin(const QPointF& halftoneOrigin);

    private:
        QTransform m_currentTransformationMatrix;
        PDFColorSpacePointer m_strokeColorSpace;
        PDFColorSpacePointer m_fillColorSpace;
        QColor m_strokeColor;
        PDFColor m_strokeColorOriginal;
        QColor m_fillColor;
        PDFColor m_fillColorOriginal;
        PDFReal m_lineWidth;
        Qt::PenCapStyle m_lineCapStyle;
        Qt::PenJoinStyle m_lineJoinStyle;
        PDFReal m_mitterLimit;
        PDFLineDashPattern m_lineDashPattern;
        QByteArray m_renderingIntentName;
        PDFReal m_flatness;
        PDFReal m_smoothness;
        PDFReal m_textCharacterSpacing; // T_c
        PDFReal m_textWordSpacing;  // T_w
        PDFReal m_textHorizontalScaling; // T_h, percentage
        PDFReal m_textLeading; // T_l
        PDFFontPointer m_textFont; // Text font
        PDFReal m_textFontSize; // T_fs
        TextRenderingMode m_textRenderingMode; // Text rendering mode
        PDFReal m_textRise; // T_rise
        bool m_textKnockout;
        QTransform m_textMatrix;
        QTransform m_textLineMatrix;
        PDFReal m_alphaStroking;
        PDFReal m_alphaFilling;
        BlendMode m_blendMode;
        RenderingIntent m_renderingIntent;
        PDFOverprintMode m_overprintMode;
        bool m_alphaIsShape;
        bool m_strokeAdjustment;
        const PDFDictionary* m_softMask;
        BlackPointCompensationMode m_blackPointCompensationMode;
        PDFObject m_blackGenerationFunction;
        PDFObject m_undercolorRemovalFunction;
        PDFObject m_transferFunction;
        PDFObject m_halftone;
        QPointF m_halftoneOrigin;
        StateFlags m_stateFlags;
    };

    enum class ProcessOrder
    {
        BeforeOperation,
        AfterOperation
    };

    /// This function has to be implemented in the client drawing implementation, it should
    /// draw the path according to the parameters.
    /// \param path Path, which should be drawn (can be emtpy - in that case nothing happens)
    /// \param stroke Stroke the path
    /// \param fill Fill the path using given rule
    /// \param text Is text being drawn?
    /// \param fillRule Fill rule used in the fill mode
    virtual void performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule);

    /// This function is used, when we want to implement custom fill using shading. If path is successfully
    /// filled by shading, then true should be returned.
    /// \param path Path to be filled
    /// \param stroke Is path actually stroked?
    /// \param fill Is path actually filled?
    /// \param shadingPattern Shading pattern
    virtual bool performPathPaintingUsingShading(const QPainterPath& path, bool stroke, bool fill, const PDFShadingPattern* shadingPattern);

    /// This function is called after path paintig is finished
    virtual void performFinishPathPainting();

    /// This function has to be implemented in the client drawing implementation, it should
    /// clip along the path (intersect with current clipping path).
    virtual void performClipping(const QPainterPath& path, Qt::FillRule fillRule);

    /// Performs image processing on original image. If processor processes
    /// original image, it should return true, so no conversion to QImage occurs,
    /// which can be performance bottleneck.
    /// \param image Image
    /// \returns true, if image is successfully processed
    virtual bool performOriginalImagePainting(const PDFImage& image);

    /// This function has to be implemented in the client drawing implementation, it should
    /// draw the image.
    /// \param image Image to be painted
    virtual void performImagePainting(const QImage& image);

    /// This function has to be implemented in the client drawing implementation, it should
    /// draw the mesh. Mesh is in device space coordinates (so world transformation matrix
    /// is identity matrix).
    /// \param mesh Mesh to be drawn
    virtual void performMeshPainting(const PDFMesh& mesh);

    /// This function has to be implemented in the client drawing implementation, it should
    /// update the device according to the graphic state change. The flags are set when
    /// the value differs from the previous graphic state.
    virtual void performUpdateGraphicsState(const PDFPageContentProcessorState& state);

    /// Implement to perform save of the graphic state. This function is called two times -
    /// before the operation and after the operation. Parameter \p order determines when
    /// this function is called.
    /// \param order If this function is called before the operation, or after the operation.
    virtual void performSaveGraphicState(ProcessOrder order);

    /// Implement to perform restore of the graphic state. This function is called two times -
    /// before the operation and after the operation. Parameter \p order determines when
    /// this function is called.
    /// \param order If this function is called before the operation, or after the operation.
    virtual void performRestoreGraphicState(ProcessOrder order);

    /// Implement to react on marked content point. Properties object can be null, in case
    /// that no properties are provided.
    /// \param tag Tag of the marked content point
    /// \param properties Properties of the marked content point.
    virtual void performMarkedContentPoint(const QByteArray& tag, const PDFObject& properties);

    /// Implement to react on marked content begin.
    /// \param tag Tag of the marked content point
    /// \param properties Properties of the marked content point.
    virtual void performMarkedContentBegin(const QByteArray& tag, const PDFObject& properties);

    /// Implement to react on marked content end
    virtual void performMarkedContentEnd();

    /// Implement to react on set char width request
    virtual void performSetCharWidth(PDFReal wx, PDFReal wy);

    /// Implement to react on set cache device request
    virtual void performSetCacheDevice(PDFReal wx, PDFReal wy, PDFReal llx, PDFReal lly, PDFReal urx, PDFReal ury);

    /// Implement to begin new transparency group
    /// \param Order, in which is function called (before/after setting new transparency group)
    /// \param transparencyGroup Transparency group
    virtual void performBeginTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup);

    /// Implement to end current transparency group
    /// \param Order, in which is function called (before/after setting new transparency group)
    /// \param transparencyGroup Transparency group
    virtual void performEndTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup);

    /// Implement to react on character printing
    virtual void performOutputCharacter(const PDFTextCharacterInfo& info);

    /// Implement to respond to text begin operator
    virtual void performTextBegin(ProcessOrder order);

    /// Implement to respond to text end operator
    virtual void performTextEnd(ProcessOrder order);

    enum class ContentKind
    {
        Shapes,     ///< General shapes (they can be also shaded / tiled)
        Text,       ///< Text outlines (they can be also shaded / tiled)
        Images,     ///< Images
        Shading,    ///< Shading
        Tiling,     ///< Tiling
    };

    /// Override this function to disable particular content type (for example
    /// shading, images, ...)
    virtual bool isContentKindSuppressed(ContentKind kind) const;

    /// Sets current graphic state and updates data
    /// \param state New graphic state
    void setGraphicsState(const PDFPageContentProcessorState& state);

    /// Returns current structural parent key
    PDFInteger getStructuralParentKey() const { return m_structuralParentKey; }

    /// Returns current graphic state
    const PDFPageContentProcessorState* getGraphicState() const { return &m_graphicState; }

    /// Adds error to the error list
    /// \param error Error message
    void addError(const QString& error) { m_errorList.append(PDFRenderError(RenderErrorType::Error, error)); }

    /// Returns true, if graphic content is suppressed
    bool isContentSuppressed() const;

    /// Returns page point to device point matrix
    const QTransform& getPagePointToDevicePointMatrix() const { return m_pagePointToDevicePointMatrix; }

    /// Returns base matrix for patterns
    const QTransform& getPatternBaseMatrix() const { return m_patternBaseMatrix; }

    /// Returns current world matrix (translating actual point to the device point)
    QTransform getCurrentWorldMatrix() const { return getGraphicState()->getCurrentTransformationMatrix() * m_pagePointToDevicePointMatrix; }

    /// Returns page bounding rectangle in device space
    const QRectF& getPageBoundingRectDeviceSpace() const { return m_pageBoundingRectDeviceSpace; }

    /// Returns current procedure sets. Procedure sets are deprecated in PDF 2.0 and are here
    /// only for compatibility purposes. See chapter 14.2 in PDF 2.0 specification.
    ProcedureSets getProcedureSets() const { return m_procedureSets; }

    /// Returns page
    const PDFPage* getPage() const { return m_page; }

    /// Returns document
    const PDFDocument* getDocument() const { return m_document; }

    /// Returns color management system
    const PDFCMS* getCMS() const { return m_CMS; }

    /// Returns font cache
    const PDFFontCache* getFontCache() const { return m_fontCache; }

    /// Returns optional content activity
    const PDFOptionalContentActivity* getOptionalContentActivity() const { return m_optionalContentActivity; }

    class PDF4QTLIBCORESHARED_EXPORT PDFTransparencyGroupGuard
    {
    public:
        explicit PDFTransparencyGroupGuard(PDFPageContentProcessor* processor, PDFTransparencyGroup&& group);
        ~PDFTransparencyGroupGuard();

    private:
        PDFPageContentProcessor* m_processor;
    };

    /// Process form using form stream
    void processForm(const PDFStream* stream);

private:
    /// Initializes the resources dictionaries
    void initDictionaries(const PDFObject& resourcesObject);

    /// Process the content stream
    void processContentStream(const PDFStream* stream);

    /// Process the content
    void processContent(const QByteArray& content);

    /// Processes single command
    void processCommand(const QByteArray& command);

    /// Performs path painting
    /// \param path Path, which should be drawn (can be emtpy - in that case nothing happens)
    /// \param stroke Stroke the path
    /// \param fill Fill the path using given rule
    /// \param text Is text being drawn?
    /// \param fillRule Fill rule used in the fill mode
    void processPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule);

    /// Performs tiling pattern painting
    /// \param tilingPattern Tiling pattern to be painted
    /// \param path Clipping path
    /// \param uncoloredPatternColorSpace Color space for uncolored color patterns
    /// \param uncoloredPatternColor Uncolored color pattern color
    void processTillingPatternPainting(const PDFTilingPattern* tilingPattern,
                                       const QPainterPath& path,
                                       PDFColorSpacePointer uncoloredPatternColorSpace,
                                       PDFColor uncoloredPatternColor);

    /// Applies graphic state dictionary
    /// \param graphicStateDictionary Dictionary to be applied to the current graphic state
    void processApplyGraphicState(const PDFDictionary* graphicStateDictionary);

    enum class MarkedContentKind
    {
        OptionalContent,
        Other
    };

    struct MarkedContentState
    {
        inline explicit MarkedContentState() = default;
        inline explicit MarkedContentState(const QByteArray& tag, MarkedContentKind kind, bool contentSuppressed) :
            tag(tag),
            kind(kind),
            contentSuppressed(contentSuppressed)
        {

        }

        QByteArray tag;
        MarkedContentKind kind = MarkedContentKind::Other;
        bool contentSuppressed = false;
    };

    class PDFPageContentProcessorStateGuard
    {
    public:
        explicit PDFPageContentProcessorStateGuard(PDFPageContentProcessor* processor);
        ~PDFPageContentProcessorStateGuard();

    private:
        PDFPageContentProcessor* m_processor;

        // Stored resources
        const PDFDictionary* m_colorSpaceDictionary;
        const PDFDictionary* m_fontDictionary;
        const PDFDictionary* m_xobjectDictionary;
        const PDFDictionary* m_extendedGraphicStateDictionary;
        const PDFDictionary* m_propertiesDictionary;
        const PDFDictionary* m_shadingDictionary;
        const PDFDictionary* m_patternDictionary;
        ProcedureSets m_procedureSets;
    };

    class PDFPageContentProcessorGraphicStateSaveRestoreGuard
    {
    public:
        inline explicit PDFPageContentProcessorGraphicStateSaveRestoreGuard(PDFPageContentProcessor* processor) :
            m_processor(processor)
        {
            m_processor->operatorSaveGraphicState();
        }
        inline ~PDFPageContentProcessorGraphicStateSaveRestoreGuard()
        {
            m_processor->operatorRestoreGraphicState();
        }

    private:
        PDFPageContentProcessor* m_processor;
    };

    /// Wrapper for PDF Name
    struct PDFOperandName
    {
        QByteArray name;
    };

    /// Wrapper for PDF String
    struct PDFOperandString
    {
        QByteArray string;
    };

    template<typename T>
    T readOperand(size_t index) const;

    template<size_t index, typename T>
    inline T readOperand() const { return readOperand<T>(index); }

    template<typename Tuple, class F, std::size_t... I>
    inline void invokeOperatorImpl(F function, std::index_sequence<I...>)
    {
        (this->*function)(readOperand<I, typename std::tuple_element<I, Tuple>::type>()...);
    }

    /// Function invokes operator (function) with given arguments. For that reason, variadic
    /// templates are used. Basically, for each argument of the function, we need type of the argument,
    /// and its index. To retrieve it, we use std::tuple, variadic template and functionality
    /// analogic to std::apply implementation.
    template<typename... Operands>
    inline void invokeOperator(void(PDFPageContentProcessor::* function)(Operands...))
    {
        invokeOperatorImpl<std::tuple<Operands...>>(function, std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<std::tuple<Operands...>>>>{});
    }

    /// Returns the current poin in the path. If path doesn't exist, then
    /// exception is thrown.
    QPointF getCurrentPoint() const;

    /// Notifies the updated graphic state. If nothing changed in graphic state, then nothing happens.
    void updateGraphicState();

    template<typename... Operands>
    inline QColor getColorFromColorSpace(const PDFAbstractColorSpace* colorSpace, Operands... operands)
    {


        constexpr const size_t operandCount = sizeof...(Operands);
        const size_t colorSpaceComponentCount = colorSpace->getColorComponentCount();
        if (operandCount == colorSpaceComponentCount)
        {
            return colorSpace->getColor(PDFColor(static_cast<PDFColorComponent>(operands)...), m_CMS, m_graphicState.getRenderingIntent(), this, true);
        }
        else
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid color component count. Provided %1, required %2.").arg(operandCount).arg(colorSpaceComponentCount));
        }
    }

    /// Converts PDF line cap to Qt's pen cap style. Function always succeeds,
    /// if invalid \p lineCap occurs, then some valid pen cap style is returned.
    /// \param lineCap PDF Line cap style (see PDF Reference 1.7, values can be 0, 1, and 2)
    static Qt::PenCapStyle convertLineCapToPenCapStyle(PDFInteger lineCap);

    /// Convers Qt's pen cap style to PDF's line cap style (defined in the PDF Reference)
    /// \param penCapStyle Qt's pen cap style to be converted
    static PDFInteger convertPenCapStyleToLineCap(Qt::PenCapStyle penCapStyle);

    /// Converts PDF line join to Qt's pen join style. Function always succeeds,
    /// if invalid \p lineJoin occurs, then some valid pen join style is returned.
    /// \param lineJoin PDF Line join style (see PDF Reference 1.7, values can be 0, 1, and 2)
    static Qt::PenJoinStyle convertLineJoinToPenJoinStyle(PDFInteger lineJoin);

    /// Convers Qt's pen join style to PDF's line join style (defined in the PDF Reference)
    /// \param penJoinStyle Qt's pen join style to be converted
    static PDFInteger convertPenJoinStyleToLineJoin(Qt::PenJoinStyle penJoinStyle);

    // General graphic state        w, J, j, M, d, ri, i, gs
    void operatorSetLineWidth(PDFReal lineWidth);           ///< w, sets the line width
    void operatorSetLineCap(PDFInteger lineCap);            ///< J, sets the line cap
    void operatorSetLineJoin(PDFInteger lineJoin);          ///< j, sets the line join
    void operatorSetMitterLimit(PDFReal mitterLimit);       ///< M, sets the mitter limit
    void operatorSetLineDashPattern();                      ///< d, sets the line dash pattern
    void operatorSetRenderingIntent(PDFOperandName intent);        ///< ri, sets the rendering intent
    void operatorSetFlatness(PDFReal flatness);             ///< i, sets the flattness (number in range from 0 to 100)
    void operatorSetGraphicState(PDFOperandName dictionaryName);   ///< gs, sets the whole graphic state (stored in resource dictionary)

    // Special graphic state:       q, Q, cm
    void operatorSaveGraphicState();                   ///< q, saves the graphic state
    void operatorRestoreGraphicState();                ///< Q, restores the graphic state
    void operatorAdjustCurrentTransformationMatrix(PDFReal a, PDFReal b, PDFReal c, PDFReal d, PDFReal e, PDFReal f);  ///< cm, modify the current transformation matrix by matrix multiplication

    // Path construction operators
    void operatorMoveCurrentPoint(PDFReal x, PDFReal y);
    void operatorLineTo(PDFReal x, PDFReal y);
    void operatorBezier123To(PDFReal x1, PDFReal y1, PDFReal x2, PDFReal y2, PDFReal x3, PDFReal y3);
    void operatorBezier23To(PDFReal x2, PDFReal y2, PDFReal x3, PDFReal y3);
    void operatorBezier13To(PDFReal x1, PDFReal y1, PDFReal x3, PDFReal y3);
    void operatorEndSubpath();
    void operatorRectangle(PDFReal x, PDFReal y, PDFReal width, PDFReal height);

    // Path painting operators
    void operatorPathStroke();
    void operatorPathCloseStroke();
    void operatorPathFillWinding();
    void operatorPathFillEvenOdd();
    void operatorPathFillStrokeWinding();
    void operatorPathFillStrokeEvenOdd();
    void operatorPathCloseFillStrokeWinding();
    void operatorPathCloseFillStrokeEvenOdd();
    void operatorPathClear();

    // Clipping paths:             W, W*
    void operatorClipWinding(); ///< W, modify current clipping path by intersecting it with current path using "Non zero winding number rule"
    void operatorClipEvenOdd(); ///< W*, modify current clipping path by intersecting it with current path using "Even-odd rule"

    // Type 3 font:                d0, d1
    void operatorType3FontSetOffset(PDFReal wx, PDFReal wy);        ///< d0, set width information, see PDF 1.7 Reference, Table 5.10
    void operatorType3FontSetOffsetAndBB(PDFReal wx, PDFReal wy, PDFReal llx, PDFReal lly, PDFReal urx, PDFReal ury);   ///< d1, set offset and glyph bounding box

    // Color:                      CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k
    void operatorColorSetStrokingColorSpace(PDFOperandName name);                           ///< CS, set current color space for stroking operations
    void operatorColorSetFillingColorSpace(PDFOperandName name);                            ///< cs, set current color space for filling operations
    void operatorColorSetStrokingColor();                                                   ///< SC, set current stroking color
    void operatorColorSetStrokingColorN();                                                  ///< SCN, same as SC, but also supports Pattern, Separation, DeviceN and ICCBased color spaces
    void operatorColorSetFillingColor();                                                    ///< sc, set current filling color
    void operatorColorSetFillingColorN();                                                   ///< scn, same as sc, but also supports Pattern, Separation, DeviceN and ICCBased color spaces
    void operatorColorSetDeviceGrayStroking(PDFReal gray);                                  ///< G, set DeviceGray color space for stroking color and set color
    void operatorColorSetDeviceGrayFilling(PDFReal gray);                                   ///< g, set DeviceGray color space for filling color and set color
    void operatorColorSetDeviceRGBStroking(PDFReal r, PDFReal g, PDFReal b);                ///< RG, set DeviceRGB color space for stroking color and set color
    void operatorColorSetDeviceRGBFilling(PDFReal r, PDFReal g, PDFReal b);                 ///< rg, set DeviceRGB color space for filling color and set color
    void operatorColorSetDeviceCMYKStroking(PDFReal c, PDFReal m, PDFReal y, PDFReal k);    ///< K, set DeviceCMYK color space for stroking color and set color
    void operatorColorSetDeviceCMYKFilling(PDFReal c, PDFReal m, PDFReal y, PDFReal k);     ///< k, set DeviceCMYK color space for filling color and set color

    // Text object:                BT, ET
    void operatorTextBegin();                          ///< BT, begin text object, initialize text matrices, cannot be nested
    void operatorTextEnd();                            ///< ET, end text object, cannot be nested

    // Text state:                 Tc, Tw, Tz, TL, Tf, Tr, Ts
    void operatorTextSetCharacterSpacing(PDFReal charSpacing);                  ///< Tc, set text character spacing
    void operatorTextSetWordSpacing(PDFReal wordSpacing);                       ///< Tw, set text word spacing
    void operatorTextSetHorizontalScale(PDFReal horizontalScaling);             ///< Tz, set text horizontal scaling (in percents, 100% = normal scaling)
    void operatorTextSetLeading(PDFReal leading);                               ///< TL, set text leading
    void operatorTextSetFontAndFontSize(PDFOperandName fontName, PDFReal fontSize);    ///< Tf, set text font (name from dictionary) and its size
    void operatorTextSetRenderMode(PDFInteger mode);                            ///< Tr, set text render mode
    void operatorTextSetRise(PDFReal rise);                                     ///< Ts, set text rise

    // Text positioning:           Td, TD, Tm, T*
    void operatorTextMoveByOffset(PDFReal t_x, PDFReal t_y);                                        ///< Td, move by offset
    void operatorTextSetLeadingAndMoveByOffset(PDFReal t_x, PDFReal t_y);                           ///< TD, sets text leading and moves by offset, x y TD is equivalent to sequence -y TL x y Td
    void operatorTextSetMatrix(PDFReal a, PDFReal b, PDFReal c, PDFReal d, PDFReal e, PDFReal f);   ///< Tm, set text matrix
    void operatorTextMoveByLeading();                                                               ///< T*, moves text by leading, equivalent to 0 leading Td

    // Text showing:               Tj, TJ, ', "
    void operatorTextShowTextString(PDFOperandString text);                                     ///< Tj, show text string
    void operatorTextShowTextIndividualSpacing();                                               ///< TJ, show text, allow individual text spacing
    void operatorTextNextLineShowText(PDFOperandString text);                                   ///< ', move to the next line and show text ("string '" is equivalent to "T* string Tj")
    void operatorTextSetSpacingAndShowText(PDFReal t_w, PDFReal t_c, PDFOperandString text);    ///< ", move to the next line, set spacing and show text (equivalent to sequence "w1 Tw w2 Tc string '")

    // Shading pattern:            sh
    void operatorShadingPaintShape(PDFOperandName name);                  ///< sh, paint shape

    // XObject:                    Do
    void operatorPaintXObject(PDFOperandName name); ///< Do, paint the X Object (image, form, ...)

    // Marked content:             MP, DP, BMC, BDC, EMC
    void operatorMarkedContentPoint(PDFOperandName name);                                       ///< MP, marked content point
    void operatorMarkedContentPointWithProperties(PDFOperandName name, PDFObject properties);   ///< DP, marked content point with properties
    void operatorMarkedContentBegin(PDFOperandName name);                                       ///< BMC, begin of sequence of marked content
    void operatorMarkedContentBeginWithProperties(PDFOperandName name, PDFObject properties);   ///< BDC, begin of sequence of marked content with properties
    void operatorMarkedContentEnd();                                                            ///< EMC, end of marked content sequence

    // Compatibility:              BX, EX
    void operatorCompatibilityBegin();   ///< BX, Compatibility mode begin (unrecognized operators are ignored)
    void operatorCompatibilityEnd();     ///< EX, Compatibility mode end

    // Draws the text using the text sequence
    void drawText(const TextSequence& textSequence);

    /// Returns realized font
    const PDFRealizedFontPointer& getRealizedFont() { return m_realizedFont.get(this, &PDFPageContentProcessor::getRealizedFontImpl); }

    /// Returns realized font (or empty font, if font can't be realized)
    PDFRealizedFontPointer getRealizedFontImpl();

    /// Checks, if stroking color is valid
    void checkStrokingColor();

    /// Checks, if filling color is valid
    void checkFillingColor();

    /// Read object from operand stack
    PDFObject readObjectFromOperandStack(size_t startPosition) const;

    /// Implementation of painting of XObject image
    void paintXObjectImage(const PDFStream* stream);

    /// Report warning about color operators in uncolored tiling pattern
    void reportWarningAboutColorOperatorsInUTP();

    /// Set rendering intent by name
    void setRenderingIntentByName(QByteArray renderingIntentName);

    /// Finishes marked content (if end of marked content is missing)
    void finishMarkedContent();

    const PDFPage* m_page;
    const PDFDocument* m_document;
    const PDFFontCache* m_fontCache;
    const PDFCMS* m_CMS;
    const PDFOptionalContentActivity* m_optionalContentActivity;
    const PDFOperationControl* m_operationControl;
    const PDFDictionary* m_colorSpaceDictionary;
    const PDFDictionary* m_fontDictionary;
    const PDFDictionary* m_xobjectDictionary;
    const PDFDictionary* m_extendedGraphicStateDictionary;
    const PDFDictionary* m_propertiesDictionary;
    const PDFDictionary* m_shadingDictionary;
    const PDFDictionary* m_patternDictionary;
    ProcedureSets m_procedureSets;

    // Default color spaces
    PDFColorSpacePointer m_deviceGrayColorSpace;
    PDFColorSpacePointer m_deviceRGBColorSpace;
    PDFColorSpacePointer m_deviceCMYKColorSpace;

    /// Array with current operand arguments
    PDFFlatArray<PDFLexicalAnalyzer::Token, 33> m_operands;

    /// Stack with saved graphic states
    std::stack<PDFPageContentProcessorState> m_stack;

    /// Stack with transparency groups
    std::stack<PDFTransparencyGroup> m_transparencyGroupStack;

    /// Stack with marked content
    std::vector<MarkedContentState> m_markedContentStack;

    /// Current graphic state
    PDFPageContentProcessorState m_graphicState;

    /// List of errors
    QList<PDFRenderError> m_errorList;

    /// Current painter path
    QPainterPath m_currentPath;

    /// Nesting level of the begin/end of text object
    int m_textBeginEndState;

    /// Compatibility level (if positive, then unrecognized operators are ignored)
    int m_compatibilityBeginEndState;

    /// Is drawing uncolored tiling pattern?
    int m_drawingUncoloredTilingPatternState;

    /// Actually realized physical font
    PDFCachedItem<PDFRealizedFontPointer> m_realizedFont;

    /// Actual clipping path obtained from text. Clipping path
    /// is in device space coordinates.
    QPainterPath m_textClippingPath;

    /// Base matrix to be used when drawing patterns. Concatenate this matrix
    /// with pattern matrix to get transformation from pattern space to device space.
    QTransform m_patternBaseMatrix;

    /// Matrix mapping page points to the device points
    QTransform m_pagePointToDevicePointMatrix;

    /// Bounding rectangle of pages media box in device space coordinates. If drawing rotation
    /// is zero, then it corresponds to the scaled media box of the page.
    QRectF m_pageBoundingRectDeviceSpace;

    /// Mesh quality settings
    PDFMeshQualitySettings m_meshQualitySettings;

    /// Set with rendering errors, which were reported (and should be reported once)
    std::set<QString> m_onceReportedErrors;

    /// Active structural parent key
    PDFInteger m_structuralParentKey;
};

template<>
PDFReal PDFPageContentProcessor::readOperand<PDFReal>(size_t index) const;

template<>
PDFInteger PDFPageContentProcessor::readOperand<PDFInteger>(size_t index) const;

template<>
PDFPageContentProcessor::PDFOperandName PDFPageContentProcessor::readOperand<PDFPageContentProcessor::PDFOperandName>(size_t index) const;

template<>
PDFPageContentProcessor::PDFOperandString PDFPageContentProcessor::readOperand<PDFPageContentProcessor::PDFOperandString>(size_t index) const;

}   // namespace pdf

#endif // PDFPAGECONTENTPROCESSOR_H
