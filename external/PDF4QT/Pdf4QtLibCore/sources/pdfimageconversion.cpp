#include "pdfimageconversion.h"
#include "pdfdbgheap.h"

#include <cmath>

namespace pdf
{

PDFImageConversion::PDFImageConversion()
{

}

void PDFImageConversion::setImage(QImage image)
{
    m_image = std::move(image);
    m_convertedImage = QImage();
    m_automaticThreshold = DEFAULT_THRESHOLD;
}

void PDFImageConversion::setConversionMethod(ConversionMethod method)
{
    m_conversionMethod = method;
}

void PDFImageConversion::setThreshold(int threshold)
{
    m_manualThreshold = threshold;
}

bool PDFImageConversion::convert()
{
    if (m_image.isNull())
    {
        return false;
    }

    QImage bitonal(m_image.width(), m_image.height(), QImage::Format_Mono);
    bitonal.fill(0);

    // Thresholding
    int threshold = DEFAULT_THRESHOLD;

    switch (m_conversionMethod)
    {
        case pdf::PDFImageConversion::ConversionMethod::Automatic:
            m_automaticThreshold = calculateOtsu1DThreshold();
            threshold = m_automaticThreshold;
            break;

        case pdf::PDFImageConversion::ConversionMethod::Manual:
            threshold = m_manualThreshold;
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    for (int y = 0; y < m_image.height(); ++y)
    {
        for (int x = 0; x < m_image.width(); ++x)
        {
            QColor pixelColor = m_image.pixelColor(x, y);
            int pixelValue = pixelColor.lightness();
            bool bit = (pixelValue >= threshold);
            bitonal.setPixel(x, y, bit);
        }
    }

    m_convertedImage = std::move(bitonal);
    return true;
}

int PDFImageConversion::getThreshold() const
{
    switch (m_conversionMethod)
    {
        case pdf::PDFImageConversion::ConversionMethod::Automatic:
            return m_automaticThreshold;

        case pdf::PDFImageConversion::ConversionMethod::Manual:
            return m_manualThreshold;

        default:
            Q_ASSERT(false);
            break;
    }

    return DEFAULT_THRESHOLD;
}

QImage PDFImageConversion::getConvertedImage() const
{
    return m_convertedImage;
}

int PDFImageConversion::calculateOtsu1DThreshold() const
{
    if (m_image.isNull())
    {
        return 128;
    }

    // Histogram of lightness occurences
    std::array<int, 256> histogram = { };

    for (int x = 0; x < m_image.width(); ++x)
    {
        for (int y = 0; y < m_image.height(); ++y)
        {
            int lightness = m_image.pixelColor(x, y).lightness();
            Q_ASSERT(lightness >= 0 && lightness <= 255);

            int clampedLightness = qBound(0, lightness, 255);
            histogram[clampedLightness] += 1;
        }
    }

    float factor = 1.0f / float(m_image.width() * m_image.height());

    std::array<float, 256> normalizedHistogram = { };
    std::array<float, 256> cumulativeProbabilities = { };
    std::array<float, 256> interClassVariance = { };

    // Compute probabilities
    for (size_t i = 0; i < histogram.size(); ++i)
    {
        normalizedHistogram[i] = histogram[i] * factor;
        cumulativeProbabilities[i] = normalizedHistogram[i];

        if (i > 0)
        {
            cumulativeProbabilities[i] += cumulativeProbabilities[i - 1];
        }
    }

    // Calculate the inter-class variance for each threshold. Variables
    // with the subscript 0 denote the background, while those with
    // subscript 1 denote the foreground.
    for (size_t i = 0; i < histogram.size(); ++i)
    {
        const float w0 = cumulativeProbabilities[i] - normalizedHistogram[i];
        const float w1 = 1.0f - w0;

        float u0 = 0.0f;
        float u1 = 0.0f;

        // Calculate mean intensity value of the background.
        if (!qFuzzyIsNull(w0))
        {
            for (size_t j = 0; j < i; ++j)
            {
                u0 += j * normalizedHistogram[j];
            }

            u0 /= w0;
        }

        // Calculate mean intensity value of the foreground.
        if (!qFuzzyIsNull(w1))
        {
            for (size_t j = i; j < histogram.size(); ++j)
            {
                u1 += j * normalizedHistogram[j];
            }

            u1 /= w1;
        }

        const float variance = w0 * w1 * std::pow(u0 - u1, 2);
        interClassVariance[i] = variance;
    }

    // Find maximal value of the variance
    size_t maxVarianceIndex = 0;
    float maxVarianceValue = 0.0f;

    for (size_t i = 0; i < interClassVariance.size(); ++i)
    {
        if (interClassVariance[i] > maxVarianceValue)
        {
            maxVarianceValue = interClassVariance[i];
            maxVarianceIndex = i;
        }
    }

    return int(maxVarianceIndex);
}

}   // namespace pdf
