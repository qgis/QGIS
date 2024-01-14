//    Copyright (C) 2021 Jakub Melka
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
