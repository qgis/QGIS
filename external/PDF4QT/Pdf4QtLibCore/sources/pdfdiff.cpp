//    Copyright (C) 2022 Jakub Melka
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

#include "pdfdiff.h"
#include "pdfrenderer.h"
#include "pdfdocumenttextflow.h"
#include "pdfexecutionpolicy.h"
#include "pdffont.h"
#include "pdfcms.h"
#include "pdfconstants.h"
#include "pdfalgorithmlcs.h"
#include "pdfpainter.h"

#include <QtConcurrent/QtConcurrent>

#include "pdfdbgheap.h"

namespace pdf
{

class PDFDiffHelper
{
public:
    using GraphicPieceInfo = PDFPrecompiledPage::GraphicPieceInfo;
    using GraphicPieceInfos = PDFPrecompiledPage::GraphicPieceInfos;
    using PageSequence = PDFAlgorithmLongestCommonSubsequenceBase::Sequence;


    struct Differences
    {
        GraphicPieceInfos left;
        GraphicPieceInfos right;

        bool isEmpty() const { return left.empty() && right.empty(); }
    };

    struct TextFlowDifferences
    {
        PDFDocumentTextFlow leftTextFlow;
        PDFDocumentTextFlow rightTextFlow;
        QString leftText;
        QString rightText;
    };

    struct TextCompareItem
    {
        size_t index = 0;
        int charIndex = 0;
        int charCount = 0;
        bool left = false;
    };

    static Differences calculateDifferences(const GraphicPieceInfos& left, const GraphicPieceInfos& right, PDFReal epsilon);
    static std::vector<size_t> getLeftUnmatched(const PageSequence& sequence);
    static std::vector<size_t> getRightUnmatched(const PageSequence& sequence);
    static void matchPage(PageSequence& sequence, size_t leftPage, size_t rightPage);
    static std::vector<TextCompareItem> prepareTextCompareItems(const PDFDocumentTextFlow& textFlow,
                                                                bool isWordsComparingMode,
                                                                bool isLeft);
    static void refineTextRectangles(PDFDiffResult::RectInfos& items);
};

PDFDiff::PDFDiff(QObject* parent) :
    BaseClass(parent),
    m_progress(nullptr),
    m_leftDocument(nullptr),
    m_rightDocument(nullptr),
    m_options(Asynchronous | PC_Text | PC_VectorGraphics | PC_Images | CompareWords),
    m_epsilon(0.001),
    m_cancelled(false),
    m_textAnalysisAlgorithm(PDFDocumentTextFlowFactory::Algorithm::Layout)
{

}

PDFDiff::~PDFDiff()
{
    stop();
}

void PDFDiff::setLeftDocument(const PDFDocument* leftDocument)
{
    if (m_leftDocument != leftDocument)
    {
        stop();
        m_leftDocument = leftDocument;
    }
}

void PDFDiff::setRightDocument(const PDFDocument* rightDocument)
{
    if (m_rightDocument != rightDocument)
    {
        stop();
        m_rightDocument = rightDocument;
    }
}

void PDFDiff::setPagesForLeftDocument(PDFClosedIntervalSet pagesForLeftDocument)
{
    stop();
    m_pagesForLeftDocument = std::move(pagesForLeftDocument);
}

void PDFDiff::setPagesForRightDocument(PDFClosedIntervalSet pagesForRightDocument)
{
    stop();
    m_pagesForRightDocument = std::move(pagesForRightDocument);
}

void PDFDiff::start()
{
    // Jakub Melka: First, we must ensure, that comparation
    // process is finished, otherwise we must wait for end.
    // Then, create a new future watcher.
    stop();

    m_cancelled = false;

    if (m_options.testFlag(Asynchronous))
    {
        m_futureWatcher = std::nullopt;
        m_futureWatcher.emplace();

        m_future = QtConcurrent::run(std::bind(&PDFDiff::perform, this));
        connect(&*m_futureWatcher, &QFutureWatcher<PDFDiffResult>::finished, this, &PDFDiff::onComparationPerformed);
        m_futureWatcher->setFuture(m_future);
    }
    else
    {
        // Just do comparation immediately
        m_result = perform();
        Q_EMIT comparationFinished();
    }
}

void PDFDiff::stop()
{
    if (m_futureWatcher && !m_futureWatcher->isFinished())
    {
        // Do stop only if process doesn't finished already.
        // If we are finished, we do not want to set cancelled state.
        m_cancelled = true;
        m_futureWatcher->waitForFinished();
    }
}

PDFDiffResult PDFDiff::perform()
{
    PDFDiffResult result;

    if (!m_leftDocument || !m_rightDocument)
    {
        result.setResult(tr("No document to be compared."));
        return result;
    }

    if (m_pagesForLeftDocument.isEmpty() || m_pagesForRightDocument.isEmpty())
    {
        result.setResult(tr("No page to be compared."));
        return result;
    }

    auto leftPages = m_pagesForLeftDocument.unfold();
    auto rightPages = m_pagesForRightDocument.unfold();

    const size_t leftDocumentPageCount = m_leftDocument->getCatalog()->getPageCount();
    const size_t rightDocumentPageCount = m_rightDocument->getCatalog()->getPageCount();

    if (leftPages.front() < 0 ||
        leftPages.back() >= PDFInteger(leftDocumentPageCount) ||
        rightPages.front() < 0 ||
        rightPages.back() >= PDFInteger(rightDocumentPageCount))
    {
        result.setResult(tr("Invalid page range."));
        return result;
    }

    if (m_progress)
    {
        ProgressStartupInfo info;
        info.showDialog = false;
        info.text = tr("Comparing documents.");
        m_progress->start(StepLast, std::move(info));
    }

    performSteps(leftPages, rightPages, result);

    if (m_progress)
    {
        m_progress->finish();
    }

    return result;
}

void PDFDiff::stepProgress()
{
    if (m_progress)
    {
        m_progress->step();
    }
}

struct PDFDiffPageContext
{
    PDFInteger pageIndex = 0;
    std::array<uint8_t, 64> pageHash = { };
    PDFPrecompiledPage::GraphicPieceInfos graphicPieces;
    PDFDocumentTextFlow text;
};

void PDFDiff::performPageMatching(const std::vector<PDFDiffPageContext>& leftPreparedPages,
                                  const std::vector<PDFDiffPageContext>& rightPreparedPages,
                                  PDFAlgorithmLongestCommonSubsequenceBase::Sequence& pageSequence,
                                  std::map<size_t, size_t>& pageMatches)
{
    // Match pages. We will use following algorithm: exact solution can fail, because
    // we are using hashes and due to numerical instability, hashes can be different
    // even for exactly the same page. But if hashes are the same, the page must be the same.
    // So, we use longest common subsequence algorithm to detect same page ranges,
    // and then we match the rest. We assume the number of failing pages is relatively small.

    auto comparePages = [&](const PDFDiffPageContext& left, const PDFDiffPageContext& right)
    {
        if (left.pageHash == right.pageHash)
        {
            return true;
        }

        auto it = pageMatches.find(left.pageIndex);
        if (it != pageMatches.cend())
        {
            return it->second == static_cast<size_t>(right.pageIndex);
        }

        return false;
    };
    PDFAlgorithmLongestCommonSubsequence algorithm(leftPreparedPages.cbegin(), leftPreparedPages.cend(),
                                                   rightPreparedPages.cbegin(), rightPreparedPages.cend(),
                                                   comparePages);
    algorithm.perform();
    pageSequence = algorithm.getSequence();

    std::vector<size_t> leftUnmatched = PDFDiffHelper::getLeftUnmatched(pageSequence);
    std::vector<size_t> rightUnmatched = PDFDiffHelper::getRightUnmatched(pageSequence);

    // We are matching left pages to the right ones
    std::map<size_t, std::vector<size_t>> matchedPages;

    for (const size_t index : leftUnmatched)
    {
        matchedPages[index] = std::vector<size_t>();
    }

    auto matchLeftPage = [&, this](size_t leftIndex)
    {
        const PDFDiffPageContext& leftPageContext = leftPreparedPages[leftIndex];

        auto page = m_leftDocument->getCatalog()->getPage(leftPageContext.pageIndex);
        PDFReal epsilon = calculateEpsilonForPage(page);

        for (const size_t rightIndex : rightUnmatched)
        {
            const PDFDiffPageContext& rightPageContext = rightPreparedPages[rightIndex];
            if (leftPageContext.graphicPieces.size() != rightPageContext.graphicPieces.size())
            {
                // Match cannot exist, graphic pieces have different size
                continue;
            }

            PDFDiffHelper::Differences differences = PDFDiffHelper::calculateDifferences(leftPageContext.graphicPieces, rightPageContext.graphicPieces, epsilon);

            if (differences.isEmpty())
            {
                // Jakub Melka: we have a match
                matchedPages[leftIndex].push_back(rightIndex);
            }
        }
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, leftUnmatched.begin(), leftUnmatched.end(), matchLeftPage);

    std::vector<size_t> leftPagesMoved;
    std::vector<size_t> rightPagesMoved;

    std::set<size_t> matchedRightPages;
    for (const auto& matchedPage : matchedPages)
    {
        for (size_t rightContextIndex : matchedPage.second)
        {
            if (!matchedRightPages.count(rightContextIndex))
            {
                matchedRightPages.insert(rightContextIndex);
                const PDFDiffPageContext& leftPageContext = leftPreparedPages[matchedPage.first];
                const PDFDiffPageContext& rightPageContext = rightPreparedPages[rightContextIndex];

                leftPagesMoved.push_back(leftPageContext.pageIndex);
                rightPagesMoved.push_back(rightPageContext.pageIndex);

                pageMatches[leftPageContext.pageIndex] = rightPageContext.pageIndex;
            }
        }
    }

    if (!pageMatches.empty())
    {
        algorithm.perform();
        pageSequence = algorithm.getSequence();
    }

    std::sort(leftPagesMoved.begin(), leftPagesMoved.end());
    std::sort(rightPagesMoved.begin(), rightPagesMoved.end());

    PDFAlgorithmLongestCommonSubsequenceBase::markSequence(pageSequence, leftPagesMoved, rightPagesMoved);
}

void PDFDiff::performSteps(const std::vector<PDFInteger>& leftPages,
                           const std::vector<PDFInteger>& rightPages,
                           PDFDiffResult& result)
{
    std::vector<PDFDiffPageContext> leftPreparedPages;
    std::vector<PDFDiffPageContext> rightPreparedPages;

    PDFDiffHelper::PageSequence pageSequence;
    std::map<size_t, size_t> pageMatches; // Indices are real page indices, not indices to page contexts

    auto createDiffPageContext = [](auto pageIndex)
    {
       PDFDiffPageContext context;
       context.pageIndex = pageIndex;
       return context;
    };
    std::transform(leftPages.cbegin(), leftPages.cend(), std::back_inserter(leftPreparedPages), createDiffPageContext);
    std::transform(rightPages.cbegin(), rightPages.cend(), std::back_inserter(rightPreparedPages), createDiffPageContext);

    // StepExtractContentLeftDocument
    if (!m_cancelled)
    {
        PDFFontCache fontCache(DEFAULT_FONT_CACHE_LIMIT, DEFAULT_REALIZED_FONT_CACHE_LIMIT);
        PDFOptionalContentActivity optionalContentActivity(m_leftDocument, pdf::OCUsage::View, nullptr);
        fontCache.setDocument(pdf::PDFModifiedDocument(const_cast<pdf::PDFDocument*>(m_leftDocument), &optionalContentActivity));

        PDFCMSManager cmsManager(nullptr);
        cmsManager.setDocument(m_leftDocument);
        PDFCMSPointer cms = cmsManager.getCurrentCMS();

        auto fillPageContext = [&, this](PDFDiffPageContext& context)
        {
            PDFPrecompiledPage compiledPage;
            constexpr PDFRenderer::Features features = PDFRenderer::IgnoreOptionalContent;
            PDFRenderer renderer(m_leftDocument, &fontCache, cms.data(), &optionalContentActivity, features, pdf::PDFMeshQualitySettings());
            renderer.compile(&compiledPage, context.pageIndex);

            auto page = m_leftDocument->getCatalog()->getPage(context.pageIndex);
            PDFReal epsilon = calculateEpsilonForPage(page);
            context.graphicPieces = compiledPage.calculateGraphicPieceInfos(page->getMediaBox(), epsilon);

            finalizeGraphicsPieces(context);
        };
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, leftPreparedPages.begin(), leftPreparedPages.end(), fillPageContext);
        stepProgress();
    }

    // StepExtractContentRightDocument
    if (!m_cancelled)
    {
        PDFFontCache fontCache(DEFAULT_FONT_CACHE_LIMIT, DEFAULT_REALIZED_FONT_CACHE_LIMIT);
        PDFOptionalContentActivity optionalContentActivity(m_rightDocument, pdf::OCUsage::View, nullptr);
        fontCache.setDocument(pdf::PDFModifiedDocument(const_cast<pdf::PDFDocument*>(m_rightDocument), &optionalContentActivity));

        PDFCMSManager cmsManager(nullptr);
        cmsManager.setDocument(m_rightDocument);
        PDFCMSPointer cms = cmsManager.getCurrentCMS();

        auto fillPageContext = [&, this](PDFDiffPageContext& context)
        {
            PDFPrecompiledPage compiledPage;
            constexpr PDFRenderer::Features features = PDFRenderer::IgnoreOptionalContent;
            PDFRenderer renderer(m_rightDocument, &fontCache, cms.data(), &optionalContentActivity, features, pdf::PDFMeshQualitySettings());
            renderer.compile(&compiledPage, context.pageIndex);

            const PDFPage* page = m_rightDocument->getCatalog()->getPage(context.pageIndex);
            PDFReal epsilon = calculateEpsilonForPage(page);
            context.graphicPieces = compiledPage.calculateGraphicPieceInfos(page->getMediaBox(), epsilon);

            finalizeGraphicsPieces(context);
        };

        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, rightPreparedPages.begin(), rightPreparedPages.end(), fillPageContext);
        stepProgress();
    }

    // StepMatchPages
    if (!m_cancelled)
    {
        performPageMatching(leftPreparedPages, rightPreparedPages, pageSequence, pageMatches);
        stepProgress();
    }

    // StepExtractTextLeftDocument
    if (!m_cancelled)
    {
        pdf::PDFDocumentTextFlowFactory factoryLeftDocumentTextFlow;
        factoryLeftDocumentTextFlow.setCalculateBoundingBoxes(true);
        PDFDocumentTextFlow leftTextFlow = factoryLeftDocumentTextFlow.create(m_leftDocument, leftPages, m_textAnalysisAlgorithm);
        std::map<PDFInteger, PDFDocumentTextFlow> splittedText = leftTextFlow.split(PDFDocumentTextFlow::Text);
        for (PDFDiffPageContext& leftContext : leftPreparedPages)
        {
            auto it = splittedText.find(leftContext.pageIndex);
            if (it != splittedText.cend())
            {
                leftContext.text = std::move(it->second);
                splittedText.erase(it);
            }
        }
        stepProgress();
    }

    // StepExtractTextRightDocument
    if (!m_cancelled)
    {
        pdf::PDFDocumentTextFlowFactory factoryRightDocumentTextFlow;
        factoryRightDocumentTextFlow.setCalculateBoundingBoxes(true);
        PDFDocumentTextFlow rightTextFlow = factoryRightDocumentTextFlow.create(m_rightDocument, rightPages, m_textAnalysisAlgorithm);
        std::map<PDFInteger, PDFDocumentTextFlow> splittedText = rightTextFlow.split(PDFDocumentTextFlow::Text);
        for (PDFDiffPageContext& rightContext : rightPreparedPages)
        {
            auto it = splittedText.find(rightContext.pageIndex);
            if (it != splittedText.cend())
            {
                rightContext.text = std::move(it->second);
                splittedText.erase(it);
            }
        }
        stepProgress();
    }

    // StepCompare
    if (!m_cancelled)
    {
        performCompare(leftPreparedPages, rightPreparedPages, pageSequence, pageMatches, result);
        stepProgress();
    }
}

void PDFDiff::performCompare(const std::vector<PDFDiffPageContext>& leftPreparedPages,
                             const std::vector<PDFDiffPageContext>& rightPreparedPages,
                             PDFAlgorithmLongestCommonSubsequenceBase::Sequence& pageSequence,
                             const std::map<size_t, size_t>& pageMatches,
                             PDFDiffResult& result)
{
    using AlgorithmLCS = PDFAlgorithmLongestCommonSubsequenceBase;

    auto modifiedRanges = AlgorithmLCS::getModifiedRanges(pageSequence);
    PDFDiffResult::PageSequence resultPageSequence;
    resultPageSequence.reserve(pageSequence.size());

    // First find all moved pages
    for (const AlgorithmLCS::SequenceItem& item : pageSequence)
    {
        if (item.isMovedLeft())
        {
            Q_ASSERT(pageMatches.contains(leftPreparedPages.at(item.index1).pageIndex));
            const PDFInteger leftIndex = leftPreparedPages[item.index1].pageIndex;
            const PDFInteger rightIndex = pageMatches.at(leftIndex);
            result.addPageMoved(leftIndex, rightIndex);
        }
        if (item.isMoved())
        {
            result.addPageMoved(leftPreparedPages[item.index1].pageIndex, rightPreparedPages[item.index2].pageIndex);
        }

        PDFDiffResult::PageSequenceItem pageSequenceItem;
        if (item.isLeftValid())
        {
            const PDFInteger leftIndex = leftPreparedPages[item.index1].pageIndex;
            pageSequenceItem.leftPage = leftIndex;
        }
        if (item.isRightValid())
        {
            const PDFInteger rightIndex = rightPreparedPages[item.index2].pageIndex;
            pageSequenceItem.rightPage = rightIndex;
        }
        resultPageSequence.emplace_back(pageSequenceItem);
    }
    result.setPageSequence(std::move(resultPageSequence));

    std::vector<PDFDiffHelper::TextFlowDifferences> textFlowDifferences;

    for (const auto& range : modifiedRanges)
    {
        AlgorithmLCS::SequenceItemFlags flags = AlgorithmLCS::collectFlags(range);

        const bool isAdded = flags.testFlag(AlgorithmLCS::Added);
        const bool isRemoved = flags.testFlag(AlgorithmLCS::Removed);
        const bool isReplaced = flags.testFlag(AlgorithmLCS::Replaced);

        Q_ASSERT(isAdded || isRemoved || isReplaced);

        // There are two cases. Some page content was replaced, or either
        // page range was added, or page range was removed.
        if (isReplaced)
        {
            PDFDocumentTextFlow leftTextFlow;
            PDFDocumentTextFlow rightTextFlow;

            const bool isTextComparedAsVectorGraphics = m_options.testFlag(CompareTextsAsVector);

            for (auto it = range.first; it != range.second; ++it)
            {
                const AlgorithmLCS::SequenceItem& item = *it;
                if (item.isReplaced() && item.isMatch())
                {
                    const PDFDiffPageContext& leftPageContext = leftPreparedPages[item.index1];
                    const PDFDiffPageContext& rightPageContext = rightPreparedPages[item.index2];

                    if (!isTextComparedAsVectorGraphics)
                    {
                        leftTextFlow.append(leftPageContext.text);
                        rightTextFlow.append(rightPageContext.text);
                    }

                    auto pageLeft = m_leftDocument->getCatalog()->getPage(leftPageContext.pageIndex);
                    auto pageRight = m_rightDocument->getCatalog()->getPage(rightPageContext.pageIndex);
                    PDFReal epsilon = (calculateEpsilonForPage(pageLeft) + calculateEpsilonForPage(pageRight)) * 0.5;

                    PDFDiffHelper::Differences differences = PDFDiffHelper::calculateDifferences(leftPageContext.graphicPieces, rightPageContext.graphicPieces, epsilon);

                    for (const PDFDiffHelper::GraphicPieceInfo& info : differences.left)
                    {
                        switch (info.type)
                        {
                            case PDFDiffHelper::GraphicPieceInfo::Type::Text:
                                if (isTextComparedAsVectorGraphics)
                                {
                                    result.addRemovedTextCharContent(leftPageContext.pageIndex, info.boundingRect);
                                }
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::VectorGraphics:
                                result.addRemovedVectorGraphicContent(leftPageContext.pageIndex, info.boundingRect);
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::Image:
                                result.addRemovedImageContent(leftPageContext.pageIndex, info.boundingRect);
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::Shading:
                                result.addRemovedShadingContent(leftPageContext.pageIndex, info.boundingRect);
                                break;

                            default:
                                Q_ASSERT(false);
                                break;
                        }
                    }

                    for (const PDFDiffHelper::GraphicPieceInfo& info : differences.right)
                    {
                        switch (info.type)
                        {
                            case PDFDiffHelper::GraphicPieceInfo::Type::Text:
                                if (isTextComparedAsVectorGraphics)
                                {
                                    result.addAddedTextCharContent(rightPageContext.pageIndex, info.boundingRect);
                                }
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::VectorGraphics:
                                result.addAddedVectorGraphicContent(rightPageContext.pageIndex, info.boundingRect);
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::Image:
                                result.addAddedImageContent(rightPageContext.pageIndex, info.boundingRect);
                                break;

                            case PDFDiffHelper::GraphicPieceInfo::Type::Shading:
                                result.addAddedShadingContent(rightPageContext.pageIndex, info.boundingRect);
                                break;

                            default:
                                Q_ASSERT(false);
                                break;
                        }
                    }
                }

                if (item.isAdded())
                {
                    const PDFDiffPageContext& rightPageContext = rightPreparedPages[item.index2];

                    if (!isTextComparedAsVectorGraphics)
                    {
                        rightTextFlow.append(rightPageContext.text);
                    }

                    result.addPageAdded(rightPageContext.pageIndex);
                }
                if (item.isRemoved())
                {
                    const PDFDiffPageContext& leftPageContext = leftPreparedPages[item.index1];

                    if (!isTextComparedAsVectorGraphics)
                    {
                        leftTextFlow.append(leftPageContext.text);
                    }

                    result.addPageRemoved(leftPageContext.pageIndex);
                }
            }

            textFlowDifferences.emplace_back();
            PDFDiffHelper::TextFlowDifferences& addedDifferences = textFlowDifferences.back();
            addedDifferences.leftText = leftTextFlow.getText();
            addedDifferences.rightText = rightTextFlow.getText();

            if (addedDifferences.leftText == addedDifferences.rightText)
            {
                // Text is the same, no difference is found
                textFlowDifferences.pop_back();
            }
            else
            {
                addedDifferences.leftTextFlow = std::move(leftTextFlow);
                addedDifferences.rightTextFlow = std::move(rightTextFlow);
            }
        }
        else
        {
            for (auto it = range.first; it != range.second; ++it)
            {
                const AlgorithmLCS::SequenceItem& item = *it;
                Q_ASSERT(item.isAdded() || item.isRemoved());

                if (item.isAdded())
                {
                    result.addPageAdded(rightPreparedPages[item.index2].pageIndex);
                }
                if (item.isRemoved())
                {
                    result.addPageRemoved(leftPreparedPages[item.index1].pageIndex);
                }
            }
        }
    }

    QMutex mutex;

    // Jakub Melka: try to compare text differences
    auto compareTexts = [this, &mutex, &result](PDFDiffHelper::TextFlowDifferences& context)
    {
        using TextCompareItem = PDFDiffHelper::TextCompareItem;
        const bool isWordsComparingMode = m_options.testFlag(CompareWords);

        std::vector<TextCompareItem> leftItems;
        std::vector<TextCompareItem> rightItems;

        leftItems = PDFDiffHelper::prepareTextCompareItems(context.leftTextFlow, isWordsComparingMode, true);
        rightItems = PDFDiffHelper::prepareTextCompareItems(context.rightTextFlow, isWordsComparingMode, false);

        auto compareCharacters = [&](const TextCompareItem& a, const TextCompareItem& b)
        {
            const auto& aItem = a.left ? context.leftTextFlow : context.rightTextFlow;
            const auto& bItem = b.left ? context.leftTextFlow : context.rightTextFlow;

            QStringView aText(aItem.getItem(a.index)->text);
            aText = aText.mid(a.charIndex, a.charCount);

            QStringView bText(bItem.getItem(b.index)->text);
            bText = bText.mid(b.charIndex, b.charCount);

            return aText == bText;
        };
        PDFAlgorithmLongestCommonSubsequence algorithm(leftItems.cbegin(), leftItems.cend(),
                                                       rightItems.cbegin(), rightItems.cend(),
                                                       compareCharacters);
        algorithm.perform();
        PDFAlgorithmLongestCommonSubsequenceBase::Sequence sequence = algorithm.getSequence();
        PDFAlgorithmLongestCommonSubsequenceBase::markSequence(sequence, { }, { });
        PDFAlgorithmLongestCommonSubsequenceBase::SequenceItemRanges modifiedRanges = PDFAlgorithmLongestCommonSubsequenceBase::getModifiedRanges(sequence);

        // Merge modified sequences separated by just space
        if (!isWordsComparingMode && !modifiedRanges.empty())
        {
            auto itPrev = sequence.end();
            for (const auto& range : modifiedRanges)
            {
                if (itPrev != sequence.end())
                {
                    auto itNext = range.first;

                    bool isReplaced = true;
                    for (auto it = itPrev; it != itNext && isReplaced; ++it)
                    {
                        const PDFAlgorithmLongestCommonSubsequenceBase::SequenceItem& item = *it;

                        // If we doesn't have a match, then it is not a whitespace
                        if (!item.isMatch())
                        {
                            isReplaced = false;
                            break;
                        }

                        const TextCompareItem& compareItem = leftItems[item.index1];
                        const auto& flowItem = compareItem.left ? context.leftTextFlow : context.rightTextFlow;
                        QChar character = flowItem.getItem(compareItem.index)->text.at(compareItem.charIndex);

                        isReplaced = !character.isSpace();
                    }

                    if (isReplaced)
                    {
                        for (auto it = itPrev; it != itNext; ++it)
                        {
                            PDFAlgorithmLongestCommonSubsequenceBase::SequenceItem& item = *it;
                            item.markReplaced();
                        }
                    }
                }

                itPrev = range.second;
            }

            modifiedRanges = PDFAlgorithmLongestCommonSubsequenceBase::getModifiedRanges(sequence);
        }

        for (const auto& range : modifiedRanges)
        {
            auto it = range.first;
            auto itEnd = range.second;

            QStringList leftStrings;
            QStringList rightStrings;

            PDFDiffResult::RectInfos leftRectInfos;
            PDFDiffResult::RectInfos rightRectInfos;

            PDFInteger pageIndex1 = -1;
            PDFInteger pageIndex2 = -1;

            for (; it != itEnd; ++it)
            {
                const PDFAlgorithmLongestCommonSubsequenceBase::SequenceItem& item = *it;

                if (item.isLeftValid())
                {
                    const TextCompareItem& textCompareItem = leftItems[item.index1];
                    const auto& textFlow = textCompareItem.left ? context.leftTextFlow : context.rightTextFlow;
                    const PDFDocumentTextFlow::Item* textItem = textFlow.getItem(textCompareItem.index);

                    QStringView text(textItem->text);
                    text = text.mid(textCompareItem.charIndex, textCompareItem.charCount);

                    leftStrings << text.toString();

                    if (pageIndex1 == -1)
                    {
                        pageIndex1 = textItem->pageIndex;
                    }

                    if (static_cast< std::size_t >( textCompareItem.charIndex ) + textCompareItem.charCount <= textItem->characterBoundingRects.size())
                    {
                        const size_t startIndex =  textCompareItem.charIndex;
                        const size_t endIndex = startIndex + textCompareItem.charCount;

                        for (size_t i = startIndex; i < endIndex; ++i)
                        {
                            leftRectInfos.emplace_back(textItem->pageIndex, textItem->characterBoundingRects[i]);
                        }
                    }
                }

                if (item.isRightValid())
                {
                    const TextCompareItem& textCompareItem = rightItems[item.index2];
                    const auto& textFlow = textCompareItem.left ? context.leftTextFlow : context.rightTextFlow;
                    const PDFDocumentTextFlow::Item* textItem = textFlow.getItem(textCompareItem.index);

                    QStringView text(textItem->text);
                    text = text.mid(textCompareItem.charIndex, textCompareItem.charCount);

                    rightStrings << text.toString();

                    if (pageIndex2 == -1)
                    {
                        pageIndex2 = textItem->pageIndex;
                    }

                    if (static_cast< std::size_t >(textCompareItem.charIndex) + textCompareItem.charCount <= textItem->characterBoundingRects.size())
                    {
                        const size_t startIndex =  textCompareItem.charIndex;
                        const size_t endIndex = startIndex + textCompareItem.charCount;

                        for (size_t i = startIndex; i < endIndex; ++i)
                        {
                            rightRectInfos.emplace_back(textItem->pageIndex, textItem->characterBoundingRects[i]);
                        }
                    }
                }
            }

            QString leftString;
            QString rightString;

            if (isWordsComparingMode)
            {
                leftString = leftStrings.join(QChar::Space);
                rightString = rightStrings.join(QChar::Space);
            }
            else
            {
                leftString = leftStrings.join(QString());
                rightString = rightStrings.join(QString());
            }

            PDFDiffHelper::refineTextRectangles(leftRectInfos);
            PDFDiffHelper::refineTextRectangles(rightRectInfos);

            QMutexLocker locker(&mutex);
            if (!leftString.isEmpty() && !rightString.isEmpty())
            {
                result.addTextReplaced(pageIndex1, pageIndex2, leftString, rightString, leftRectInfos, rightRectInfos);
            }
            else
            {
                if (!leftString.isEmpty())
                {
                    result.addTextRemoved(pageIndex1, leftString, leftRectInfos);
                }

                if (!rightString.isEmpty())
                {
                    result.addTextAdded(pageIndex2, rightString, rightRectInfos);
                }
            }
        }
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Page, textFlowDifferences.begin(), textFlowDifferences.end(), compareTexts);

    // Jakub Melka: sort results
    result.finalize();
}

void PDFDiff::finalizeGraphicsPieces(PDFDiffPageContext& context)
{
    std::sort(context.graphicPieces.begin(), context.graphicPieces.end());

    // Compute page hash using active settings
    QCryptographicHash hasher(QCryptographicHash::Sha512);
    hasher.reset();

    for (const PDFPrecompiledPage::GraphicPieceInfo& info : context.graphicPieces)
    {
        if (info.isText() && !m_options.testFlag(PC_Text))
        {
            continue;
        }
        if (info.isVectorGraphics() && !m_options.testFlag(PC_VectorGraphics))
        {
            continue;
        }
        if (info.isImage() && !m_options.testFlag(PC_Images))
        {
            continue;
        }
        if (info.isShading() && !m_options.testFlag(PC_Mesh))
        {
            continue;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QByteArrayView view(reinterpret_cast<const char*>(info.hash.data()), info.hash.size());
        hasher.addData(view);
#else
        hasher.addData(reinterpret_cast<const char*>(info.hash.data()), info.hash.size());
#endif

    }

    QByteArray hash = hasher.result();
    Q_ASSERT(QCryptographicHash::hashLength(QCryptographicHash::Sha512) == 64);

    size_t size = qMin<size_t>(hash.length(), context.pageHash.size());
    std::copy(hash.data(), hash.data() + size, context.pageHash.data());
}

void PDFDiff::onComparationPerformed()
{
    m_cancelled = false;
    m_result = m_future.result();
    Q_EMIT comparationFinished();
}

PDFReal PDFDiff::calculateEpsilonForPage(const PDFPage* page) const
{
    Q_ASSERT(page);

    QRectF mediaBox = page->getMediaBox();

    PDFReal width = mediaBox.width();
    PDFReal height = mediaBox.height();
    PDFReal factor = qMax(width, height);

    return factor * m_epsilon;
}

PDFDocumentTextFlowFactory::Algorithm PDFDiff::getTextAnalysisAlgorithm() const
{
    return m_textAnalysisAlgorithm;
}

void PDFDiff::setTextAnalysisAlgorithm(PDFDocumentTextFlowFactory::Algorithm textAnalysisAlgorithm)
{
    m_textAnalysisAlgorithm = textAnalysisAlgorithm;
}

PDFDiffResult::PDFDiffResult() :
    m_result(true)
{

}

void PDFDiffResult::addPageMoved(PDFInteger pageIndex1, PDFInteger pageIndex2)
{
    Difference difference;

    difference.type = Type::PageMoved;
    difference.pageIndex1 = pageIndex1;
    difference.pageIndex2 = pageIndex2;

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addPageAdded(PDFInteger pageIndex)
{
    Difference difference;

    difference.type = Type::PageAdded;
    difference.pageIndex2 = pageIndex;

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addPageRemoved(PDFInteger pageIndex)
{
    Difference difference;

    difference.type = Type::PageRemoved;
    difference.pageIndex1 = pageIndex;

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addLeftItem(Type type, PDFInteger pageIndex, QRectF rect)
{
    Difference difference;

    difference.type = type;
    difference.pageIndex1 = pageIndex;
    addRectLeft(difference, rect);

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addRightItem(Type type, PDFInteger pageIndex, QRectF rect)
{
    Difference difference;

    difference.type = type;
    difference.pageIndex2 = pageIndex;
    addRectRight(difference, rect);

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addRemovedTextCharContent(PDFInteger pageIndex, QRectF rect)
{
    addLeftItem(Type::RemovedTextCharContent, pageIndex, rect);
}

void PDFDiffResult::addRemovedVectorGraphicContent(PDFInteger pageIndex, QRectF rect)
{
    addLeftItem(Type::RemovedVectorGraphicContent, pageIndex, rect);
}

void PDFDiffResult::addRemovedImageContent(PDFInteger pageIndex, QRectF rect)
{
    addLeftItem(Type::RemovedImageContent, pageIndex, rect);
}

void PDFDiffResult::addRemovedShadingContent(PDFInteger pageIndex, QRectF rect)
{
    addLeftItem(Type::RemovedShadingContent, pageIndex, rect);
}

void PDFDiffResult::addAddedTextCharContent(PDFInteger pageIndex, QRectF rect)
{
    addRightItem(Type::AddedTextCharContent, pageIndex, rect);
}

void PDFDiffResult::addAddedVectorGraphicContent(PDFInteger pageIndex, QRectF rect)
{
    addRightItem(Type::AddedVectorGraphicContent, pageIndex, rect);
}

void PDFDiffResult::addAddedImageContent(PDFInteger pageIndex, QRectF rect)
{
    addRightItem(Type::AddedImageContent, pageIndex, rect);
}

void PDFDiffResult::addAddedShadingContent(PDFInteger pageIndex, QRectF rect)
{
    addRightItem(Type::AddedShadingContent, pageIndex, rect);
}

void PDFDiffResult::addTextAdded(PDFInteger pageIndex,
                                 QString text,
                                 const RectInfos& rectInfos)
{
    Difference difference;

    difference.type = Type::TextAdded;
    difference.pageIndex2 = pageIndex;
    difference.textAddedIndex = m_strings.size();
    m_strings << text;
    difference.rightRectIndex = m_rects.size();
    difference.rightRectCount = rectInfos.size();
    m_rects.insert(m_rects.end(), rectInfos.cbegin(), rectInfos.cend());

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addTextRemoved(PDFInteger pageIndex,
                                   QString text,
                                   const RectInfos& rectInfos)
{
    Difference difference;

    difference.type = Type::TextRemoved;
    difference.pageIndex1 = pageIndex;
    difference.textRemovedIndex = m_strings.size();
    m_strings << text;
    difference.leftRectIndex = m_rects.size();
    difference.leftRectCount = rectInfos.size();
    m_rects.insert(m_rects.end(), rectInfos.cbegin(), rectInfos.cend());

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::addTextReplaced(PDFInteger pageIndex1,
                                    PDFInteger pageIndex2,
                                    QString textRemoved,
                                    QString textAdded,
                                    const RectInfos& rectInfos1,
                                    const RectInfos& rectInfos2)
{
    Difference difference;

    difference.type = Type::TextReplaced;
    difference.pageIndex1 = pageIndex1;
    difference.pageIndex2 = pageIndex2;
    difference.textRemovedIndex = m_strings.size();
    m_strings << textRemoved;
    difference.textAddedIndex = m_strings.size();
    m_strings << textAdded;

    difference.leftRectIndex = m_rects.size();
    difference.leftRectCount = rectInfos1.size();
    m_rects.insert(m_rects.end(), rectInfos1.cbegin(), rectInfos1.cend());

    difference.rightRectIndex = m_rects.size();
    difference.rightRectCount = rectInfos2.size();
    m_rects.insert(m_rects.end(), rectInfos2.cbegin(), rectInfos2.cend());

    m_differences.emplace_back(std::move(difference));
}

void PDFDiffResult::saveToStream(QXmlStreamWriter* stream) const
{
    stream->setAutoFormatting(true);
    stream->setAutoFormattingIndent(2);
    stream->writeStartDocument();
    stream->writeNamespace("https://github.com/JakubMelka/PDF4QT", "pdf4qt");
    stream->writeStartElement("difference-report");

    // Jakub Melka: write all differences
    stream->writeStartElement("differences");
    for (const Difference& difference : m_differences)
    {
        stream->writeStartElement("difference");

        QString type;
        switch (difference.type)
        {
            case Type::PageMoved:
                type = "page-moved";
                break;

            case Type::PageAdded:
                type = "page-added";
                break;

            case Type::PageRemoved:
                type = "page-removed";
                break;

            case Type::RemovedTextCharContent:
                type = "removed-text-char";
                break;

            case Type::RemovedVectorGraphicContent:
                type = "removed-vector-graphics";
                break;

            case Type::RemovedImageContent:
                type = "removed-image";
                break;

            case Type::RemovedShadingContent:
                type = "removed-shading";
                break;

            case Type::AddedTextCharContent:
                type = "added-text-char";
                break;

            case Type::AddedVectorGraphicContent:
                type = "added-vector-graphics";
                break;

            case Type::AddedImageContent:
                type = "added-image";
                break;

            case Type::AddedShadingContent:
                type = "added-shading";
                break;

            case Type::TextAdded:
                type = "text-added";
                break;

            case Type::TextRemoved:
                type = "text-removed";
                break;

            case Type::TextReplaced:
                type = "text-replaced";
                break;

            default:
                Q_ASSERT(false);
                break;
        }
        stream->writeAttribute("type", type);

        if (difference.pageIndex1 != -1)
        {
            stream->writeAttribute("left", QString::number(difference.pageIndex1 + 1));
        }

        if (difference.pageIndex2 != -1)
        {
            stream->writeAttribute("right", QString::number(difference.pageIndex2 + 1));
        }

        if (difference.textAddedIndex != -1)
        {
            stream->writeTextElement("text-added", m_strings[difference.textAddedIndex]);
        }

        if (difference.textRemovedIndex != -1)
        {
            stream->writeTextElement("text-removed", m_strings[difference.textRemovedIndex]);
        }

        stream->writeEndElement();
    }
    stream->writeEndElement();

    stream->writeStartElement("page-sequence");
    for (const PageSequenceItem& item : m_pageSequence)
    {
        stream->writeStartElement("item");

        QString left = item.leftPage != -1 ? QString::number(item.leftPage + 1) : QString("none");
        QString right = item.rightPage != -1 ? QString::number(item.rightPage + 1) : QString("none");

        stream->writeAttribute("left", left);
        stream->writeAttribute("right", right);

        stream->writeEndElement();
    }
    stream->writeEndElement();

    stream->writeEndElement();
    stream->writeEndDocument();
}

void PDFDiffResult::finalize()
{
    auto predicate = [](const Difference& l, const Difference& r)
    {
        return qMax(l.pageIndex1, l.pageIndex2) < qMax(r.pageIndex1, r.pageIndex2);
    };

    std::stable_sort(m_differences.begin(), m_differences.end(), predicate);

    m_typeFlags = 0;
    for (const Difference& difference : m_differences)
    {
        m_typeFlags |= static_cast<uint32_t>(difference.type);
    }
}

uint32_t PDFDiffResult::getTypeFlags(size_t index) const
{
    if (index >= m_differences.size())
    {
        return 0;
    }

    return uint32_t(m_differences[index].type);
}

QString PDFDiffResult::getMessage(size_t index) const
{
    if (index >= m_differences.size())
    {
        return QString();
    }

    const Difference& difference = m_differences[index];
    switch (difference.type)
    {
        case Type::PageMoved:
            return PDFDiff::tr("Page no. %1 was moved to a page no. %2.").arg(difference.pageIndex1 + 1).arg(difference.pageIndex2 + 1);

        case Type::PageAdded:
            return PDFDiff::tr("Page no. %1 was added.").arg(difference.pageIndex2 + 1);

        case Type::PageRemoved:
            return PDFDiff::tr("Page no. %1 was removed.").arg(difference.pageIndex1 + 1);

        case Type::RemovedTextCharContent:
            return PDFDiff::tr("Removed text character from page %1.").arg(difference.pageIndex1 + 1);

        case Type::RemovedVectorGraphicContent:
            return PDFDiff::tr("Removed vector graphics from page %1.").arg(difference.pageIndex1 + 1);

        case Type::RemovedImageContent:
            return PDFDiff::tr("Removed image from page %1.").arg(difference.pageIndex1 + 1);

        case Type::RemovedShadingContent:
            return PDFDiff::tr("Removed shading from page %1.").arg(difference.pageIndex1 + 1);

        case Type::AddedTextCharContent:
            return PDFDiff::tr("Added text character to page %1.").arg(difference.pageIndex2 + 1);

        case Type::AddedVectorGraphicContent:
            return PDFDiff::tr("Added vector graphics to page %1.").arg(difference.pageIndex2 + 1);

        case Type::AddedImageContent:
            return PDFDiff::tr("Added image to page %1.").arg(difference.pageIndex2 + 1);

        case Type::AddedShadingContent:
            return PDFDiff::tr("Added shading to page %1.").arg(difference.pageIndex2 + 1);

        case Type::TextAdded:
            return PDFDiff::tr("Text '%1' has been added to page %2.").arg(m_strings[difference.textAddedIndex]).arg(difference.pageIndex2 + 1);

        case Type::TextRemoved:
            return PDFDiff::tr("Text '%1' has been removed from page %2.").arg(m_strings[difference.textRemovedIndex]).arg(difference.pageIndex1 + 1);

        case Type::TextReplaced:
            return PDFDiff::tr("Text '%1' on page %2 has been replaced by text '%3' on page %4.").arg(m_strings[difference.textRemovedIndex]).arg(difference.pageIndex1 + 1).arg(m_strings[difference.textAddedIndex]).arg(difference.pageIndex2 + 1);

        default:
            Q_ASSERT(false);
            break;
    }

    return QString();
}

PDFInteger PDFDiffResult::getLeftPage(size_t index) const
{
    if (index >= m_differences.size())
    {
        return -1;
    }

    return m_differences[index].pageIndex1;
}

PDFInteger PDFDiffResult::getRightPage(size_t index) const
{
    if (index >= m_differences.size())
    {
        return -1;
    }

    return m_differences[index].pageIndex2;
}

PDFDiffResult::Type PDFDiffResult::getType(size_t index) const
{
    if (index >= m_differences.size())
    {
        return Type::Invalid;
    }

    return m_differences[index].type;
}

QString PDFDiffResult::getTypeDescription(size_t index) const
{
    switch (getType(index))
    {
        case Type::Invalid:
            return PDFDiff::tr("Invalid");
        case Type::PageMoved:
            return PDFDiff::tr("Page moved");
        case Type::PageAdded:
            return PDFDiff::tr("Page added");
        case Type::PageRemoved:
            return PDFDiff::tr("Page removed");
        case Type::RemovedTextCharContent:
            return PDFDiff::tr("Removed text character");
        case Type::RemovedVectorGraphicContent:
            return PDFDiff::tr("Removed vector graphics");
        case Type::RemovedImageContent:
            return PDFDiff::tr("Removed image");
        case Type::RemovedShadingContent:
            return PDFDiff::tr("Removed shading");
        case Type::AddedTextCharContent:
            return PDFDiff::tr("Added text character");
        case Type::AddedVectorGraphicContent:
            return PDFDiff::tr("Added vector graphics");
        case Type::AddedImageContent:
            return PDFDiff::tr("Added image");
        case Type::AddedShadingContent:
            return PDFDiff::tr("Added shading");
        case Type::TextAdded:
            return PDFDiff::tr("Text added");
        case Type::TextRemoved:
            return PDFDiff::tr("Text removed");
        case Type::TextReplaced:
            return PDFDiff::tr("Text replaced");

        default:
            Q_ASSERT(false);
            break;
    }

    return QString();
}

std::pair<PDFDiffResult::RectInfosIt, PDFDiffResult::RectInfosIt> PDFDiffResult::getLeftRectangles(size_t index) const
{
    if (index >= m_differences.size())
    {
        return std::make_pair(m_rects.cend(), m_rects.cend());
    }

    const Difference& difference = m_differences[index];
    if (difference.leftRectCount > 0)
    {
        auto it = std::next(m_rects.cbegin(), difference.leftRectIndex);
        auto itEnd = std::next(it, difference.leftRectCount);
        return std::make_pair(it, itEnd);
    }

    return std::make_pair(m_rects.cend(), m_rects.cend());
}

std::pair<PDFDiffResult::RectInfosIt, PDFDiffResult::RectInfosIt> PDFDiffResult::getRightRectangles(size_t index) const
{
    if (index >= m_differences.size())
    {
        return std::make_pair(m_rects.cend(), m_rects.cend());
    }

    const Difference& difference = m_differences[index];
    if (difference.rightRectCount > 0)
    {
        auto it = std::next(m_rects.cbegin(), difference.rightRectIndex);
        auto itEnd = std::next(it, difference.rightRectCount);
        return std::make_pair(it, itEnd);
    }

    return std::make_pair(m_rects.cend(), m_rects.cend());
}

bool PDFDiffResult::isPageMoveAddRemoveDifference(size_t index) const
{
    return getTypeFlags(index) & FLAGS_TYPE_PAGE_MOVE_ADD_REMOVE;
}

bool PDFDiffResult::isPageMoveDifference(size_t index) const
{
    return getTypeFlags(index) & FLAGS_TYPE_PAGE_MOVE;
}

bool PDFDiffResult::isAddDifference(size_t index) const
{
    return getTypeFlags(index) & FLAGS_TYPE_ADD;
}

bool PDFDiffResult::isRemoveDifference(size_t index) const
{
    return getTypeFlags(index) & FLAGS_TYPE_REMOVE;
}

bool PDFDiffResult::isReplaceDifference(size_t index) const
{
    return getTypeFlags(index) & FLAGS_TYPE_REPLACE;
}

std::vector<PDFInteger> PDFDiffResult::getChangedLeftPageIndices() const
{
    std::set<PDFInteger> changedPageIndices;

    for (size_t i = 0; i < m_differences.size(); ++i)
    {
        changedPageIndices.insert(getLeftPage(i));
    }
    changedPageIndices.erase(-1);

    return std::vector<PDFInteger>(changedPageIndices.cbegin(), changedPageIndices.cend());
}

std::vector<PDFInteger> PDFDiffResult::getChangedRightPageIndices() const
{
    std::set<PDFInteger> changedPageIndices;

    for (size_t i = 0; i < m_differences.size(); ++i)
    {
        changedPageIndices.insert(getRightPage(i));
    }
    changedPageIndices.erase(-1);

    return std::vector<PDFInteger>(changedPageIndices.cbegin(), changedPageIndices.cend());
}

PDFDiffResult PDFDiffResult::filter(bool filterPageMoveDifferences,
                                    bool filterTextDifferences,
                                    bool filterVectorGraphicsDifferences,
                                    bool filterImageDifferences,
                                    bool filterShadingDifferences)
{
    PDFDiffResult filteredResult = *this;

    uint32_t typeFlags = 0;

    if (filterPageMoveDifferences)
    {
        typeFlags |= FLAGS_PAGE_MOVE;
    }

    if (filterTextDifferences)
    {
        typeFlags |= FLAGS_TEXT;
    }

    if (filterVectorGraphicsDifferences)
    {
        typeFlags |= FLAGS_VECTOR_GRAPHICS;
    }

    if (filterImageDifferences)
    {
        typeFlags |= FLAGS_IMAGE;
    }

    if (filterShadingDifferences)
    {
        typeFlags |= FLAGS_SHADING;
    }

    auto remove = [typeFlags](const Difference& difference)
    {
        return (uint32_t(difference.type) & typeFlags) == 0;
    };
    filteredResult.m_differences.erase(std::remove_if(filteredResult.m_differences.begin(), filteredResult.m_differences.end(), remove), filteredResult.m_differences.end());

    return filteredResult;
}

void PDFDiffResult::addRectLeft(Difference& difference, QRectF rect)
{
    difference.leftRectIndex = m_rects.size();
    difference.leftRectCount = 1;
    m_rects.emplace_back(difference.pageIndex1, rect);
}

void PDFDiffResult::addRectRight(Difference& difference, QRectF rect)
{
    difference.rightRectIndex = m_rects.size();
    difference.rightRectCount = 1;
    m_rects.emplace_back(difference.pageIndex2, rect);
}

const PDFDiffResult::PageSequence& PDFDiffResult::getPageSequence() const
{
    return m_pageSequence;
}

void PDFDiffResult::setPageSequence(PageSequence pageSequence)
{
    m_pageSequence = pageSequence;
}

void PDFDiffResult::saveToXML(QIODevice* device) const
{
    QXmlStreamWriter stream(device);
    saveToStream(&stream);
}

void PDFDiffResult::saveToXML(QByteArray* byteArray) const
{
    QXmlStreamWriter stream(byteArray);
    saveToStream(&stream);
}

void PDFDiffResult::saveToXML(QString* string) const
{
    QXmlStreamWriter stream(string);
    saveToStream(&stream);
}

PDFDiffHelper::Differences PDFDiffHelper::calculateDifferences(const GraphicPieceInfos& left,
                                                               const GraphicPieceInfos& right,
                                                               PDFReal epsilon)
{
    Differences differences;

    Q_ASSERT(std::is_sorted(left.cbegin(), left.cend()));
    Q_ASSERT(std::is_sorted(right.cbegin(), right.cend()));

    for (const GraphicPieceInfo& info : left)
    {
        if (!std::binary_search(right.cbegin(), right.cend(), info))
        {
            differences.left.push_back(info);
        }
    }

    for (const GraphicPieceInfo& info : right)
    {
        if (!std::binary_search(left.cbegin(), left.cend(), info))
        {
            differences.right.push_back(info);
        }
    }

    const PDFReal epsilonSquared = epsilon * epsilon;

    // If exact match fails, then try to use match with epsilon. For each
    // item in left, we try to find matching item in right.
    for (auto it = differences.left.begin(); it != differences.left.end();)
    {
        bool hasMatch = false;

        const GraphicPieceInfo& leftInfo = *it;
        for (auto it2 = differences.right.begin(); it2 != differences.right.end();)
        {
            // Heuristically compare these items

            const GraphicPieceInfo& rightInfo = *it2;
            if (leftInfo.type != rightInfo.type || !leftInfo.boundingRect.intersects(rightInfo.boundingRect))
            {
                ++it2;
                continue;
            }

            const int elementCountPath1 = leftInfo.pagePath.elementCount();
            const int elementCountPath2 = rightInfo.pagePath.elementCount();

            if (elementCountPath1 != elementCountPath2)
            {
                ++it2;
                continue;
            }

            hasMatch = (leftInfo.type != GraphicPieceInfo::Type::Image) || (leftInfo.imageHash == rightInfo.imageHash);
            const int elementCount = leftInfo.pagePath.elementCount();
            for (int i = 0; i < elementCount && hasMatch; ++i)
            {
                QPainterPath::Element leftElement = leftInfo.pagePath.elementAt(i);
                QPainterPath::Element rightElement = rightInfo.pagePath.elementAt(i);

                PDFReal diffX = leftElement.x - rightElement.x;
                PDFReal diffY = leftElement.y - rightElement.y;
                PDFReal squaredDistance = diffX * diffX + diffY * diffY;

                hasMatch = (leftElement.type == rightElement.type) &&
                           (squaredDistance < epsilonSquared);
            }

            if (hasMatch)
            {
                it2 = differences.right.erase(it2);
            }
            else
            {
                ++it2;
            }
        }

        if (hasMatch)
        {
            it = differences.left.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return differences;
}

std::vector<size_t> PDFDiffHelper::getLeftUnmatched(const PageSequence& sequence)
{
    std::vector<size_t> result;

    for (const auto& item : sequence)
    {
        if (item.isLeft())
        {
            result.push_back(item.index1);
        }
    }

    return result;
}

std::vector<size_t> PDFDiffHelper::getRightUnmatched(const PageSequence& sequence)
{
    std::vector<size_t> result;

    for (const auto& item : sequence)
    {
        if (item.isRight())
        {
            result.push_back(item.index2);
        }
    }

    return result;
}

void PDFDiffHelper::matchPage(PageSequence& sequence,
                              size_t leftPage,
                              size_t rightPage)
{
    for (auto it = sequence.begin(); it != sequence.end();)
    {
        auto& item = *it;

        if (item.isLeft() && item.index1 == leftPage)
        {
            item.index2 = rightPage;
        }

        if (item.isRight() && item.index2 == rightPage)
        {
            it = sequence.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::vector<PDFDiffHelper::TextCompareItem> PDFDiffHelper::prepareTextCompareItems(const PDFDocumentTextFlow& textFlow,
                                                                                   bool isWordsComparingMode,
                                                                                   bool isLeft)
{
    std::vector<TextCompareItem> items;

    const size_t leftCount = textFlow.getSize();
    for (size_t i = 0; i < leftCount; ++i)
    {
        PDFDiffHelper::TextCompareItem item;
        item.index = i;
        item.left = isLeft;
        item.charCount = 0;

        const PDFDocumentTextFlow::Item* textFlowItem = textFlow.getItem(i);
        for (int j = 0; j < textFlowItem->text.size(); ++j)
        {
            if (isWordsComparingMode)
            {
                if (textFlowItem->text[j].isSpace())
                {
                    // Flush buffer
                    if (item.charCount > 0)
                    {
                        items.push_back(item);
                        item.charCount = 0;
                    }
                }
                else
                {
                    if (item.charCount == 0)
                    {
                        item.charIndex = j;
                    }
                    ++item.charCount;
                }
            }
            else
            {
                item.charIndex = j;
                item.charCount = 1;
                items.push_back(item);
            }
        }

        if (isWordsComparingMode && item.charCount > 0)
        {
            items.push_back(item);
            item.charCount = 0;
        }
    }

    return items;
}

void PDFDiffHelper::refineTextRectangles(PDFDiffResult::RectInfos& items)
{
    PDFDiffResult::RectInfos refinedItems;

    auto it = items.cbegin();
    auto itEnd = items.cend();
    while (it != itEnd)
    {
        // Jakub Melka: find range which can be merged into one
        // rectangle (it must be on a single page and rectangles must go
        // in right direction).

        auto itNext = std::next(it);
        while (itNext != itEnd)
        {
            const std::pair<PDFInteger, QRectF>& currentItem = *std::prev(itNext);
            const std::pair<PDFInteger, QRectF>& nextItem = *itNext;
            if (nextItem.first != currentItem.first)
            {
                // Page index has changed...
                break;
            }

            const QRectF& left = currentItem.second;
            const QRectF& right = nextItem.second;

            if (left.center().x() >= right.center().x())
            {
                break;
            }

            ++itNext;
        }

        // Merge range [it, itNext) into one new sequence
        QRectF unifiedRect;
        for (auto cit = it; cit != itNext; ++cit)
        {
            unifiedRect = unifiedRect.united((*cit).second);
        }
        refinedItems.emplace_back((*it).first, unifiedRect);

        it = itNext;
    }

    items = std::move(refinedItems);
}

PDFDiffResultNavigator::PDFDiffResultNavigator(QObject* parent) :
    QObject(parent),
    m_diffResult(nullptr),
    m_currentIndex(0)
{

}

PDFDiffResultNavigator::~PDFDiffResultNavigator()
{

}

void PDFDiffResultNavigator::setResult(const PDFDiffResult* diffResult)
{
    if (m_diffResult != diffResult)
    {
        m_diffResult = diffResult;
        Q_EMIT selectionChanged(m_currentIndex);
    }
}

bool PDFDiffResultNavigator::isSelected() const
{
    const size_t limit = getLimit();
    return m_currentIndex < limit;
}

bool PDFDiffResultNavigator::canGoNext() const
{
    const size_t limit = getLimit();
    return limit > 0 && m_currentIndex + 1 < limit;
}

bool PDFDiffResultNavigator::canGoPrevious() const
{
    const size_t limit = getLimit();
    return limit > 0 && m_currentIndex > 0;
}

void PDFDiffResultNavigator::goNext()
{
    if (!canGoNext())
    {
        return;
    }

    ++m_currentIndex;
    Q_EMIT selectionChanged(m_currentIndex);
}

void PDFDiffResultNavigator::goPrevious()
{
    if (!canGoPrevious())
    {
        return;
    }

    const size_t limit = getLimit();
    if (m_currentIndex >= limit)
    {
        m_currentIndex = limit - 1;
    }
    else
    {
        --m_currentIndex;
    }
    Q_EMIT selectionChanged(m_currentIndex);
}

void PDFDiffResultNavigator::update()
{
    const size_t limit = getLimit();
    if (limit > 0 && m_currentIndex >= limit)
    {
        m_currentIndex = limit - 1;
        Q_EMIT selectionChanged(m_currentIndex);
    }
}

void PDFDiffResultNavigator::select(size_t currentIndex)
{
    if (currentIndex < getLimit() && m_currentIndex != currentIndex)
    {
        m_currentIndex = currentIndex;
        Q_EMIT selectionChanged(m_currentIndex);
    }
}

}   // namespace pdf
