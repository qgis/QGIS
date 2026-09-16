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

#include "pdfdocumentsanitizer.h"
#include "pdfvisitor.h"
#include "pdfexecutionpolicy.h"
#include "pdfoptimizer.h"
#include "pdfdocumentbuilder.h"
#include "pdfparser.h"
#include "pdfstreamfilters.h"
#include "pdffont.h"
#include "pdfdbgheap.h"

namespace pdf
{

struct SanitizedObjectResult
{
    PDFObject object;
    bool changed = false;
};

struct InvisibleTextSanitizationContext
{
    std::set<PDFObjectReference> processedContentObjects;
    std::set<PDFObjectReference> processedResourceObjects;
    std::set<PDFObjectReference> processedXObjectObjects;
    PDFInteger modifiedStreamCount = 0;
};

/// Helper implementing low-level sanitization of invisible OCR text.
///
/// The goal is intentionally narrow: remove only text-showing operators executed
/// with text rendering mode Tr = 3 (invisible text), while keeping all other
/// operators and stream structure unchanged whenever possible. This is safer than
/// rebuilding the page from a higher-level model because the original content may
/// contain inline images or uncommon operator sequences that should survive the
/// sanitization unchanged.
class PDFInvisibleTextSanitizerHelper
{
public:
    using SanitizedObjectResult = ::pdf::SanitizedObjectResult;
    using InvisibleTextSanitizationContext = ::pdf::InvisibleTextSanitizationContext;

    static bool isTextShowingOperator(const QByteArray& command);
    static bool isInlineImageOperator(const QByteArray& command);
    static PDFInteger readDirectIntegerFromDictionary(const PDFDictionary* dictionary, const char* key, PDFInteger defaultValue);
    static QByteArray readDirectNameFromDictionary(const PDFDictionary* dictionary, const char* key);
    static QByteArray sanitizeInvisibleTextInContent(const QByteArray& content, bool* changed);
    static PDFObject createCompressedContentStream(const PDFStream* originalStream, QByteArray decodedContent, PDFDictionary dictionary);
    static bool isFormXObject(const PDFObject& object, PDFDocumentBuilder& builder);
    static SanitizedObjectResult sanitizeResourcesObject(PDFDocumentBuilder& builder, const PDFObject& resourcesObject, InvisibleTextSanitizationContext& context);
    static SanitizedObjectResult sanitizeFormXObject(PDFDocumentBuilder& builder, const PDFObject& object, InvisibleTextSanitizationContext& context);
    static SanitizedObjectResult sanitizeContentsObject(PDFDocumentBuilder& builder, const PDFObject& contentsObject, InvisibleTextSanitizationContext& context);
};

bool PDFInvisibleTextSanitizerHelper::isTextShowingOperator(const QByteArray& command)
{
    return command == "Tj" || command == "TJ" || command == "'" || command == "\"";
}

bool PDFInvisibleTextSanitizerHelper::isInlineImageOperator(const QByteArray& command)
{
    return command == "BI";
}

PDFInteger PDFInvisibleTextSanitizerHelper::readDirectIntegerFromDictionary(const PDFDictionary* dictionary, const char* key, PDFInteger defaultValue)
{
    if (!dictionary || !dictionary->hasKey(key))
    {
        return defaultValue;
    }

    const PDFObject& object = dictionary->get(key);
    if (object.isInt())
    {
        return object.getInteger();
    }

    if (object.isReal())
    {
        return PDFInteger(object.getReal());
    }

    return defaultValue;
}

QByteArray PDFInvisibleTextSanitizerHelper::readDirectNameFromDictionary(const PDFDictionary* dictionary, const char* key)
{
    if (!dictionary || !dictionary->hasKey(key))
    {
        return QByteArray();
    }

    const PDFObject& object = dictionary->get(key);
    return object.isName() ? object.getString() : QByteArray();
}

QByteArray PDFInvisibleTextSanitizerHelper::sanitizeInvisibleTextInContent(const QByteArray& content, bool* changed)
{
    Q_ASSERT(changed);
    *changed = false;

    struct GraphicState
    {
        TextRenderingMode textRenderingMode = TextRenderingMode::Fill;
    };

    // Work directly on lexical tokens so the sanitizer can remove only the
    // affected text-showing instructions and preserve the rest of the stream.
    PDFLexicalAnalyzer parser(content.constBegin(), content.constEnd());
    QByteArray output;
    std::vector<PDFLexicalAnalyzer::Token> operands;
    std::vector<GraphicState> graphicStateStack;

    GraphicState currentGraphicState;
    bool inTextObject = false;
    PDFInteger copyPosition = 0;
    PDFInteger operandStartPosition = -1;

    while (!parser.isAtEnd())
    {
        const PDFInteger tokenStartPosition = parser.pos();
        PDFLexicalAnalyzer::Token token = parser.fetch();
        const PDFInteger tokenEndPosition = parser.pos();

        if (token.type == PDFLexicalAnalyzer::TokenType::EndOfFile)
        {
            break;
        }

        if (token.type != PDFLexicalAnalyzer::TokenType::Command)
        {
            if (operandStartPosition == -1)
            {
                operandStartPosition = tokenStartPosition;
            }

            operands.push_back(std::move(token));
            continue;
        }

        const QByteArray command = token.data.toByteArray();

        if (isInlineImageOperator(command))
        {
            // Inline images are copied through unchanged. They are parsed only to
            // determine where the image data ends so the tokenizer stays aligned.
            const PDFInteger operatorBIPosition = parser.pos();
            PDFInteger operatorIDPosition = parser.findSubstring("ID", operatorBIPosition);
            PDFInteger operatorEIPosition = parser.findSubstring("EI", operatorIDPosition);
            const PDFInteger startDataPosition = operatorIDPosition + 3;

            if (operatorIDPosition != -1 && operatorEIPosition != -1)
            {
                PDFLexicalAnalyzer inlineImageLexicalAnalyzer(content.constBegin() + operatorBIPosition, content.constBegin() + operatorIDPosition);
                PDFParser inlineImageParser([&inlineImageLexicalAnalyzer] { return inlineImageLexicalAnalyzer.fetch(); });

                constexpr std::pair<const char*, const char*> replacements[] =
                {
                    { "BPC", "BitsPerComponent" },
                    { "CS", "ColorSpace" },
                    { "D", "Decode" },
                    { "DP", "DecodeParms" },
                    { "F", "Filter" },
                    { "H", "Height" },
                    { "IM", "ImageMask" },
                    { "I", "Interpolate" },
                    { "W", "Width" },
                    { "L", "Length" },
                    { "G", "DeviceGray" },
                    { "RGB", "DeviceRGB" },
                    { "CMYK", "DeviceCMYK" }
                };

                std::shared_ptr<PDFDictionary> dictionarySharedPointer = std::make_shared<PDFDictionary>();
                PDFDictionary* dictionary = dictionarySharedPointer.get();

                while (inlineImageParser.lookahead().type != PDFLexicalAnalyzer::TokenType::EndOfFile)
                {
                    PDFObject nameObject = inlineImageParser.getObject();
                    PDFObject valueObject = inlineImageParser.getObject();

                    if (!nameObject.isName())
                    {
                        break;
                    }

                    QByteArray name = nameObject.getString();
                    for (auto [string, replacement] : replacements)
                    {
                        if (name == string)
                        {
                            name = replacement;
                            break;
                        }
                    }

                    dictionary->addEntry(PDFInplaceOrMemoryString(qMove(name)), qMove(valueObject));
                }

                PDFInteger dataLength = 0;

                if (dictionary->hasKey("Length"))
                {
                    dataLength = readDirectIntegerFromDictionary(dictionary, "Length", 0);
                }
                else if (dictionary->hasKey("Filter"))
                {
                    dataLength = -1;
                    const QByteArray filterName = readDirectNameFromDictionary(dictionary, "Filter");
                    if (!filterName.isEmpty())
                    {
                        dataLength = PDFStreamFilterStorage::getStreamDataLength(content, filterName, startDataPosition);
                    }

                    if (dataLength == -1)
                    {
                        dataLength = operatorEIPosition - startDataPosition;
                    }
                }
                else
                {
                    const PDFInteger width = readDirectIntegerFromDictionary(dictionary, "Width", 0);
                    const PDFInteger height = readDirectIntegerFromDictionary(dictionary, "Height", 0);
                    const PDFInteger bpc = readDirectIntegerFromDictionary(dictionary, "BitsPerComponent", 8);

                    if (width > 0 && height > 0 && bpc > 0)
                    {
                        const PDFInteger stride = (width * bpc + 7) / 8;
                        dataLength = stride * height;
                    }
                }

                if (dataLength >= 0)
                {
                    operatorEIPosition = parser.findSubstring("EI", startDataPosition + dataLength);
                    if (operatorEIPosition != -1)
                    {
                        parser.seek(operatorEIPosition + 2);
                    }
                }
            }

            operands.clear();
            operandStartPosition = -1;
            continue;
        }

        // Only remove text-showing operators from text objects currently using
        // invisible text rendering mode. Other rendering modes may affect page
        // appearance or clipping and are intentionally left untouched.
        const bool isInvisibleTextToRemove = inTextObject &&
                                             currentGraphicState.textRenderingMode == TextRenderingMode::Invisible &&
                                             isTextShowingOperator(command) &&
                                             operandStartPosition != -1;

        if (isInvisibleTextToRemove)
        {
            output.append(content.constData() + copyPosition, operandStartPosition - copyPosition);
            copyPosition = tokenEndPosition;
            *changed = true;
        }

        if (command == "q")
        {
            graphicStateStack.push_back(currentGraphicState);
        }
        else if (command == "Q")
        {
            if (!graphicStateStack.empty())
            {
                currentGraphicState = graphicStateStack.back();
                graphicStateStack.pop_back();
            }
        }
        else if (command == "BT")
        {
            inTextObject = true;
        }
        else if (command == "ET")
        {
            inTextObject = false;
        }
        else if (command == "Tr" && !operands.empty())
        {
            const PDFLexicalAnalyzer::Token& modeToken = operands.back();
            int mode = -1;
            if (modeToken.type == PDFLexicalAnalyzer::TokenType::Integer)
            {
                mode = int(modeToken.data.toLongLong());
            }
            else if (modeToken.type == PDFLexicalAnalyzer::TokenType::Real)
            {
                mode = int(modeToken.data.toDouble());
            }

            if (mode >= 0 && mode <= 7)
            {
                currentGraphicState.textRenderingMode = static_cast<TextRenderingMode>(mode);
            }
        }

        operands.clear();
        operandStartPosition = -1;
    }

    if (*changed)
    {
        output.append(content.constData() + copyPosition, content.size() - copyPosition);
        return output;
    }

    return content;
}

PDFObject PDFInvisibleTextSanitizerHelper::createCompressedContentStream(const PDFStream* originalStream,
                                                                         QByteArray decodedContent,
                                                                         PDFDictionary dictionary)
{
    // Re-encode the rewritten content into a regular compressed stream so the
    // updated object can be written back through the document builder.
    PDFArray filters;
    filters.appendItem(PDFObject::createName("FlateDecode"));

    dictionary.removeEntry("DecodeParms");

    QByteArray compressedData = PDFFlateDecodeFilter::compress(decodedContent);
    dictionary.setEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(compressedData.size()));
    dictionary.setEntry(PDFInplaceOrMemoryString("Filter"), PDFObject::createArray(std::make_shared<PDFArray>(qMove(filters))));

    Q_UNUSED(originalStream);
    return PDFObject::createStream(std::make_shared<PDFStream>(qMove(dictionary), qMove(compressedData)));
}

bool PDFInvisibleTextSanitizerHelper::isFormXObject(const PDFObject& object, PDFDocumentBuilder& builder)
{
    const PDFObject& dereferencedObject = builder.getObject(object);
    if (!dereferencedObject.isStream())
    {
        return false;
    }

    const PDFDictionary* dictionary = dereferencedObject.getStream()->getDictionary();
    return dictionary &&
           dictionary->hasKey("Subtype") &&
           builder.getObject(dictionary->get("Subtype")).isName() &&
           builder.getObject(dictionary->get("Subtype")).getString() == "Form";
}

SanitizedObjectResult PDFInvisibleTextSanitizerHelper::sanitizeFormXObject(PDFDocumentBuilder& builder,
                                                                           const PDFObject& object,
                                                                           InvisibleTextSanitizationContext& context)
{
    if (object.isReference())
    {
        const PDFObjectReference reference = object.getReference();
        if (!context.processedXObjectObjects.insert(reference).second)
        {
            return { object, false };
        }

        const PDFObject referencedObject = builder.getObjectByReference(reference);
        SanitizedObjectResult result = sanitizeFormXObject(builder, referencedObject, context);
        if (result.changed)
        {
            builder.setObject(reference, qMove(result.object));
        }
        return { object, false };
    }

    if (!isFormXObject(object, builder))
    {
        return { object, false };
    }

    const PDFStream* stream = object.getStream();
    PDFDictionary dictionary = *stream->getDictionary();
    QByteArray decodedContent = builder.getDecodedStream(stream);
    bool contentChanged = false;
    QByteArray sanitizedContent = sanitizeInvisibleTextInContent(decodedContent, &contentChanged);

    bool resourcesChanged = false;
    if (dictionary.hasKey("Resources"))
    {
        // OCR text may also be nested inside Form XObjects referenced from this
        // form, so sanitize the local resource tree recursively.
        SanitizedObjectResult resourcesResult = sanitizeResourcesObject(builder, dictionary.get("Resources"), context);
        if (resourcesResult.changed)
        {
            dictionary.setEntry(PDFInplaceOrMemoryString("Resources"), qMove(resourcesResult.object));
            resourcesChanged = true;
        }
    }

    if (!contentChanged && !resourcesChanged)
    {
        return { object, false };
    }

    if (contentChanged)
    {
        ++context.modifiedStreamCount;
    }

    return { createCompressedContentStream(stream, qMove(sanitizedContent), qMove(dictionary)), true };
}

SanitizedObjectResult PDFInvisibleTextSanitizerHelper::sanitizeResourcesObject(PDFDocumentBuilder& builder,
                                                                               const PDFObject& resourcesObject,
                                                                               InvisibleTextSanitizationContext& context)
{
    if (resourcesObject.isReference())
    {
        const PDFObjectReference reference = resourcesObject.getReference();
        if (!context.processedResourceObjects.insert(reference).second)
        {
            return { resourcesObject, false };
        }

        const PDFObject referencedObject = builder.getObjectByReference(reference);
        SanitizedObjectResult result = sanitizeResourcesObject(builder, referencedObject, context);
        if (result.changed)
        {
            builder.setObject(reference, qMove(result.object));
        }
        return { resourcesObject, false };
    }

    const PDFDictionary* resourcesDictionary = builder.getDictionaryFromObject(resourcesObject);
    if (!resourcesDictionary || !resourcesDictionary->hasKey("XObject"))
    {
        return { resourcesObject, false };
    }

    PDFObject xobjectObject = builder.getObject(resourcesDictionary->get("XObject"));
    const PDFDictionary* xobjectDictionary = builder.getDictionaryFromObject(xobjectObject);
    if (!xobjectDictionary)
    {
        return { resourcesObject, false };
    }

    PDFDictionary updatedResourcesDictionary(*resourcesDictionary);
    PDFDictionary updatedXObjectDictionary(*xobjectDictionary);
    bool changed = false;

    // Walk XObjects recursively because invisible OCR text is often stored in
    // nested Form XObjects instead of the page content stream itself.
    for (size_t i = 0, count = xobjectDictionary->getCount(); i < count; ++i)
    {
        const PDFInplaceOrMemoryString key = xobjectDictionary->getKey(i);
        const PDFObject currentObject = xobjectDictionary->getValue(i);
        SanitizedObjectResult result = sanitizeFormXObject(builder, currentObject, context);
        if (result.changed)
        {
            updatedXObjectDictionary.setEntry(key, qMove(result.object));
            changed = true;
        }
    }

    if (!changed)
    {
        return { resourcesObject, false };
    }

    updatedResourcesDictionary.setEntry(PDFInplaceOrMemoryString("XObject"),
                                        PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(updatedXObjectDictionary))));
    return { PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(updatedResourcesDictionary))), true };
}

SanitizedObjectResult PDFInvisibleTextSanitizerHelper::sanitizeContentsObject(PDFDocumentBuilder& builder,
                                                                              const PDFObject& contentsObject,
                                                                              InvisibleTextSanitizationContext& context)
{
    if (contentsObject.isReference())
    {
        const PDFObjectReference reference = contentsObject.getReference();
        if (!context.processedContentObjects.insert(reference).second)
        {
            return { contentsObject, false };
        }

        const PDFObject referencedObject = builder.getObjectByReference(reference);
        SanitizedObjectResult result = sanitizeContentsObject(builder, referencedObject, context);
        if (result.changed)
        {
            builder.setObject(reference, qMove(result.object));
        }
        return { contentsObject, false };
    }

    if (contentsObject.isArray())
    {
        PDFArray updatedArray(*contentsObject.getArray());
        bool changed = false;

        for (size_t i = 0, count = updatedArray.getCount(); i < count; ++i)
        {
            SanitizedObjectResult result = sanitizeContentsObject(builder, updatedArray.getItem(i), context);
            if (result.changed)
            {
                updatedArray.setItem(qMove(result.object), i);
                changed = true;
            }
        }

        return { changed ? PDFObject::createArray(std::make_shared<PDFArray>(qMove(updatedArray))) : contentsObject, changed };
    }

    if (!contentsObject.isStream())
    {
        return { contentsObject, false };
    }

    const PDFStream* stream = contentsObject.getStream();
    const QByteArray decodedContent = builder.getDecodedStream(stream);
    bool changed = false;
    QByteArray sanitizedContent = sanitizeInvisibleTextInContent(decodedContent, &changed);

    if (!changed)
    {
        return { contentsObject, false };
    }

    ++context.modifiedStreamCount;
    return { createCompressedContentStream(stream, qMove(sanitizedContent), PDFDictionary(*stream->getDictionary())), true };
}

class PDFRemoveMetadataVisitor : public PDFUpdateObjectVisitor
{
public:
    explicit PDFRemoveMetadataVisitor(const PDFObjectStorage* storage, std::atomic<PDFInteger>* counter) :
        PDFUpdateObjectVisitor(storage),
        m_counter(counter)
    {

    }

    virtual void visitDictionary(const PDFDictionary* dictionary) override;

private:
    std::atomic<PDFInteger>* m_counter;
};

void PDFRemoveMetadataVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    std::vector<PDFDictionary::DictionaryEntry> entries;
    entries.reserve(dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
        Q_ASSERT(!m_objectStack.empty());
        if (dictionary->getKey(i) != "Metadata")
        {
            entries.emplace_back(dictionary->getKey(i), m_objectStack.back());
        }
        else
        {
            ++*m_counter;
        }
        m_objectStack.pop_back();
    }

    m_objectStack.push_back(PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(entries))));
}

PDFDocumentSanitizer::PDFDocumentSanitizer(SanitizationFlag flags, QObject* parent) :
    QObject(parent),
    m_flags(flags)
{

}

void PDFDocumentSanitizer::sanitize()
{
    Q_EMIT sanitizationStarted();

    if (m_flags.testFlag(DocumentInfo))
    {
        performSanitizeDocumentInfo();
    }

    if (m_flags.testFlag(Metadata))
    {
        performSanitizeMetadata();
    }

    if (m_flags.testFlag(Outline))
    {
        performSanitizeOutline();
    }

    if (m_flags.testFlag(FileAttachments))
    {
        performSanitizeFileAttachments();
    }

    if (m_flags.testFlag(EmbeddedSearchIndex))
    {
        performSanitizeEmbeddedSearchIndex();
    }

    if (m_flags.testFlag(MarkupAnnotations))
    {
        performSanitizeMarkupAnnotations();
    }

    if (m_flags.testFlag(PageThumbnails))
    {
        performSanitizePageThumbnails();
    }

    if (m_flags.testFlag(PageLabels))
    {
        performSanitizePageLabels();
    }

    if (m_flags.testFlag(InvisibleText))
    {
        performSanitizeInvisibleText();
    }

    // Optimize - remove unused objects
    PDFOptimizer optimizer(PDFOptimizer::OptimizationFlags(PDFOptimizer::RemoveUnusedObjects | PDFOptimizer::ShrinkObjectStorage | PDFOptimizer::RemoveNullObjects), nullptr);
    optimizer.setStorage(m_storage);
    optimizer.optimize();
    m_storage = optimizer.takeStorage();

    Q_EMIT sanitizationFinished();
}

PDFDocumentSanitizer::SanitizationFlags PDFDocumentSanitizer::getFlags() const
{
    return m_flags;
}

void PDFDocumentSanitizer::setFlags(SanitizationFlags flags)
{
    m_flags = flags;
}

void PDFDocumentSanitizer::performSanitizeDocumentInfo()
{
    PDFObjectReference emptyDocumentInfoReference = m_storage.addObject(PDFObject());

    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    const bool hasDocumentInfo = builder.getDocumentInfo().isValid();
    builder.setDocumentInfo(emptyDocumentInfoReference);
    PDFDocument document = builder.build();
    m_storage = document.getStorage();

    if (hasDocumentInfo)
    {
        Q_EMIT sanitizationProgress(tr("Document info was removed."));
    }
}

void PDFDocumentSanitizer::performSanitizeMetadata()
{
    std::atomic<PDFInteger> counter = 0;

    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    auto processEntry = [this, &counter](PDFObjectStorage::Entry& entry)
    {
        PDFRemoveMetadataVisitor visitor(&m_storage, &counter);
        entry.object.accept(&visitor);
        entry.object = visitor.getObject();
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objects.begin(), objects.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT sanitizationProgress(tr("Metadata streams removed: %1").arg(counter.load()));
}

void PDFDocumentSanitizer::performSanitizeOutline()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasOutline = catalogDictionary && catalogDictionary->hasKey("Outlines");

    if (hasOutline)
    {
        builder.removeOutline();
        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Outline was removed."));
    }
}

void PDFDocumentSanitizer::performSanitizeFileAttachments()
{
    auto filter = [](const PDFAnnotation* annotation)
    {
        return annotation->getType() == AnnotationType::FileAttachment;
    };
    removeAnnotations(filter, tr("File attachments removed: %1."));

    // Remove files in name tree
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasNames = catalogDictionary && catalogDictionary->hasKey("Names");

    if (hasNames)
    {
        PDFObject namesObject = builder.getObject(catalogDictionary->get("Names"));
        const PDFDictionary* namesDictionary = builder.getDictionaryFromObject(namesObject);
        if (namesDictionary->hasKey("EmbeddedFiles"))
        {
            PDFDictionary dictionaryCopy = *namesDictionary;
            dictionaryCopy.setEntry(PDFInplaceOrMemoryString("EmbeddedFiles"), PDFObject());
            namesObject = PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(dictionaryCopy)));

            PDFObjectFactory factory;
            factory.beginDictionary();
            factory.beginDictionaryItem("Names");
            factory << namesObject;
            factory.endDictionaryItem();
            factory.endDictionary();
            PDFObject newCatalog = factory.takeObject();
            builder.mergeTo(builder.getCatalogReference(), std::move(newCatalog));
            PDFDocument document = builder.build();
            m_storage = document.getStorage();
            Q_EMIT sanitizationProgress(tr("Embedded files were removed."));
        }
    }
}

void PDFDocumentSanitizer::performSanitizeEmbeddedSearchIndex()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasPieceInfo = catalogDictionary && catalogDictionary->hasKey("PieceInfo");

    if (hasPieceInfo)
    {
        PDFObject pieceInfoObject = builder.getObject(catalogDictionary->get("PieceInfo"));
        const PDFDictionary* pieceInfoDictionary = builder.getDictionaryFromObject(pieceInfoObject);
        if (pieceInfoDictionary->hasKey("SearchIndex"))
        {
            PDFDictionary dictionaryCopy = *pieceInfoDictionary;
            dictionaryCopy.setEntry(PDFInplaceOrMemoryString("SearchIndex"), PDFObject());
            pieceInfoObject = PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(dictionaryCopy)));

            PDFObjectFactory factory;
            factory.beginDictionary();
            factory.beginDictionaryItem("PieceInfo");
            factory << pieceInfoObject;
            factory.endDictionaryItem();
            factory.endDictionary();
            PDFObject newCatalog = factory.takeObject();
            builder.mergeTo(builder.getCatalogReference(), std::move(newCatalog));
            PDFDocument document = builder.build();
            m_storage = document.getStorage();
            Q_EMIT sanitizationProgress(tr("Search index was removed."));
        }
    }
}

void PDFDocumentSanitizer::performSanitizeMarkupAnnotations()
{
    auto filter = [](const PDFAnnotation* annotation)
    {
        return annotation->asMarkupAnnotation() != nullptr;
    };
    removeAnnotations(filter, tr("Markup annotations removed: %1."));
}

void PDFDocumentSanitizer::performSanitizePageThumbnails()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    builder.flattenPageTree();
    std::vector<PDFObjectReference> pageReferences = builder.getPages();
    std::vector<PDFObjectReference> pagesWithThumbnail;

    for (const PDFObjectReference& pageReference : pageReferences)
    {
        const PDFDictionary* pageDictionary = builder.getDictionaryFromObject(builder.getObjectByReference(pageReference));
        if (pageDictionary && pageDictionary->hasKey("Thumb"))
        {
            pagesWithThumbnail.push_back(pageReference);
        }
    }

    if (!pagesWithThumbnail.empty())
    {
        for (const auto& pageReference : pagesWithThumbnail)
        {
            builder.removePageThumbnail(pageReference);
        }

        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Page thumbnails removed: %1.").arg(pagesWithThumbnail.size()));
    }
}

void PDFDocumentSanitizer::performSanitizePageLabels()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasPageLabels = catalogDictionary && catalogDictionary->hasKey("PageLabels");

    if (hasPageLabels)
    {
        PDFObjectFactory objectBuilder;
        objectBuilder.beginDictionary();
        objectBuilder.beginDictionaryItem("PageLabels");
        objectBuilder << PDFObject();
        objectBuilder.endDictionaryItem();
        objectBuilder.endDictionary();

        builder.mergeTo(builder.getCatalogReference(), objectBuilder.takeObject());
        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Page labels were removed."));
    }
}

void PDFDocumentSanitizer::performSanitizeInvisibleText()
{
    // This sanitizer intentionally performs a targeted low-level rewrite of page
    // and form content streams. It removes only invisible text operators used by
    // OCR layers and leaves the rest of the page description intact.
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    builder.flattenPageTree();
    const std::vector<PDFObjectReference> pageReferences = builder.getPages();

    PDFInvisibleTextSanitizerHelper::InvisibleTextSanitizationContext context;
    bool changed = false;

    for (const PDFObjectReference& pageReference : pageReferences)
    {
        const PDFObject pageObject = builder.getObjectByReference(pageReference);
        const PDFDictionary* pageDictionary = builder.getDictionaryFromObject(pageObject);
        if (!pageDictionary)
        {
            continue;
        }

        PDFDictionary updatedPageDictionary(*pageDictionary);
        bool pageChanged = false;

        if (pageDictionary->hasKey("Contents"))
        {
            SanitizedObjectResult result = PDFInvisibleTextSanitizerHelper::sanitizeContentsObject(builder, pageDictionary->get("Contents"), context);
            if (result.changed)
            {
                updatedPageDictionary.setEntry(PDFInplaceOrMemoryString("Contents"), qMove(result.object));
                pageChanged = true;
            }
        }

        if (pageDictionary->hasKey("Resources"))
        {
            SanitizedObjectResult result = PDFInvisibleTextSanitizerHelper::sanitizeResourcesObject(builder, pageDictionary->get("Resources"), context);
            if (result.changed)
            {
                updatedPageDictionary.setEntry(PDFInplaceOrMemoryString("Resources"), qMove(result.object));
                pageChanged = true;
            }
        }

        if (pageChanged)
        {
            builder.setObject(pageReference, PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(updatedPageDictionary))));
            changed = true;
        }
    }

    if (changed || context.modifiedStreamCount > 0)
    {
        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Invisible text content streams sanitized: %1.").arg(context.modifiedStreamCount));
    }
}

void PDFDocumentSanitizer::removeAnnotations(const std::function<bool (const PDFAnnotation*)>& filter,
                                             QString message)
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    builder.flattenPageTree();
    std::vector<PDFObjectReference> pageReferences = builder.getPages();
    std::vector<std::pair<PDFObjectReference, PDFObjectReference>> annotationsToBeRemoved;

    PDFDocumentDataLoaderDecorator loader(&m_storage);
    for (const PDFObjectReference pageReference : pageReferences)
    {
        const PDFObject& pageObject = m_storage.getObjectByReference(pageReference);
        const PDFDictionary* pageDictionary = m_storage.getDictionaryFromObject(pageObject);

        if (!pageDictionary)
        {
            continue;
        }

        std::vector<PDFObjectReference> annotationReferences = loader.readReferenceArrayFromDictionary(pageDictionary, "Annots");
        for (const PDFObjectReference& annotationReference : annotationReferences)
        {
            PDFAnnotationPtr annotation = PDFAnnotation::parse(&m_storage, annotationReference);
            if (filter(annotation.get()))
            {
                annotationsToBeRemoved.emplace_back(pageReference, annotationReference);
            }
        }
    }

    if (!annotationsToBeRemoved.empty())
    {
        for (const auto& item : annotationsToBeRemoved)
        {
            const PDFObjectReference pageReference = item.first;
            const PDFObjectReference annotationReference = item.second;
            builder.removeAnnotation(pageReference, annotationReference);
        }

        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(message.arg(annotationsToBeRemoved.size()));
    }
}

}   // namespace pdf
