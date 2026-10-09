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

#include "pdfimagecompressor.h"

#include "pdfcatalog.h"
#include "pdfcms.h"
#include "pdfconstants.h"
#include "pdfdocument.h"
#include "pdfimage.h"
#include "pdfmeshqualitysettings.h"
#include "pdfoptionalcontent.h"
#include "pdfpage.h"
#include "pdfpagecontentprocessor.h"
#include "pdffont.h"

#include <QScopeGuard>

#include <cmath>
#include <limits>
#include <map>

namespace pdf
{
namespace
{
static double updateAxisDpi(double currentValue, double candidate)
{
    if (candidate <= 0.0 || !std::isfinite(candidate))
    {
        return currentValue;
    }

    if (!std::isfinite(currentValue) || candidate < currentValue)
    {
        return candidate;
    }

    return currentValue;
}

class PDFImageCollectorProcessor : public PDFPageContentProcessor
{
public:
    PDFImageCollectorProcessor(const PDFPage* page,
                               const PDFDocument* document,
                               const PDFFontCache* fontCache,
                               const PDFCMS* cms,
                               const PDFOptionalContentActivity* optionalContentActivity,
                               const PDFMeshQualitySettings& meshQualitySettings,
                               std::map<PDFObjectReference, PDFImageCompressor::ImageStatistics>* statistics) :
        PDFPageContentProcessor(page, document, fontCache, cms, optionalContentActivity, QTransform(), meshQualitySettings),
        m_statistics(statistics)
    {
    }

protected:
    bool isContentKindSuppressed(ContentKind kind) const override
    {
        switch (kind)
        {
            case ContentKind::Images:
            case ContentKind::Tiling:
            case ContentKind::Forms:
                return false;

            default:
                return true;
        }
    }

    bool performOriginalImagePainting(const PDFImage& image, const PDFStream* stream, PDFObjectReference reference) override
    {
        if (isContentSuppressed())
        {
            return true;
        }

        Q_UNUSED(stream);

        if (!reference.isValid())
        {
            // Inline images or direct streams without reference are skipped, as they can't be shared
            return true;
        }

        updateStatistics(image, reference);
        return true;
    }

private:
    QPointF calculateDpi(const PDFImage& image) const
    {
        const QTransform ctm = getGraphicState()->getCurrentTransformationMatrix();

        const auto axisLength = [](qreal x, qreal y)
        {
            const double length = std::hypot(static_cast<double>(x), static_cast<double>(y));
            return length * PDF_POINT_TO_INCH;
        };

        QPointF dpi(0.0, 0.0);

        const double widthInches = axisLength(ctm.m11(), ctm.m12());
        if (widthInches > std::numeric_limits<double>::epsilon())
        {
            dpi.setX(static_cast<double>(image.getImageData().getWidth()) / widthInches);
        }

        const double heightInches = axisLength(ctm.m21(), ctm.m22());
        if (heightInches > std::numeric_limits<double>::epsilon())
        {
            dpi.setY(static_cast<double>(image.getImageData().getHeight()) / heightInches);
        }

        return dpi;
    }

    void updateStatistics(const PDFImage& image, PDFObjectReference reference)
    {
        Q_ASSERT(m_statistics);

        auto [iterator, inserted] = m_statistics->try_emplace(reference);
        PDFImageCompressor::ImageStatistics& stats = iterator->second;

        if (inserted)
        {
            stats.reference = reference;
        }

        if (stats.image.isNull())
        {
            stats.image = image.getImage(getCMS(), this, nullptr);
        }

        const QPointF dpi = calculateDpi(image);
        stats.minimalDpi.setX(updateAxisDpi(stats.minimalDpi.x(), dpi.x()));
        stats.minimalDpi.setY(updateAxisDpi(stats.minimalDpi.y(), dpi.y()));
    }

    std::map<PDFObjectReference, PDFImageCompressor::ImageStatistics>* m_statistics = nullptr;
};

}   // namespace

PDFImageCompressor::ImageStatisticsList PDFImageCompressor::collectImages(const PDFDocument* document) const
{
    ImageStatisticsList result;
    if (!document)
    {
        return result;
    }

    const PDFCatalog* catalog = document->getCatalog();
    if (!catalog)
    {
        return result;
    }

    PDFOptionalContentActivity optionalContentActivity(document, OCUsage::Export, nullptr);
    PDFCMSManager cmsManager(nullptr);
    cmsManager.setDocument(document);
    cmsManager.setSettings(cmsManager.getDefaultSettings());
    PDFCMSPointer cms = cmsManager.getCurrentCMS();

    PDFFontCache fontCache(DEFAULT_FONT_CACHE_LIMIT, DEFAULT_REALIZED_FONT_CACHE_LIMIT);
    PDFModifiedDocument modifiedDocument(const_cast<PDFDocument*>(document), &optionalContentActivity);
    fontCache.setDocument(modifiedDocument);
    fontCache.setCacheShrinkEnabled(nullptr, false);
    auto fontCacheGuard = qScopeGuard([&fontCache]() { fontCache.setCacheShrinkEnabled(nullptr, true); });

    PDFMeshQualitySettings meshQualitySettings;
    std::map<PDFObjectReference, ImageStatistics> statistics;

    const size_t pageCount = catalog->getPageCount();
    for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
    {
        const PDFPage* page = catalog->getPage(pageIndex);
        if (!page)
        {
            continue;
        }

        PDFImageCollectorProcessor processor(page,
                                             document,
                                             &fontCache,
                                             cms.data(),
                                             &optionalContentActivity,
                                             meshQualitySettings,
                                             &statistics);
        processor.processContents();
    }

    result.reserve(statistics.size());
    for (auto& [reference, stats] : statistics)
    {
        if (!std::isfinite(stats.minimalDpi.x()))
        {
            stats.minimalDpi.setX(0.0);
        }
        if (!std::isfinite(stats.minimalDpi.y()))
        {
            stats.minimalDpi.setY(0.0);
        }
        result.push_back(std::move(stats));
    }

    return result;
}

}   // namespace pdf
