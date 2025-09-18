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

#ifndef PDFDOCUMENTTEXTFLOWEDITORMODEL_H
#define PDFDOCUMENTTEXTFLOWEDITORMODEL_H

#include "pdfglobal.h"
#include "pdfutils.h"

#include <QAbstractTableModel>

namespace pdf
{
class PDFDocumentTextFlowEditor;

class PDF4QTLIBCORESHARED_EXPORT PDFDocumentTextFlowEditorModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    using BaseClass = QAbstractTableModel;

public:
    PDFDocumentTextFlowEditorModel(QObject* parent);
    virtual ~PDFDocumentTextFlowEditorModel() override;

    enum Column
    {
        ColumnPageNo,
        ColumnType,
        ColumnState,
        ColumnText,
        ColumnLast
    };

    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    PDFDocumentTextFlowEditor* getEditor() const;
    void setEditor(PDFDocumentTextFlowEditor* editor);

    void beginFlowChange();
    void endFlowChange();

    void clear();
    void setSelectionActivated(bool activate);
    void selectByRectangle(QRectF rectangle);
    void selectByContainedText(QString text);
    void selectByRegularExpression(const QRegularExpression& expression);
    void selectByPageIndices(const pdf::PDFClosedIntervalSet& indices);
    void restoreOriginalTexts();
    void moveSelectionUp();
    void moveSelectionDown();
    void notifyDataChanged();

private:
    PDFDocumentTextFlowEditor* m_editor;
};

}   // namespace pdf

#endif // PDFDOCUMENTTEXTFLOWEDITORMODEL_H
