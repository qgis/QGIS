//    Copyright (C) 2019-2022 Jakub Melka
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

#include "pdffunction.h"
#include "pdfflatarray.h"
#include "pdfparser.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfutils.h"

#include <QIODevice>
#include <QtMath>

#include "pdfdbgheap.h"

#include <stack>
#include <iterator>
#include <type_traits>

namespace pdf
{

PDFFunction::PDFFunction(uint32_t m, uint32_t n, std::vector<PDFReal>&& domain, std::vector<PDFReal>&& range) :
    m_m(m),
    m_n(n),
    m_domain(std::move(domain)),
    m_range(std::move(range))
{

}

PDFFunctionPtr PDFFunction::createFunction(const PDFDocument* document, const PDFObject& object)
{
    PDFParsingContext context(nullptr);
    return createFunctionImpl(document, object, &context);
}

PDFFunctionPtr PDFFunction::createFunctionImpl(const PDFDocument* document, const PDFObject& object, PDFParsingContext* context)
{
    PDFParsingContext::PDFParsingContextObjectGuard guard(context, &object);

    const PDFDictionary* dictionary = nullptr;
    QByteArray streamData;

    const PDFObject& dereferencedObject = document->getObject(object);
    if (dereferencedObject.isName() && dereferencedObject.getString() == "Identity")
    {
        return std::make_shared<PDFIdentityFunction>();
    }
    else if (dereferencedObject.isDictionary())
    {
        dictionary = dereferencedObject.getDictionary();
    }
    else if (dereferencedObject.isStream())
    {
        const PDFStream* stream = dereferencedObject.getStream();
        dictionary = stream->getDictionary();
        streamData = document->getDecodedStream(stream);
    }

    if (!dictionary)
    {
        throw PDFException(PDFParsingContext::tr("Function dictionary expected."));
    }

    PDFDocumentDataLoaderDecorator loader(document);
    const PDFInteger functionType = loader.readIntegerFromDictionary(dictionary, "FunctionType", -1);
    std::vector<PDFReal> domain = loader.readNumberArrayFromDictionary(dictionary, "Domain");
    std::vector<PDFReal> range = loader.readNumberArrayFromDictionary(dictionary, "Range");

    // Domain is required for all function
    if (domain.empty())
    {
        throw PDFException(PDFParsingContext::tr("Fuction has invalid domain."));
    }

    if ((functionType == 0 || functionType == 4) && range.empty())
    {
        throw PDFException(PDFParsingContext::tr("Fuction has invalid range."));
    }

    switch (functionType)
    {
        case 0:
        {
            // Sampled function
            std::vector<PDFInteger> size = loader.readIntegerArrayFromDictionary(dictionary, "Size");
            const size_t bitsPerSample = loader.readIntegerFromDictionary(dictionary, "BitsPerSample", 0);
            std::vector<PDFReal> encode = loader.readNumberArrayFromDictionary(dictionary, "Encode");
            std::vector<PDFReal> decode = loader.readNumberArrayFromDictionary(dictionary, "Decode");

            if (size.empty() || !std::all_of(size.cbegin(), size.cend(), [](PDFInteger size) { return size >= 1; }))
            {
                throw PDFException(PDFParsingContext::tr("Sampled function has invalid sample size."));
            }

            if (bitsPerSample < 1 || bitsPerSample > 32)
            {
                throw PDFException(PDFParsingContext::tr("Sampled function has invalid count of bits per sample."));
            }

            if (encode.empty())
            {
                // Construct default array according to the PDF 1.7 specification
                encode.resize(2 * size.size(), 0);
                for (size_t i = 0, count = size.size(); i < count; ++i)
                {
                    encode[2 * i + 1] = size[i] - 1;
                }
            }

            if (decode.empty())
            {
                // Default decode array is same as range, see PDF 1.7 specification
                decode = range;
            }

            const size_t m = size.size();
            const size_t n = range.size() / 2;

            if (n == 0)
            {
                throw PDFException(PDFParsingContext::tr("Sampled function hasn't any output."));
            }

            if (domain.size() != encode.size())
            {
                throw PDFException(PDFParsingContext::tr("Sampled function has invalid encode array."));
            }

            if (range.size() != decode.size())
            {
                throw PDFException(PDFParsingContext::tr("Sampled function has invalid decode array."));
            }

            const uint64_t sampleMaxValueInteger = (static_cast<uint64_t>(1) << static_cast<uint64_t>(bitsPerSample)) - 1;
            const PDFReal sampleMaxValue = sampleMaxValueInteger;

            // Load samples - first see, how much of them will be needed.
            const PDFInteger sampleCount = std::accumulate(size.cbegin(), size.cend(), 1, [](PDFInteger a, PDFInteger b) { return a * b; } ) * n;
            std::vector<PDFReal> samples;
            samples.resize(sampleCount, 0.0);

            // We must use 64 bit, because we can have 32 bit values
            uint64_t buffer = 0;
            uint64_t bitsWritten = 0;
            uint64_t bitMask = sampleMaxValueInteger;

            QDataStream reader(&streamData, QIODevice::ReadOnly);
            for (PDFReal& sample : samples)
            {
                while (bitsWritten < bitsPerSample)
                {
                    if (!reader.atEnd())
                    {
                        uint8_t currentByte = 0;
                        reader >> currentByte;
                        buffer = (buffer << 8) | currentByte;
                        bitsWritten += 8;
                    }
                    else
                    {
                        throw PDFException(PDFParsingContext::tr("Not enough samples for sampled function."));
                    }
                }

                // Now we have enough bits to read the data
                uint64_t sampleUint = (buffer >> (bitsWritten - bitsPerSample)) & bitMask;
                bitsWritten -= bitsPerSample;
                sample = sampleUint;
            }

            std::vector<uint32_t> sizeAsUint;
            std::transform(size.cbegin(), size.cend(), std::back_inserter(sizeAsUint), [](PDFInteger integer) { return static_cast<uint32_t>(integer); });

            return std::make_shared<PDFSampledFunction>(static_cast<uint32_t>(m), static_cast<uint32_t>(n), std::move(domain), std::move(range), std::move(sizeAsUint), std::move(samples), std::move(encode), std::move(decode), sampleMaxValue, loader.readIntegerFromDictionary(dictionary, "Order", 1));
        }
        case 2:
        {
            // Exponential function
            std::vector<PDFReal> c0 = loader.readNumberArrayFromDictionary(dictionary, "C0");
            std::vector<PDFReal> c1 = loader.readNumberArrayFromDictionary(dictionary, "C1");
            const PDFReal exponent = loader.readNumberFromDictionary(dictionary, "N", 1.0);

            if (domain.size() != 2)
            {
                throw PDFException(PDFParsingContext::tr("Exponential function can have only one input value."));
            }

            if (exponent < 0.0 && domain[0] <= 0.0)
            {
                throw PDFException(PDFParsingContext::tr("Invalid domain of exponential function."));
            }

            if (!qFuzzyIsNull(std::fmod(exponent, 1.0)) && domain[0] < 0.0)
            {
                throw PDFException(PDFParsingContext::tr("Invalid domain of exponential function."));
            }

            constexpr uint32_t m = 1;

            // Determine n.
            uint32_t n = static_cast<uint32_t>(std::max({ static_cast<size_t>(1), range.size() / 2, c0.size(), c1.size() }));

            // Resolve default values
            if (c0.empty())
            {
                c0.resize(n, 0.0);
            }
            if (c1.empty())
            {
                c1.resize(n, 1.0);
            }

            if (c0.size() != n)
            {
                throw PDFException(PDFParsingContext::tr("Invalid parameter of exponential function (at x = 0.0)."));
            }
            if (c1.size() != n)
            {
                throw PDFException(PDFParsingContext::tr("Invalid parameter of exponential function (at x = 1.0)."));
            }

            return std::make_shared<PDFExponentialFunction>(m, n, std::move(domain), std::move(range), std::move(c0), std::move(c1), exponent);
        }
        case 3:
        {
            // Stitching function
            std::vector<PDFReal> bounds = loader.readNumberArrayFromDictionary(dictionary, "Bounds");
            std::vector<PDFReal> encode = loader.readNumberArrayFromDictionary(dictionary, "Encode");

            if (domain.size() != 2)
            {
                throw PDFException(PDFParsingContext::tr("Stitching function can have only one input value."));
            }

            if (dictionary->hasKey("Functions"))
            {
                const PDFObject& functions = document->getObject(dictionary->get("Functions"));
                if (functions.isArray())
                {
                    const PDFArray* array = functions.getArray();
                    if (array->getCount() != bounds.size() + 1)
                    {
                        throw PDFException(PDFParsingContext::tr("Stitching function has different function count. Expected %1, actual %2.").arg(array->getCount()).arg(bounds.size() + 1));
                    }

                    std::vector<PDFStitchingFunction::PartialFunction> partialFunctions;
                    partialFunctions.resize(array->getCount());

                    if (encode.size() != partialFunctions.size() * 2)
                    {
                        throw PDFException(PDFParsingContext::tr("Stitching function has invalid encode array. Expected %1 items, actual %2.").arg(partialFunctions.size() * 2).arg(encode.size()));
                    }

                    std::vector<PDFReal> boundsAdjusted;
                    boundsAdjusted.resize(bounds.size() + 2);
                    boundsAdjusted.front() = domain.front();
                    boundsAdjusted.back() = domain.back();
                    std::copy(bounds.cbegin(), bounds.cend(), std::next(boundsAdjusted.begin()));

                    Q_ASSERT(boundsAdjusted.size() == partialFunctions.size() + 1);

                    uint32_t n = 0;
                    for (size_t i = 0; i < partialFunctions.size(); ++i)
                    {
                        PDFStitchingFunction::PartialFunction& partialFunction = partialFunctions[i];
                        partialFunction.function = createFunctionImpl(document, array->getItem(i), context);
                        partialFunction.bound0 = boundsAdjusted[i];
                        partialFunction.bound1 = boundsAdjusted[i + 1];
                        partialFunction.encode0 = encode[2 * i];
                        partialFunction.encode1 = encode[2 * i + 1];

                        const uint32_t nLocal = partialFunction.function->getOutputVariableCount();

                        if (n == 0)
                        {
                            n = nLocal;
                        }
                        else if (n != nLocal)
                        {
                            throw PDFException(PDFParsingContext::tr("Functions in stitching function has different number of output variables."));
                        }
                    }

                    return std::make_shared<PDFStitchingFunction>(1, n, std::move(domain), std::move(range), std::move(partialFunctions));
                }
                else
                {
                    throw PDFException(PDFParsingContext::tr("Stitching function has invalid functions."));
                }
            }
            else
            {
                throw PDFException(PDFParsingContext::tr("Stitching function hasn't functions array."));
            }
        }
        case 4:
        {
            // Postscript function
            PDFPostScriptFunction::Program program = PDFPostScriptFunction::parseProgram(streamData);

            const uint32_t m = static_cast<uint32_t>(domain.size()) / 2;
            const uint32_t n = static_cast<uint32_t>(range.size()) / 2;

            if (program.empty())
            {
                throw PDFException(PDFParsingContext::tr("Empty program in PostScript function."));
            }

            return std::make_shared<PDFPostScriptFunction>(m, n, std::move(domain), std::move(range), std::move(program));
        }

        default:
        {
            throw PDFException(PDFParsingContext::tr("Invalid function type: %1.").arg(functionType));
        }
    }
}

PDFSampledFunction::PDFSampledFunction(uint32_t m, uint32_t n,
                                       std::vector<PDFReal>&& domain,
                                       std::vector<PDFReal>&& range,
                                       std::vector<uint32_t>&& size,
                                       std::vector<PDFReal>&& samples,
                                       std::vector<PDFReal>&& encoder,
                                       std::vector<PDFReal>&& decoder,
                                       PDFReal sampleMaximalValue,
                                       PDFInteger order) :
    PDFFunction(m, n, std::move(domain), std::move(range)),
    m_hypercubeNodeCount(1 << m_m),
    m_size(std::move(size)),
    m_samples(std::move(samples)),
    m_encoder(std::move(encoder)),
    m_decoder(std::move(decoder)),
    m_sampleMaximalValue(sampleMaximalValue),
    m_order(order)
{
    // Asserts, that we get sane input
    Q_ASSERT(m > 0);
    Q_ASSERT(n > 0);
    Q_ASSERT(m_size.size() == m);
    Q_ASSERT(m_domain.size() == 2 * m);
    Q_ASSERT(m_range.size() == 2 * n);
    Q_ASSERT(m_domain.size() == m_encoder.size());
    Q_ASSERT(m_range.size() == m_decoder.size());

    m_hypercubeNodeOffsets.resize(m_hypercubeNodeCount, 0);

    const uint32_t lastInputVariableIndex = m_m - 1;

    // Calculate hypercube offsets. Offsets are indexed in bits, from the lowest
    // bit to the highest. We assume, that we do not have more, than 32 input
    // variables (we probably run out of memory in that time). Example:
    //
    // We have m = 3, f(x_0, x_1, x_2) is sampled function of 3 variables, n = 1.
    // We have 2, 4, 6 samples for x_0, x_1 and x_2 (so sample count differs).
    // Then the i-th bit corresponds to variable x_i. We will have m_hypercubeNodeCount == 8,
    // hypercube offset indices are from 0 to 7.
    //      m_hypercubeNodeOffsets[0] =  0;     - f(0, 0, 0)
    //		m_hypercubeNodeOffsets[1] =  1;     - f(1, 0, 0)
    //		m_hypercubeNodeOffsets[2] =  2;     - f(0, 1, 0)
    //		m_hypercubeNodeOffsets[3] =  3;     - f(1, 1, 0)
    //		m_hypercubeNodeOffsets[4] =  8;     - f(0, 0, 1)    2 * 4 = 8
    //		m_hypercubeNodeOffsets[5] =  9;     - f(1, 0, 1)    2 * 4 + 1 (for x_1 = 1, x_2 = 0) = 8
    //		m_hypercubeNodeOffsets[6] = 10;     - f(0, 1, 1)    2 * 4 + 2 (for x_1 = 0, x_2 = 1) = 9
    //		m_hypercubeNodeOffsets[7] = 11;     - f(1, 1, 1)    2 * 4 + 2 + 1 = 11
    for (uint32_t i = 0; i < m_hypercubeNodeCount; ++i)
    {
        uint32_t index = 0;
        uint32_t mask = i;
        for (uint32_t j = lastInputVariableIndex; j > 0; --j)
        {
            uint32_t bit = 0;
            if (m_size[j] > 1)
            {
                // We shift mask, so we are accessing bits from highest to lowest in reverse order
                bit = (mask >> lastInputVariableIndex) & static_cast<uint32_t>(1);
            }

            index = (index + bit) * m_size[j - 1];
            mask = mask << 1;
        }

        uint32_t lastBit = 0;
        if (m_size[0] > 1)
        {
            lastBit = (mask >> lastInputVariableIndex) & static_cast<uint32_t>(1);
        }

        m_hypercubeNodeOffsets[i] = (index + lastBit) * m_n;
    }
}

PDFFunction::FunctionResult PDFSampledFunction::apply(const_iterator x_1,
                                                      const_iterator x_m,
                                                      iterator y_1,
                                                      iterator y_n) const
{
    const size_t m = std::distance(x_1, x_m);
    const size_t n = std::distance(y_1, y_n);

    if (m != m_m)
    {
        return PDFTranslationContext::tr("Invalid number of operands for function. Expected %1, provided %2.").arg(m_m).arg(m);
    }
    if (n != m_n)
    {
        return PDFTranslationContext::tr("Invalid number of output variables for function. Expected %1, provided %2.").arg(m_n).arg(n);
    }

    PDFFlatArray<uint32_t, DEFAULT_OPERAND_COUNT> encoded;
    PDFFlatArray<PDFReal, DEFAULT_OPERAND_COUNT> encoded0;
    PDFFlatArray<PDFReal, DEFAULT_OPERAND_COUNT> encoded1;

    for (uint32_t i = 0; i < m_m; ++i)
    {
        const PDFReal x = *std::next(x_1, i);

        // First clamp it in the function domain
        const PDFReal xClamped = clampInput(i, x);
        const PDFReal xEncoded = interpolate(xClamped, m_domain[2 * i], m_domain[2 * i + 1], m_encoder[2 * i], m_encoder[2 * i + 1]);
        const PDFReal xClampedToSamples = qBound<PDFReal>(0, xEncoded, m_size[i]);

        uint32_t xRounded = static_cast<uint32_t>(xClampedToSamples);
        if (xRounded == m_size[i] && m_size[i] > 1)
        {
            // We want one value before the end (so we can use the "hypercube" algorithm)
            xRounded = m_size[i] - 2;
        }

        const PDFReal x1 = xClampedToSamples - static_cast<PDFReal>(xRounded);
        const PDFReal x0 = 1.0 - x1;
        encoded.push_back(xRounded);
        encoded0.push_back(x0);
        encoded1.push_back(x1);
    }

    // Index (offset) for hypercube node (0, 0, ..., 0)
    uint32_t baseOffset = 0;
    for (uint32_t i = m_m - 1; i > 0; --i)
    {
        baseOffset = (baseOffset + encoded[i]) * m_size[i - 1];
    }
    baseOffset = (baseOffset + encoded[0]) * m_n;

    // Samples for hypercube nodes (for each hypercube node, single
    // sample is fetched). Of course, size of this array is 2^m, so
    // it can be very huge.
    PDFFlatArray<PDFReal, DEFAULT_OPERAND_COUNT> hyperCubeSamples;
    hyperCubeSamples.resize(m_hypercubeNodeCount);

    for (uint32_t outputIndex = 0; outputIndex < m_n; ++outputIndex)
    {
        // Load samples into hypercube
        for (uint32_t i = 0; i < m_hypercubeNodeCount; ++i)
        {
            const uint32_t offset = baseOffset + m_hypercubeNodeOffsets[i] + outputIndex;
            hyperCubeSamples[i] = (offset < m_samples.size()) ? m_samples[offset] : 0.0;
        }

        // We have loaded samples into the hypercube. Now, in each round of algorithm,
        // reduce the hypercube dimension by 1. At the end, we will have hypercube
        // with dimension 0, e.g. node.
        uint32_t currentHypercubeNodeCount = m_hypercubeNodeCount;
        for (uint32_t i = 0; i < m_m; ++i)
        {
            for (uint32_t j = 0; j < currentHypercubeNodeCount; j += 2)
            {
                hyperCubeSamples[j / 2] = encoded0[i] * hyperCubeSamples[j] + encoded1[i] * hyperCubeSamples[j + 1];
            }

            // We have reduced the hypercube node count 2 times - we have
            // reduced it by one dimension.
            currentHypercubeNodeCount = currentHypercubeNodeCount / 2;
        }

        const PDFReal outputValue = hyperCubeSamples[0];
        const PDFReal outputValueDecoded = interpolate(outputValue, 0.0, m_sampleMaximalValue, m_decoder[2 * outputIndex], m_decoder[2 * outputIndex + 1]);
        const PDFReal outputValueClamped = clampOutput(outputIndex, outputValueDecoded);
        *std::next(y_1, outputIndex) = outputValueClamped;
    }

    return true;
}

PDFExponentialFunction::PDFExponentialFunction(uint32_t m, uint32_t n,
                                               std::vector<PDFReal>&& domain,
                                               std::vector<PDFReal>&& range,
                                               std::vector<PDFReal>&& c0,
                                               std::vector<PDFReal>&& c1,
                                               PDFReal exponent) :
    PDFFunction(m, n, std::move(domain), std::move(range)),
    m_c0(std::move(c0)),
    m_c1(std::move(c1)),
    m_exponent(exponent),
    m_isLinear(qFuzzyCompare(exponent, 1.0))
{
    Q_ASSERT(m == 1);
    Q_ASSERT(m_c0.size() == n);
    Q_ASSERT(m_c1.size() == n);
}

PDFFunction::FunctionResult PDFExponentialFunction::apply(PDFFunction::const_iterator x_1,
                                                          PDFFunction::const_iterator x_m,
                                                          PDFFunction::iterator y_1,
                                                          PDFFunction::iterator y_n) const
{
    const size_t m = std::distance(x_1, x_m);
    const size_t n = std::distance(y_1, y_n);

    if (m != m_m)
    {
        return PDFTranslationContext::tr("Invalid number of operands for function. Expected %1, provided %2.").arg(m_m).arg(m);
    }
    if (n != m_n)
    {
        return PDFTranslationContext::tr("Invalid number of output variables for function. Expected %1, provided %2.").arg(m_n).arg(n);
    }

    Q_ASSERT(m == 1);
    const PDFReal x = clampInput(0, *x_1);

    if (!m_isLinear)
    {
        // Perform exponential interpolation
        size_t index = 0;
        for (PDFFunction::iterator y = y_1; y != y_n; ++y, ++index)
        {
            *y = m_c0[index] + std::pow(x, m_exponent) * (m_c1[index] - m_c0[index]);
        }
    }
    else
    {
        // Perform linear interpolation
        size_t index = 0;
        for (PDFFunction::iterator y = y_1; y != y_n; ++y, ++index)
        {
            *y = mix(x, m_c0[index], m_c1[index]);
        }
    }

    if (hasRange())
    {
        size_t index = 0;
        for (PDFFunction::iterator y = y_1; y != y_n; ++y, ++index)
        {
            *y = clampOutput(index, *y);
        }
    }

    return true;
}

PDFStitchingFunction::PDFStitchingFunction(uint32_t m, uint32_t n,
                                           std::vector<PDFReal>&& domain,
                                           std::vector<PDFReal>&& range,
                                           std::vector<PDFStitchingFunction::PartialFunction>&& partialFunctions) :
    PDFFunction(m, n, std::move(domain), std::move(range)),
    m_partialFunctions(std::move(partialFunctions))
{
    Q_ASSERT(m == 1);
}

PDFStitchingFunction::~PDFStitchingFunction()
{

}

PDFFunction::FunctionResult PDFStitchingFunction::apply(const_iterator x_1,
                                                        const_iterator x_m,
                                                        iterator y_1,
                                                        iterator y_n) const
{
    const size_t m = std::distance(x_1, x_m);
    const size_t n = std::distance(y_1, y_n);

    if (m != m_m)
    {
        return PDFTranslationContext::tr("Invalid number of operands for function. Expected %1, provided %2.").arg(m_m).arg(m);
    }
    if (n != m_n)
    {
        return PDFTranslationContext::tr("Invalid number of output variables for function. Expected %1, provided %2.").arg(m_n).arg(n);
    }

    Q_ASSERT(m == 1);
    const PDFReal x = clampInput(0, *x_1);

    // First search for partial function, which defines our range. Use algorithm
    // similar to the std::lower_bound.
    auto it = std::lower_bound(m_partialFunctions.cbegin(), m_partialFunctions.cend(), x, [](const auto& partialFunction, PDFReal value) { return partialFunction.bound1 < value; });
    if (it == m_partialFunctions.cend())
    {
        --it;
    }
    const PartialFunction& function = *it;

    // Encode the value into the input range of the function
    const PDFReal xEncoded = interpolate(x, function.bound0, function.bound1, function.encode0, function.encode1);
    FunctionResult result = function.function->apply(&xEncoded, &xEncoded + 1, y_1, y_n);

    if (hasRange())
    {
        size_t index = 0;
        for (PDFFunction::iterator y = y_1; y != y_n; ++y, ++index)
        {
            *y = clampOutput(index, *y);
        }
    }

    return result;
}

PDFIdentityFunction::PDFIdentityFunction() :
    PDFFunction(0, 0, std::vector<PDFReal>(), std::vector<PDFReal>())
{

}

PDFFunction::FunctionResult PDFIdentityFunction::apply(const_iterator x_1,
                                                       const_iterator x_m,
                                                       iterator y_1,
                                                       iterator y_n) const
{
    const size_t m = std::distance(x_1, x_m);
    const size_t n = std::distance(y_1, y_n);

    if (m != n)
    {
        return PDFTranslationContext::tr("Invalid number of operands for identity function. Expected %1, provided %2.").arg(n).arg(m);
    }

    std::copy(x_1, x_m, y_1);
    return true;
}

class PDFPostScriptFunctionStack
{
public:
    inline explicit PDFPostScriptFunctionStack() = default;

    using OperandObject = PDFPostScriptFunction::OperandObject;
    using InstructionPointer = PDFPostScriptFunction::InstructionPointer;

    inline void pushReal(PDFReal value) { m_stack.push_back(OperandObject::createReal(value)); checkOverflow(); }
    inline void pushInteger(PDFInteger value) { m_stack.push_back(OperandObject::createInteger(value)); checkOverflow(); }
    inline void pushBoolean(bool value) { m_stack.push_back(OperandObject::createBoolean(value)); checkOverflow(); }
    inline void pushInstructionPointer(InstructionPointer value) { m_stack.push_back(OperandObject::createInstructionPointer(value)); checkOverflow(); }

    /// Returns true, if integer operation should be performed instead of operation with real values.
    /// (two top elements are integer).
    bool isBinaryOperationInteger() const;

    /// Returns true, if boolean operation should be performed instead of operation with integer values.
    /// (two top elements are boolean).
    bool isBinaryOperationBoolean() const;

    /// Pops the real value from the stack (throw exception, if stack underflow occurs,
    /// or value is not of type real).
    PDFReal popReal();

    /// Pops the integer value from the stack (throw exception, if stack underflow occurs,
    /// or value is not of type integer).
    PDFInteger popInteger();

    /// Pops the boolean value from the stack (throw exception, if stack underflow occurs,
    /// or value is not of type boolean).
    bool popBoolean();

    /// Pops the instruction pointer from the stack (throw exception, if stack underflow occurs,
    /// or value is not of type instruction pointer).
    InstructionPointer popInstructionPointer();

    /// Pops number (integer is converted to the real value) form the stack (throw exception, if stack underflow occurs,
    /// or value is not of type real or integer).
    PDFReal popNumber();

    /// Returns true, if current value is real
    bool isReal() const { checkUnderflow(); return m_stack.back().type == PDFPostScriptFunction::OperandType::Real; }

    /// Returns true, if current value is integer
    bool isInteger() const { checkUnderflow(); return m_stack.back().type == PDFPostScriptFunction::OperandType::Integer; }

    /// Pops the current value
    inline void pop() { checkUnderflow(); m_stack.pop_back(); }

    /// Exchange the two top elements
    void exch();

    /// Duplicate the top element
    void dup();

    /// Copy the n elements
    /// \param n Number of elements to be copied
    void copy(PDFInteger n);

    /// Copy the n-th element on the stack
    /// \param n Index of the element (indexed is from the top - top has index 0, bottom has index size() - 1)
    void index(PDFInteger n);

    /// Roll n elements on the stack j-times left
    /// \param n Number of elements to be rolled
    /// \param j Roll j-times
    void roll(PDFInteger n, PDFInteger j);

    /// Pushes the operand onto the stack
    void push(const OperandObject& operand) { m_stack.push_back(operand); checkOverflow(); }

    /// Returns true, if stack is empty
    bool empty() const { return m_stack.empty(); }

    /// Returns size of the stack
    std::size_t size() const { return m_stack.size(); }

private:
    /// Check operand stack overflow (maximum limit is 100, according to the PDF 1.7 specification)
    void checkOverflow() const;

    /// Check operand stack underflow (if stack has at least \p n values)
    /// \param n Number of values to check
    void checkUnderflow(size_t n = 1) const;

    PDFFlatArray<OperandObject, 8> m_stack;
};

/// Executes the postscript program. Can throw PDFPostScriptFunctionException.
class PDFPostScriptFunctionExecutor
{
public:
    using Program = PDFPostScriptFunction::Program;
    using Stack = PDFPostScriptFunctionStack;
    using InstructionPointer = PDFPostScriptFunction::InstructionPointer;
    using CodeObject = PDFPostScriptFunction::CodeObject;
    using PDFIntegerUnsigned = std::make_unsigned<PDFInteger>::type;

    /// Creates new postscript program
    explicit inline PDFPostScriptFunctionExecutor(const Program& program, Stack& stack) :
        m_program(program),
        m_stack(stack)
    {

    }

    /// Executes the postscript program
    void execute();

private:
   template<template<typename> typename Comparator>
    void executeRelationOperator()
    {
        if (m_stack.isBinaryOperationInteger())
        {
            const PDFInteger b = m_stack.popInteger();
            const PDFInteger a = m_stack.popInteger();
            m_stack.pushBoolean(Comparator<PDFInteger>()(a, b));
        }
        else
        {
            const PDFReal b = m_stack.popNumber();
            const PDFReal a = m_stack.popNumber();
            m_stack.pushBoolean(Comparator<PDFReal>()(a, b));
        }
    }

    const Program& m_program;
    Stack& m_stack;
};

void PDFPostScriptFunctionExecutor::execute()
{
    Q_ASSERT(!m_program.empty());

    std::stack<InstructionPointer> callStack;

    InstructionPointer ip = 0; // First instruction is at zero
    while (ip != PDFPostScriptFunction::INVALID_INSTRUCTION_POINTER)
    {
        if (ip >= m_program.size())
        {
            throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Invalid instruction pointer."));
        }

        const CodeObject& instruction = m_program[ip];
        switch (instruction.code)
        {
            case PDFPostScriptFunction::Code::Add:
            {
                if (m_stack.isBinaryOperationInteger())
                {
                    const PDFInteger b = m_stack.popInteger();
                    const PDFInteger a = m_stack.popInteger();
                    m_stack.pushInteger(a + b);
                }
                else
                {
                    const PDFReal b = m_stack.popNumber();
                    const PDFReal a = m_stack.popNumber();
                    m_stack.pushReal(a + b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Sub:
            {
                if (m_stack.isBinaryOperationInteger())
                {
                    const PDFInteger b = m_stack.popInteger();
                    const PDFInteger a = m_stack.popInteger();
                    m_stack.pushInteger(a - b);
                }
                else
                {
                    const PDFReal b = m_stack.popNumber();
                    const PDFReal a = m_stack.popNumber();
                    m_stack.pushReal(a - b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Mul:
            {
                if (m_stack.isBinaryOperationInteger())
                {
                    const PDFInteger b = m_stack.popInteger();
                    const PDFInteger a = m_stack.popInteger();
                    m_stack.pushInteger(a * b);
                }
                else
                {
                    const PDFReal b = m_stack.popNumber();
                    const PDFReal a = m_stack.popNumber();
                    m_stack.pushReal(a * b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Div:
            {
                const PDFReal b = m_stack.popNumber();
                const PDFReal a = m_stack.popNumber();

                if (qFuzzyIsNull(b))
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Division by zero (PostScript engine)."));
                }

                m_stack.pushReal(a / b);
                break;
            }

            case PDFPostScriptFunction::Code::Idiv:
            {
                const PDFInteger b = m_stack.popInteger();
                const PDFInteger a = m_stack.popInteger();

                if (b == 0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Division by zero (PostScript engine)."));
                }

                m_stack.pushInteger(a / b);
                break;
            }

            case PDFPostScriptFunction::Code::Mod:
            {
                const PDFInteger b = m_stack.popInteger();
                const PDFInteger a = m_stack.popInteger();

                if (b == 0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Division by zero (PostScript engine)."));
                }

                m_stack.pushInteger(a % b);
                break;
            }

            case PDFPostScriptFunction::Code::Neg:
            {
                if (m_stack.isInteger())
                {
                    m_stack.pushInteger(-m_stack.popInteger());
                }
                else
                {
                    m_stack.pushReal(-m_stack.popReal());
                }
                break;
            }

            case PDFPostScriptFunction::Code::Abs:
            {
                if (m_stack.isInteger())
                {
                    m_stack.pushInteger(qAbs(m_stack.popInteger()));
                }
                else
                {
                    m_stack.pushReal(qAbs(m_stack.popReal()));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Ceiling:
            {
                if (m_stack.isReal())
                {
                    m_stack.pushReal(std::ceil(m_stack.popReal()));
                }
                else if (!m_stack.isInteger())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Number expected for ceil function (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Floor:
            {
                if (m_stack.isReal())
                {
                    m_stack.pushReal(std::floor(m_stack.popReal()));
                }
                else if (!m_stack.isInteger())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Number expected for floor function (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Round:
            {
                if (m_stack.isReal())
                {
                    m_stack.pushReal(qRound(m_stack.popReal()));
                }
                else if (!m_stack.isInteger())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Number expected for round function (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Truncate:
            {
                if (m_stack.isReal())
                {
                    m_stack.pushReal(std::trunc(m_stack.popReal()));
                }
                else if (!m_stack.isInteger())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Number expected for truncate function (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Sqrt:
            {
                const PDFReal value = m_stack.popNumber();

                if (value < 0.0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Square root of negative value can't be computed (PostScript engine)."));
                }

                m_stack.pushReal(std::sqrt(value));
                break;
            }

            case PDFPostScriptFunction::Code::Sin:
            {
                m_stack.pushReal(qSin(qDegreesToRadians(m_stack.popNumber())));
                break;
            }

            case PDFPostScriptFunction::Code::Cos:
            {
                m_stack.pushReal(qCos(qDegreesToRadians(m_stack.popNumber())));
                break;
            }

            case PDFPostScriptFunction::Code::Atan:
            {
                const PDFReal b = m_stack.popNumber();
                const PDFReal a = m_stack.popNumber();

                const PDFReal angles = qRadiansToDegrees(qAtan2(a, b));
                m_stack.pushReal(angles < 0.0 ? (angles + 360.0) : angles);
                break;
            }

            case PDFPostScriptFunction::Code::Exp:
            {
                const PDFReal exponent = m_stack.popNumber();
                const PDFReal base = m_stack.popNumber();
                m_stack.pushReal(qPow(base, exponent));
                break;
            }

            case PDFPostScriptFunction::Code::Ln:
            {
                const PDFReal value = m_stack.popNumber();

                if (value < 0.0 || qFuzzyIsNull(value))
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Logarithm's input should be positive value  (PostScript engine)."));
                }

                m_stack.pushReal(qLn(value));
                break;
            }

            case PDFPostScriptFunction::Code::Log:
            {
                const PDFReal value = m_stack.popNumber();

                if (value < 0.0 || qFuzzyIsNull(value))
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Logarithm's input should be positive value (PostScript engine)."));
                }

                m_stack.pushReal(std::log10(value));
                break;
            }

            case PDFPostScriptFunction::Code::Cvi:
            {
                if (m_stack.isReal())
                {
                    m_stack.pushInteger(static_cast<PDFInteger>(m_stack.popReal()));
                }
                else if (!m_stack.isInteger())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Real value expected for conversion to integer (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Cvr:
            {
                if (m_stack.isInteger())
                {
                    m_stack.pushReal(m_stack.popInteger());
                }
                else if (!m_stack.isReal())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Integer value expected for conversion to real (PostScript engine)."));
                }
                break;
            }

            case PDFPostScriptFunction::Code::Eq:
            {
                if (m_stack.isBinaryOperationInteger())
                {
                    const PDFInteger b = m_stack.popInteger();
                    const PDFInteger a = m_stack.popInteger();
                    m_stack.pushBoolean(a == b);
                }
                else if (m_stack.isBinaryOperationBoolean())
                {
                    const bool b = m_stack.popBoolean();
                    const bool a = m_stack.popBoolean();
                    m_stack.pushBoolean(a == b);
                }
                else
                {
                    // Real values
                    const PDFReal b = m_stack.popNumber();
                    const PDFReal a = m_stack.popNumber();
                    m_stack.pushBoolean(a == b);
                }

                break;
            }

            case PDFPostScriptFunction::Code::Ne:
            {
                if (m_stack.isBinaryOperationInteger())
                {
                    const PDFInteger b = m_stack.popInteger();
                    const PDFInteger a = m_stack.popInteger();
                    m_stack.pushBoolean(a != b);
                }
                else if (m_stack.isBinaryOperationBoolean())
                {
                    const bool b = m_stack.popBoolean();
                    const bool a = m_stack.popBoolean();
                    m_stack.pushBoolean(a != b);
                }
                else
                {
                    // Real values
                    const PDFReal b = m_stack.popNumber();
                    const PDFReal a = m_stack.popNumber();
                    m_stack.pushBoolean(a != b);
                }

                break;
            }

            case PDFPostScriptFunction::Code::Gt:
            {
                executeRelationOperator<std::greater>();
                break;
            }

            case PDFPostScriptFunction::Code::Ge:
            {
                executeRelationOperator<std::greater_equal>();
                break;
            }

            case PDFPostScriptFunction::Code::Lt:
            {
                executeRelationOperator<std::less>();
                break;
            }

            case PDFPostScriptFunction::Code::Le:
            {
                executeRelationOperator<std::less_equal>();
                break;
            }

            case PDFPostScriptFunction::Code::And:
            {
                if (m_stack.isBinaryOperationBoolean())
                {
                    const bool a = m_stack.popBoolean();
                    const bool b = m_stack.popBoolean();
                    m_stack.pushBoolean(a && b);
                }
                else
                {
                    const PDFIntegerUnsigned a = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    const PDFIntegerUnsigned b = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    m_stack.pushInteger(a & b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Or:
            {
                if (m_stack.isBinaryOperationBoolean())
                {
                    const bool a = m_stack.popBoolean();
                    const bool b = m_stack.popBoolean();
                    m_stack.pushBoolean(a || b);
                }
                else
                {
                    const PDFIntegerUnsigned a = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    const PDFIntegerUnsigned b = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    m_stack.pushInteger(a | b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Xor:
            {
                if (m_stack.isBinaryOperationBoolean())
                {
                    const bool a = m_stack.popBoolean();
                    const bool b = m_stack.popBoolean();
                    m_stack.pushBoolean(a != b);
                }
                else
                {
                    const PDFIntegerUnsigned a = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    const PDFIntegerUnsigned b = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    m_stack.pushInteger(a ^ b);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Not:
            {
                if (m_stack.isInteger())
                {
                    const PDFIntegerUnsigned value = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                    m_stack.pushInteger(~value);
                }
                else
                {
                    const bool value = m_stack.popBoolean();
                    m_stack.pushBoolean(!value);
                }
                break;
            }

            case PDFPostScriptFunction::Code::Bitshift:
            {
                const PDFInteger shift = m_stack.popInteger();
                const PDFIntegerUnsigned value = static_cast<PDFIntegerUnsigned>(m_stack.popInteger());
                PDFIntegerUnsigned shiftedValue = value;

                if (shift > 0)
                {
                    // Positive is left
                    shiftedValue = value << shift;
                }
                else if (shift < 0)
                {
                    // Negative is right
                    shiftedValue = value >> -shift;
                }

                m_stack.pushInteger(shiftedValue);
                break;
            }

            case PDFPostScriptFunction::Code::True:
            {
                m_stack.pushBoolean(true);
                break;
            }

            case PDFPostScriptFunction::Code::False:
            {
                m_stack.pushBoolean(false);
                break;
            }

            case PDFPostScriptFunction::Code::Execute:
            {
                const PDFPostScriptFunctionStack::InstructionPointer callIp = m_stack.popInstructionPointer();
                callStack.push(instruction.next);
                ip = callIp;
                continue;
            }

            case PDFPostScriptFunction::Code::If:
            {
                const PDFPostScriptFunctionStack::InstructionPointer callIp = m_stack.popInstructionPointer();
                const bool condition = m_stack.popBoolean();

                if (condition)
                {
                    // Call the if block
                    callStack.push(instruction.next);
                    ip = callIp;
                    continue;
                }

                break;
            }

            case PDFPostScriptFunction::Code::IfElse:
            {
                const PDFPostScriptFunctionStack::InstructionPointer falsePartIp = m_stack.popInstructionPointer();
                const PDFPostScriptFunctionStack::InstructionPointer truePartIp = m_stack.popInstructionPointer();
                const bool condition = m_stack.popBoolean();

                callStack.push(instruction.next);
                if (condition)
                {
                    // Call the if part
                    ip = truePartIp;
                }
                else
                {
                    // Call the else part
                    ip = falsePartIp;
                }

                continue;
            }
            case PDFPostScriptFunction::Code::Pop:
            {
                m_stack.pop();
                break;
            }

            case PDFPostScriptFunction::Code::Exch:
            {
                m_stack.exch();
                break;
            }

            case PDFPostScriptFunction::Code::Dup:
            {
                m_stack.dup();
                break;
            }

            case PDFPostScriptFunction::Code::Copy:
            {
                const PDFInteger n = m_stack.popInteger();

                if (n < 0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Can't copy negative number of arguments (PostScript engine)."));
                }

                if (n > 0)
                {
                    m_stack.copy(n);
                }

                break;
            }

            case PDFPostScriptFunction::Code::Index:
            {
                const PDFInteger n = m_stack.popInteger();

                if (n < 0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Negative index of operand (PostScript engine)."));
                }

                m_stack.index(n);
                break;
            }

            case PDFPostScriptFunction::Code::Roll:
            {
                const PDFInteger j = m_stack.popInteger();
                const PDFInteger n = m_stack.popInteger();

                if (n < 0)
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Negative number of operands (PostScript engine)."));
                }

                m_stack.roll(n, j);
                break;
            }

            case PDFPostScriptFunction::Code::Call:
            {
                Q_ASSERT(instruction.operand.type == PDFPostScriptFunction::OperandType::InstructionPointer);
                m_stack.pushInstructionPointer(instruction.operand.instructionPointer);
                break;
            }

            case PDFPostScriptFunction::Code::Return:
            {
                if (callStack.empty())
                {
                    throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Call stack underflow (PostScript engine)."));
                }

                ip = callStack.top();
                callStack.pop();
                continue;
            }

            case PDFPostScriptFunction::Code::Push:
            {
                m_stack.push(instruction.operand);
                break;
            }
        }

        // Move to the next instruction
        ip = instruction.next;
    }
}

bool PDFPostScriptFunctionStack::isBinaryOperationInteger() const
{
    checkUnderflow(2);

    const size_t size = m_stack.size();
    return m_stack[size - 1].type == PDFPostScriptFunction::OperandType::Integer &&
            m_stack[size - 2].type == PDFPostScriptFunction::OperandType::Integer;
}

bool PDFPostScriptFunctionStack::isBinaryOperationBoolean() const
{
    checkUnderflow(2);

    const size_t size = m_stack.size();
    return m_stack[size - 1].type == PDFPostScriptFunction::OperandType::Boolean &&
            m_stack[size - 2].type == PDFPostScriptFunction::OperandType::Boolean;
}

PDFReal PDFPostScriptFunctionStack::popReal()
{
    checkUnderflow();

    const PDFPostScriptFunction::OperandObject& topElement = m_stack.back();
    if (topElement.type == PDFPostScriptFunction::OperandType::Real)
    {
        const PDFReal value = topElement.realNumber;
        m_stack.pop_back();
        return value;
    }
    else
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Real value expected (PostScript engine)."));
    }
}

PDFInteger PDFPostScriptFunctionStack::popInteger()
{
    checkUnderflow();

    const PDFPostScriptFunction::OperandObject& topElement = m_stack.back();
    if (topElement.type == PDFPostScriptFunction::OperandType::Integer)
    {
        const PDFInteger value = topElement.integerNumber;
        m_stack.pop_back();
        return value;
    }
    else
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Integer value expected (PostScript engine)."));
    }
}

bool PDFPostScriptFunctionStack::popBoolean()
{
    checkUnderflow();

    const PDFPostScriptFunction::OperandObject& topElement = m_stack.back();
    if (topElement.type == PDFPostScriptFunction::OperandType::Boolean)
    {
        const bool value = topElement.boolean;
        m_stack.pop_back();
        return value;
    }
    else
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Boolean value expected (PostScript engine)."));
    }
}

PDFPostScriptFunctionStack::InstructionPointer PDFPostScriptFunctionStack::popInstructionPointer()
{
    checkUnderflow();

    const PDFPostScriptFunction::OperandObject& topElement = m_stack.back();
    if (topElement.type == PDFPostScriptFunction::OperandType::InstructionPointer)
    {
        const InstructionPointer value = topElement.instructionPointer;
        m_stack.pop_back();
        return value;
    }
    else
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Instruction pointer expected (PostScript engine)."));
    }
}

PDFReal PDFPostScriptFunctionStack::popNumber()
{
    checkUnderflow();

    const PDFPostScriptFunction::OperandObject& topElement = m_stack.back();
    if (topElement.type == PDFPostScriptFunction::OperandType::Real)
    {
        const PDFReal value = topElement.realNumber;
        m_stack.pop_back();
        return value;
    }
    else if (topElement.type == PDFPostScriptFunction::OperandType::Integer)
    {
        const PDFInteger value = topElement.integerNumber;
        m_stack.pop_back();
        return value;
    }
    else
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Number expected (PostScript engine)."));
    }
}

void PDFPostScriptFunctionStack::exch()
{
    checkUnderflow(2);

    const size_t size = m_stack.size();
    std::swap(m_stack[size - 2], m_stack[size - 1]);
}

void PDFPostScriptFunctionStack::dup()
{
    checkUnderflow();
    m_stack.push_back(m_stack.back());
    checkOverflow();
}

void PDFPostScriptFunctionStack::copy(PDFInteger n)
{
    Q_ASSERT(n > 0);

    checkUnderflow(static_cast<size_t>(n));

    size_t startIndex = m_stack.size() - n;
    for (size_t i = 0; i < static_cast<size_t>(n); ++i)
    {
        m_stack.push_back(m_stack[startIndex + i]);
        checkOverflow();
    }
}

void PDFPostScriptFunctionStack::index(PDFInteger n)
{
    Q_ASSERT(n >= 0);

    checkUnderflow(static_cast<size_t>(n) + 1);
    m_stack.push_back(m_stack[m_stack.size() - 1 - n]);
}

void PDFPostScriptFunctionStack::roll(PDFInteger n, PDFInteger j)
{
    if (n == 0)
    {
        // If n is zero, then we are rolling zero arguments - do nothing
        return;
    }

    // If we roll n-times, then we get original sequence
    j = j % n;
    if (j == 0)
    {
        // If j is zero, then we don't roll anything at all - do nothing
        return;
    }

    checkUnderflow(n);

    // Load operands into temporary array
    const size_t firstIndexOnStack = m_stack.size() - n;
    std::vector<OperandObject> operands(n);
    for (size_t i = 0; i < static_cast<size_t>(n); ++i)
    {
        operands[i] = m_stack[firstIndexOnStack + i];
    }

    if (j > 0)
    {
        // Rotate left j times
        std::rotate(operands.begin(), operands.end() - j, operands.end());
    }
    else
    {
        // Rotate right j times
        std::rotate(operands.rbegin(), operands.rend() + j, operands.rend());
    }

    // Load data back from temporary array
    for (size_t i = 0; i < static_cast<size_t>(n); ++i)
    {
        m_stack[firstIndexOnStack + i] = operands[i];
    }
}

void PDFPostScriptFunctionStack::checkOverflow() const
{
    if (m_stack.size() > 100)
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Stack overflow occured (PostScript engine)."));
    }
}

void PDFPostScriptFunctionStack::checkUnderflow(size_t n) const
{
    if (m_stack.size() < n)
    {
        throw PDFPostScriptFunction::PDFPostScriptFunctionException(PDFTranslationContext::tr("Stack underflow occured (PostScript engine)."));
    }
}

PDFPostScriptFunction::Code PDFPostScriptFunction::getCode(const QByteArray& byteArray)
{
    static constexpr const std::pair<Code, const  char*> codes[] =
    {
        // B.1 Arithmetic operators
        std::pair<Code, const  char*>{ Code::Add, "add" },
        std::pair<Code, const  char*>{ Code::Sub, "sub" },
        std::pair<Code, const  char*>{ Code::Mul, "mul" },
        std::pair<Code, const  char*>{ Code::Div, "div" },
        std::pair<Code, const  char*>{ Code::Idiv, "idiv" },
        std::pair<Code, const  char*>{ Code::Mod, "mod" },
        std::pair<Code, const  char*>{ Code::Neg, "neg" },
        std::pair<Code, const  char*>{ Code::Abs, "abs" },
        std::pair<Code, const  char*>{ Code::Ceiling, "ceiling" },
        std::pair<Code, const  char*>{ Code::Floor, "floor" },
        std::pair<Code, const  char*>{ Code::Round, "round" },
        std::pair<Code, const  char*>{ Code::Truncate, "truncate" },
        std::pair<Code, const  char*>{ Code::Sqrt, "sqrt" },
        std::pair<Code, const  char*>{ Code::Sin, "sin" },
        std::pair<Code, const  char*>{ Code::Cos, "cos" },
        std::pair<Code, const  char*>{ Code::Atan, "atan" },
        std::pair<Code, const  char*>{ Code::Exp, "exp" },
        std::pair<Code, const  char*>{ Code::Ln, "ln" },
        std::pair<Code, const  char*>{ Code::Log, "log" },
        std::pair<Code, const  char*>{ Code::Cvi, "cvi" },
        std::pair<Code, const  char*>{ Code::Cvr, "cvr" },

        // B.2 Relational, Boolean and Bitwise operators
        std::pair<Code, const  char*>{ Code::Eq, "eq" },
        std::pair<Code, const  char*>{ Code::Ne, "ne" },
        std::pair<Code, const  char*>{ Code::Gt, "gt" },
        std::pair<Code, const  char*>{ Code::Ge, "ge" },
        std::pair<Code, const  char*>{ Code::Lt, "lt" },
        std::pair<Code, const  char*>{ Code::Le, "le" },
        std::pair<Code, const  char*>{ Code::And, "and" },
        std::pair<Code, const  char*>{ Code::Or, "or" },
        std::pair<Code, const  char*>{ Code::Xor, "xor" },
        std::pair<Code, const  char*>{ Code::Not, "not" },
        std::pair<Code, const  char*>{ Code::Bitshift, "bitshift" },
        std::pair<Code, const  char*>{ Code::True, "true" },
        std::pair<Code, const  char*>{ Code::False, "false" },

        // B.3 Conditional operators
        std::pair<Code, const  char*>{ Code::If, "if" },
        std::pair<Code, const  char*>{ Code::IfElse, "ifelse" },

        // B.4 Stack operators
        std::pair<Code, const  char*>{ Code::Pop, "pop" },
        std::pair<Code, const  char*>{ Code::Exch, "exch" },
        std::pair<Code, const  char*>{ Code::Dup, "dup" },
        std::pair<Code, const  char*>{ Code::Copy, "copy" },
        std::pair<Code, const  char*>{ Code::Index, "index" },
        std::pair<Code, const  char*>{ Code::Roll, "roll" }
    };

    for (const std::pair<Code, const  char*>& codeItem : codes)
    {
        if (byteArray == codeItem.second)
        {
            return codeItem.first;
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid operator (PostScript function) '%1'.").arg(QString::fromLatin1(byteArray)));
}

PDFPostScriptFunction::PDFPostScriptFunction(uint32_t m, uint32_t n, std::vector<PDFReal>&& domain, std::vector<PDFReal>&& range, PDFPostScriptFunction::Program&& program) :
    PDFFunction(m, n, std::move(domain), std::move(range)),
    m_program(std::move(program))
{
    Q_ASSERT(!m_program.empty());
}

PDFPostScriptFunction::~PDFPostScriptFunction()
{

}

PDFPostScriptFunction::Program PDFPostScriptFunction::parseProgram(const QByteArray& byteArray)
{
    // Lexical analyzer can't handle when '{' or '}' is near next token (for example '{0' etc.)
    QByteArray adjustedArray = byteArray;
    adjustedArray.replace('{', " { ").replace('}', " } ");

    Program result;
    PDFLexicalAnalyzer parser(adjustedArray.constBegin(), adjustedArray.constEnd());
    parser.setTokenizingPostScriptFunction();

    std::stack<InstructionPointer> blockCallStack;
    while (true)
    {
        PDFLexicalAnalyzer::Token token = parser.fetch();
        if (token.type == PDFLexicalAnalyzer::TokenType::EndOfFile)
        {
            // We are at end, stop the parsing
            break;
        }

        switch (token.type)
        {
            case PDFLexicalAnalyzer::TokenType::Boolean:
            {
                result.emplace_back(OperandObject::createBoolean(token.data.toBool()), result.size() + 1);
                break;
            }

            case PDFLexicalAnalyzer::TokenType::Integer:
            {
                result.emplace_back(OperandObject::createInteger(token.data.toLongLong()), result.size() + 1);
                break;
            }

            case PDFLexicalAnalyzer::TokenType::Real:
            {
                result.emplace_back(OperandObject::createReal(token.data.toDouble()), result.size() + 1);
                break;
            }

            case PDFLexicalAnalyzer::TokenType::Command:
            {
                QByteArray command = token.data.toByteArray();
                if (command == "{")
                {
                    // Opening bracket - means start of block
                    blockCallStack.push(result.size());
                    result.emplace_back(Code::Call, INVALID_INSTRUCTION_POINTER);
                    result.back().operand = OperandObject::createInstructionPointer(result.size());
                }
                else if (command == "}")
                {
                    // Closing bracket - means end of block
                    if (blockCallStack.empty())
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid program - bad enclosing brackets (PostScript function)."));
                    }

                    result[blockCallStack.top()].next = result.size() + 1;
                    blockCallStack.pop();
                    result.emplace_back(Code::Return, INVALID_INSTRUCTION_POINTER);
                }
                else
                {
                    result.emplace_back(getCode(command), result.size() + 1);
                }

                break;
            }

            default:
            {
                // All other tokens treat as invalid.
                throw PDFException(PDFTranslationContext::tr("Invalid program (PostScript function)."));
            }
        }
    }

    if (result.empty())
    {
        throw PDFException(PDFTranslationContext::tr("Empty program (PostScript function)."));
    }

    // We must insert execute instructions, where blocks without if/ifelse occurs.
    // We can have following program "{ 2 3 add }" which must return 5. How to find blocks,
    // after which instructions must be executed? Next instruction must be if, or next instruction
    // must be a call and next-next instruction must be ifelse

    auto isBlockUsed = [&result](InstructionPointer ip)
    {
        // We should call this function only on Call opcode
        Q_ASSERT(result[ip].code == Code::Call);

        const InstructionPointer next = result[ip].next;
        if (next < result.size())
        {
            switch (result[next].code)
            {
                case Code::If:
                case Code::IfElse:
                {
                    // Block is used in 'If' statement
                    return true;
                }

                case Code::Call:
                {
                    // We must detect, if we use 'If-Else' statement
                    const InstructionPointer nextnext = result[next].next;

                    if (nextnext < result.size())
                    {
                        return result[nextnext].code == Code::IfElse;
                    }
                    return false;
                }

                default:
                    return false;
            }
        }

        return false;
    };

    // Insert execute instructions, where there are call blocks, which are not used in if/ifelse statements
    for (size_t i = 0; i < result.size(); ++i)
    {
        if (result[i].code == Code::Call && !isBlockUsed(i))
        {
            InstructionPointer insertPosition = result[i].next;

            // We must update the instructions pointers for inserting the instruction
            for (CodeObject& codeObject : result)
            {
                if (codeObject.next > insertPosition && codeObject.next != INVALID_INSTRUCTION_POINTER)
                {
                    ++codeObject.next;
                }
                if (codeObject.operand.type == OperandType::InstructionPointer &&
                    codeObject.operand.instructionPointer > insertPosition &&
                    codeObject.operand.instructionPointer != INVALID_INSTRUCTION_POINTER)
                {
                    ++codeObject.operand.instructionPointer;
                }
            }

            // We must insert an execute statement, block is not used in if/ifelse statement
            result.insert(std::next(result.begin(), insertPosition), CodeObject(Code::Execute, insertPosition + 1));
        }
    }

    // Mark we are at the end of the program
    for (CodeObject& codeObject : result)
    {
        if (codeObject.next == result.size())
        {
            codeObject.next = INVALID_INSTRUCTION_POINTER;
        }
    }
    Q_ASSERT(result.back().next == INVALID_INSTRUCTION_POINTER);

    result.shrink_to_fit();
    return result;
}

PDFFunction::FunctionResult PDFPostScriptFunction::apply(const_iterator x_1, const_iterator x_m, iterator y_1, iterator y_n) const
{
    const size_t m = std::distance(x_1, x_m);
    const size_t n = std::distance(y_1, y_n);

    if (m != m_m)
    {
        return PDFTranslationContext::tr("Invalid number of operands for function. Expected %1, provided %2.").arg(m_m).arg(m);
    }
    if (n != m_n)
    {
        return PDFTranslationContext::tr("Invalid number of output variables for function. Expected %1, provided %2.").arg(m_n).arg(n);
    }

    try
    {
        PDFPostScriptFunctionStack stack;

        // Insert input values
        for (uint32_t i = 0; i < m; ++i)
        {
            const PDFReal x = *std::next(x_1, i);
            const PDFReal xClamped = clampInput(i, x);
            stack.pushReal(xClamped);
        }

        PDFPostScriptFunctionExecutor executor(m_program, stack);
        executor.execute();

        uint32_t i = static_cast<uint32_t>(n);
        auto it = std::make_reverse_iterator(y_n);
        auto itEnd = std::make_reverse_iterator(y_1);
        for (; it != itEnd; ++it)
        {
            const PDFReal y = stack.popNumber();
            const PDFReal yClamped = clampOutput(--i, y);
            *it = yClamped;
        }

        if (!stack.empty())
        {
            return PDFTranslationContext::tr("Stack contains more values, than output size (%1 remains) (PostScript function).").arg(stack.size());
        }
    }
    catch (const PDFPostScriptFunction::PDFPostScriptFunctionException& exception)
    {
        return exception.getMessage();
    }

    return true;
}

}   // namespace pdf
