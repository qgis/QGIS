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

#include "pdfplugin.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFPlugin::PDFPlugin(QObject* parent) :
    QObject(parent),
    m_dataExchangeInterface(nullptr),
    m_widget(nullptr),
    m_cmsManager(nullptr),
    m_document(nullptr)
{

}

void PDFPlugin::setDataExchangeInterface(IPluginDataExchange* dataExchangeInterface)
{
    m_dataExchangeInterface = dataExchangeInterface;
}

void PDFPlugin::setWidget(PDFWidget* widget)
{
    m_widget = widget;
}

void PDFPlugin::setCMSManager(PDFCMSManager* manager)
{
    m_cmsManager = manager;
}

void PDFPlugin::setDocument(const PDFModifiedDocument& document)
{
    m_document = document;
}

std::vector<QAction*> PDFPlugin::getActions() const
{
    return std::vector<QAction*>();
}

PDFPluginInfo PDFPluginInfo::loadFromJson(const QJsonObject* json)
{
    PDFPluginInfo result;

    QJsonObject metadata = json->value("MetaData").toObject();
    result.name = metadata.value(QLatin1String("Name")).toString();
    result.author = metadata.value(QLatin1String("Author")).toString();
    result.version = metadata.value(QLatin1String("Version")).toString();
    result.license = metadata.value(QLatin1String("License")).toString();
    result.description = metadata.value(QLatin1String("Description")).toString();

    return result;
}

}   // namespace pdf
