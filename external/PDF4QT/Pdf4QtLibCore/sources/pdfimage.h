// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PDFIMAGE_H
#define PDFIMAGE_H

#include "pdfobject.h"
#include "pdfcolorspaces.h"
#include "pdfoperationcontrol.h"

#include <QByteArray>

class QByteArray;

namespace pdf
{
class PDFStream;
class PDFDocument;
class PDFObjectStorage;
class PDFRenderErrorReporter;

/// Alternate image object. Defines alternate image, which
/// can be shown in some circumstances instead of main image.
class PDFAlternateImage
{
public:
    explicit inline PDFAlternateImage() = default;

    /// Parses alternate image dictionary object.
    /// \param storage Object storage
    /// \param object Alternate image object
    static PDFAlternateImage parse(const PDFObjectStorage* storage, PDFObject object);

    PDFObjectReference getImage() const { return m_image; }
    PDFObjectReference getOc() const { return m_oc; }
    bool isDefaultForPrinting() const { return m_defaultForPrinting; }

private:
    PDFObjectReference m_image;
    PDFObjectReference m_oc;
    bool m_defaultForPrinting = false;
};

class PDF4QTLIBCORESHARED_EXPORT PDFImage
{
public:
    PDFImage() = default;

    /// Creates image from the content and the dictionary. If image can't be created, then exception is thrown.
    /// \param document Document
    /// \param stream Stream with image
    /// \param colorSpace Color space of the image
    /// \param isSoftMask Is it a soft mask image?
    /// \param renderingIntent Default rendering intent of the image
    /// \param errorReporter Error reporter for reporting errors (or warnings)
    static PDFImage createImage(const PDFDocument* document,
                                const PDFStream* stream,
                                PDFColorSpacePointer colorSpace,
                                bool isSoftMask,
                                RenderingIntent renderingIntent,
                                PDFRenderErrorReporter* errorReporter);

    /// Returns image transformed from image data and color space
    QImage getImage(const PDFCMS* cms,
                    PDFRenderErrorReporter* reporter,
                    const PDFOperationControl* operationControl) const;

    /// Returns rendering intent of the image
    RenderingIntent getRenderingIntent() const { return m_renderingIntent; }

    /// Color space of image samples
    const PDFColorSpacePointer& getColorSpace() const { return m_colorSpace; }

    /// Should PDF processor perform interpolation on this image?
    bool isInterpolated() const { return m_interpolate; }

    /// Array of alternate images
    const std::vector<PDFAlternateImage>& getAlternates() const { return m_alternates; }

    /// Returns name of image, under which is referenced in resources dictionary.
    /// It was mandatory in PDF 1.0, otherwise it is optional and now it is deprecated
    /// in PDF 2.0 specification.
    const QByteArray& getName() const { return m_name; }

    /// Returns idenfitier to structural parent in structure tree
    PDFInteger getStructuralParent() const { return m_structuralParent; }

    /// Web capture content set identifier
    const QByteArray& getWebCaptureContentSetID() const { return m_webCaptureContentSetId; }

    const PDFObject& getOPI() const { return m_OPI; }
    const PDFObject& getOC() const { return m_OC; }
    const PDFObject& getMetadata() const { return m_metadata; }
    const PDFObject& getAssociatedFiles() const { return m_associatedFiles; }
    const PDFObject& getMeasure() const { return m_measure; }
    const PDFObject& getPointData() const { return m_pointData; }

    const PDFImageData& getImageData() const { return m_imageData; }
    const PDFImageData& getSoftMaskData() const { return m_softMask; }

    static bool canBeConvertedToMonochromatic(const QImage& image);

private:
    PDFImageData m_imageData;
    PDFImageData m_softMask;
    PDFColorSpacePointer m_colorSpace;
    RenderingIntent m_renderingIntent = RenderingIntent::Perceptual;
    bool m_interpolate = false;
    std::vector<PDFAlternateImage> m_alternates;
    QByteArray m_name;
    QByteArray m_webCaptureContentSetId;
    PDFInteger m_structuralParent = 0;
    PDFObject m_OPI;
    PDFObject m_OC;
    PDFObject m_metadata;
    PDFObject m_associatedFiles;
    PDFObject m_measure;
    PDFObject m_pointData;
};

}   // namespace pdf

#endif // PDFIMAGE_H
