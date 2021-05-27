// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.7.2021.04.09

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/WICFileIONative.h>

// For access to the reference-counting COM interface wrapper. The header
// wrl.h includes windows.h, so we must turn off min and max macros.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <wincodec.h>
#include <wrl.h>
#include <algorithm>
#include <iterator>
using namespace gte;

class ComInitializer
{
public:
    ~ComInitializer()
    {
        if (mInitialized && SUCCEEDED(mHR))
        {
            ::CoUninitialize();
        }
    }

    ComInitializer()
        :
        mHR(S_OK),
        mInitialized(false)
    {
        mHR = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(mHR))
        {
            // The COM library was initialized successfully in the current
            // thread.
            mInitialized = true;
            return;
        }

        if (FAILED(mHR))
        {
            // The COM library was initialized previously in the current
            // thread, so it is safe to use.
            mInitialized = true;
            return;
        }

        // hr == E_INVALIDARG
        // hr == E_OUTOFMEMORY
        // hr == E_UNEXPECTED
        // hr == RPC_E_CHANGED_MODE. A previous call to CoInitializeEx
        // specified the COINIT_APARTMENTTHREADED.
    }

    inline HRESULT GetHResult() const
    {
        return mHR;
    }

    inline bool IsInitialized() const
    {
        return mInitialized;
    }

private:
    HRESULT mHR;
    bool mInitialized;
};

template <typename T>
using ComObject = Microsoft::WRL::ComPtr<T>;

void WICFileIONative::Load(std::string const& filename, uint32_t& outFormat,
    size_t& outWidth, size_t& outHeight, std::vector<uint8_t>& outTexels)
{
    auto const* bytesPerTexel = msBytesPerTexel.data();

    TextureCreator creator = [&outFormat, &outWidth, &outHeight, &outTexels, &bytesPerTexel](
        uint32_t format, size_t width, size_t height)
    {
        outFormat = format;
        outWidth = width;
        outHeight = height;
        outTexels.resize(width * height * bytesPerTexel[format]);
        return reinterpret_cast<uint8_t*>(outTexels.data());
    };

    WICFileIONative::Load(filename, creator);
}

void WICFileIONative::Load(std::string const& filename, TextureCreator const& creator)
{
    // Start COM and create WIC.
    ComInitializer comInitializer;
    if (!comInitializer.IsInitialized())
    {
        throw std::runtime_error("Unable to initialize COM for WIC.");
    }

    // Create a WIC imaging factory.
    ComObject<IWICImagingFactory> wicFactory;
    HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
        reinterpret_cast<LPVOID*>(wicFactory.GetAddressOf()));
    if (FAILED(hr))
    {
        throw std::runtime_error("Unable to create WIC imaging factory.");
    }

    // Create a decoder based on the file name.
    std::wstring wfilename(filename.begin(), filename.end());
    ComObject<IWICBitmapDecoder> wicDecoder;
    hr = wicFactory->CreateDecoderFromFilename(wfilename.c_str(),
        nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateDecoderFromFilename failed (" + filename + ").");
    }

    // Create a WIC decoder.
    ComObject<IWICBitmapFrameDecode> wicFrameDecode;
    hr = wicDecoder->GetFrame(0, &wicFrameDecode);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicDecoder->GetFrame failed.");
    }

    DoLoad(wicFactory.Get(), wicFrameDecode.Get(), creator);
}

void WICFileIONative::Load(void* module, std::string const& rtype,
    int resource, uint32_t& outFormat, size_t& outWidth, size_t& outHeight,
    std::vector<uint8_t>& outTexels)
{
    auto const* bytesPerTexel = msBytesPerTexel.data();

    TextureCreator creator = [&outFormat, &outWidth, &outHeight, &outTexels, &bytesPerTexel](
        uint32_t format, size_t width, size_t height)
    {
        outFormat = format;
        outWidth = width;
        outHeight = height;
        outTexels.resize(width * height * bytesPerTexel[format]);
        return reinterpret_cast<uint8_t*>(outTexels.data());
    };

    WICFileIONative::Load(module, rtype, resource, creator);
}

void WICFileIONative::Load(void* module, std::string const& rtype,
    int resource, TextureCreator const& creator)
{
    // Start COM and create WIC.
    ComInitializer comInitializer;
    if (!comInitializer.IsInitialized())
    {
        throw std::runtime_error("Unable to initialize COM for WIC.");
    }

    // Create a WIC imaging factory.
    ComObject<IWICImagingFactory> wicFactory;
    HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
        reinterpret_cast<LPVOID*>(wicFactory.GetAddressOf()));
    if (FAILED(hr))
    {
        throw std::runtime_error("Unable to create WIC imaging factory.");
    }

    // Locate the resource in the application's executable.
    auto hModule = reinterpret_cast<HMODULE>(module);
    auto const wrtype = ConvertNarrowToWide(rtype);
    HRSRC imageResHandle = FindResource(hModule, MAKEINTRESOURCE(resource), wrtype.c_str());
    if (!imageResHandle)
    {
        throw std::runtime_error("FindResource failed.");
    }

    HGLOBAL imageResDataHandle = LoadResource(hModule, imageResHandle);
    if (!imageResDataHandle)
    {
        throw std::runtime_error("LoadResource failed.");
    }

    // Lock the resource to retrieve memory pointer.
    void* imageFile = LockResource(imageResDataHandle);
    if (!imageFile)
    {
        throw std::runtime_error("LockResource failed.");
    }

    // Calculate the size.
    DWORD imageFileSize = SizeofResource(hModule, imageResHandle);
    if (!imageFileSize)
    {
        throw std::runtime_error("SizeofResource failed.");
    }

    // Create a WIC stream to map onto the memory.
    IWICStream* wicStream = nullptr;
    hr = wicFactory->CreateStream(&wicStream);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateStream failed.");
    }

    // Initialize the stream with the memory pointer and size.
    hr = wicStream->InitializeFromMemory(reinterpret_cast<BYTE*>(imageFile), imageFileSize);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->InitializeFromMemory failed.");
    }

    // Create a decoder for the stream.
    IWICBitmapDecoder* wicDecoder = nullptr;
    hr = wicFactory->CreateDecoderFromStream(
        wicStream,                     // stream for creating the decoder
        NULL,                          // do not prefer a particular vendor
        WICDecodeMetadataCacheOnLoad,  // cache metadata when needed
        &wicDecoder);                  // pointer to the decoder
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateDecoderFromStream failed.");
    }

    IWICBitmapFrameDecode* wicFrameDecode = nullptr;
    hr = wicDecoder->GetFrame(0, &wicFrameDecode);
    if (FAILED(hr))
    {
        throw std::runtime_error("pIDecoder->GetFrame failed.");
    }

    DoLoad(wicFactory.Get(), wicFrameDecode, creator);
}

void WICFileIONative::SaveToPNG(std::string const& filename, uint32_t inFormat,
    size_t inWidth, size_t inHeight, uint8_t const* inTexels)
{
    SaveTo(filename, inFormat, inWidth, inHeight, inTexels, -1.0f);
}

void WICFileIONative::SaveToJPEG(std::string const& filename, uint32_t inFormat,
    size_t inWidth, size_t inHeight, uint8_t const* inTexels,
    float imageQuality)
{
    imageQuality = std::min(std::max(imageQuality, 0.0f), 1.0f);
    SaveTo(filename, inFormat, inWidth, inHeight, inTexels, imageQuality);
}

std::wstring WICFileIONative::ConvertNarrowToWide(std::string const& input)
{
    std::wstring output;
    std::transform(input.begin(), input.end(), std::back_inserter(output),
        [](char c) { return static_cast<wchar_t>(c); });
    return output;
}

void WICFileIONative::DoLoad(void* factory, void* frameDecode, TextureCreator const& creator)
{
    auto wicFactory = reinterpret_cast<IWICImagingFactory*>(factory);
    auto wicFrameDecode = reinterpret_cast<IWICBitmapFrameDecode*>(frameDecode);

    // Get the pixel format of the image.
    WICPixelFormatGUID wicSourceGUID;
    HRESULT hr = wicFrameDecode->GetPixelFormat(&wicSourceGUID);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameDecode->GetPixelFormat failed.");
    }

    // Find the supported WIC input pixel format that matches a Texture2
    // format. If a matching format is not found, the returned texture
    // is an R8G8B8A8 format with texels converted from the source format.
    WICPixelFormatGUID wicConvertGUID = GUID_WICPixelFormat32bppRGBA;
    uint32_t format = R8G8B8A8;
    for (int i = 0; i < NUM_LOAD_FORMATS; ++i)
    {
        if (IsEqualGUID(wicSourceGUID, *msLoadFormatMap[i].wicInputGUID))
        {
            // Determine whether there is a conversion format.
            if (msLoadFormatMap[i].wicConvertGUID)
            {
                wicConvertGUID = *msLoadFormatMap[i].wicConvertGUID;
            }
            else
            {
                wicConvertGUID = *msLoadFormatMap[i].wicInputGUID;
            }
            format = msLoadFormatMap[i].format;
            break;
        }
    }

    // The wicFrameDecode value is used when no conversion is required. If
    // the decoder does not support the format in the texture, then a
    // conversion is required.
    IWICBitmapSource* wicBitmapSource = wicFrameDecode;
    ComObject<IWICFormatConverter> wicFormatConverter;
    if (!IsEqualGUID(wicSourceGUID, wicConvertGUID))
    {
        // Create a WIC format converter.
        hr = wicFactory->CreateFormatConverter(&wicFormatConverter);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicFactory->CreateFormatConverter failed.");
        }

        // Initialize format converter to convert the input texture format
        // to the nearest format supported by the decoder.
        hr = wicFormatConverter->Initialize(wicBitmapSource, wicConvertGUID,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicFormatConverter->Initialize failed.");
        }

        // Use the format converter.
        wicBitmapSource = wicFormatConverter.Get();
    }

    // Get the image dimensions.
    UINT width, height;
    hr = wicBitmapSource->GetSize(&width, &height);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicBitmapSource->GetSize failed.");
    }

    // Create the 2D image.
    uint8_t* texels = creator(format, width, height);

    // Copy the pixels from the decoder to the texture.
    UINT bytesPerTexel = msBytesPerTexel[format];
    UINT const stride = width * bytesPerTexel;
    UINT const imageSize = stride * height;
    hr = wicBitmapSource->CopyPixels(nullptr, stride, imageSize, texels);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicBitmapSource->CopyPixels failed.");
    }
}

void WICFileIONative::SaveTo(std::string const& filename, uint32_t inFormat,
    size_t inWidth, size_t inHeight, uint8_t const* inTexels, float imageQuality)
{
    if (inFormat >= NUM_FORMATS)
    {
        throw std::runtime_error("The texture format is incorrect.");
    }

    if (inWidth == 0 || inHeight == 0 || inTexels == nullptr)
    {
        throw std::runtime_error("The texture and its data must exist.");
    }

    // Start COM and create WIC.
    ComInitializer comInitializer;
    if (!comInitializer.IsInitialized())
    {
        throw std::runtime_error("Unable to initialize COM for WIC.");
    }

    // Select the WIC format that matches the input texture format.
    WICPixelFormatGUID wicSourceGUID = GUID_WICPixelFormatUndefined;
    for (int i = 0; i < NUM_SAVE_FORMATS; ++i)
    {
        if (msSaveFormatMap[i].format == inFormat)
        {
            wicSourceGUID = *msSaveFormatMap[i].wicOutputGUID;
            break;
        }
    }
    if (IsEqualGUID(wicSourceGUID, GUID_WICPixelFormatUndefined))
    {
        throw std::runtime_error("Format " + std::to_string(inFormat) +
            "is not supported for saving.");
    }

    // Create a WIC imaging factory.
    ComObject<IWICImagingFactory> wicFactory;
    HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
        reinterpret_cast<LPVOID*>(wicFactory.GetAddressOf()));
    if (FAILED(hr))
    {
        throw std::runtime_error("Unable to create WIC imaging factory.");
    }

    // Create a WIC stream for output.
    ComObject<IWICStream> wicStream;
    hr = wicFactory->CreateStream(&wicStream);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateStream failed.");
    }

    std::wstring wfilename(filename.begin(), filename.end());
    hr = wicStream->InitializeFromFilename(wfilename.c_str(), GENERIC_WRITE);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicStream->InitializeFromFilename failed (" +
            filename + ").");
    }

    // Create a WIC JPEG encoder.
    ComObject<IWICBitmapEncoder> wicEncoder;
    if (imageQuality == -1.0f)
    {
        hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &wicEncoder);
    }
    else
    {
        hr = wicFactory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &wicEncoder);
    }
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateEncoder failed.");
    }

    hr = wicEncoder->Initialize(wicStream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicEncoder->Initialize failed.");
    }

    // Create a new frame and a property bag for encoder options.
    ComObject<IWICBitmapFrameEncode> wicFrameEncode;
    ComObject<IPropertyBag2> wicPropertyBag;
    hr = wicEncoder->CreateNewFrame(&wicFrameEncode, &wicPropertyBag);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicEncoder->CreateNewFrame failed.");
    }

    if (imageQuality == -1.0f)
    {
        // Set the options for the PNG encoder.
        PROPBAG2 option = { 0 };
        VARIANT varValue;

        // Default subsampling.
        option.pstrName = const_cast<LPOLESTR>(L"InterlaceOption");
        VariantInit(&varValue);
        varValue.vt = VT_BOOL;
        varValue.boolVal = FALSE;
        hr = wicPropertyBag->Write(1, &option, &varValue);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicPropertyBag->Write failed for InterlaceOption.");
        }

        // Disable filtering.
        option.pstrName = const_cast<LPOLESTR>(L"FilterOption");
        VariantInit(&varValue);
        varValue.vt = VT_UI1;
        varValue.bVal = WICPngFilterNone;
        hr = wicPropertyBag->Write(1, &option, &varValue);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicPropertyBag->Write failed for FilterOption.");
        }
    }
    else
    {
        // Set the options for the PNG encoder.
        PROPBAG2 option = { 0 };
        VARIANT varValue;

        // Set image quality, a number in [0,1].
        option.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
        VariantInit(&varValue);
        varValue.vt = VT_R4;
        varValue.fltVal = imageQuality;
        hr = wicPropertyBag->Write(1, &option, &varValue);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicPropertyBag->Write failed for ImageQuality.");
        }
    }

    // Initialize the encoder.
    hr = wicFrameEncode->Initialize(wicPropertyBag.Get());
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameEncode->Initialize failed.");
    }

    // Set the image size.
    UINT width = static_cast<UINT>(inWidth);
    UINT height = static_cast<UINT>(inHeight);
    hr = wicFrameEncode->SetSize(width, height);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameEncode->SetSize failed.");
    }

    // Set the image format.
    WICPixelFormatGUID wicTargetGUID = wicSourceGUID;
    hr = wicFrameEncode->SetPixelFormat(&wicTargetGUID);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameEncode->SetPixelFormat failed.");
    }

    // Compute the stride and image size.
    UINT const bytesPerTexel = msBytesPerTexel[inFormat];
    UINT const stride = width * bytesPerTexel;
    UINT const imageSize = stride * height;

    // Create a WIC bitmap to wrap the texture image data.
    ComObject<IWICBitmap> wicTextureBitmap;
    hr = wicFactory->CreateBitmapFromMemory(width, height, wicSourceGUID,
        stride, imageSize, const_cast<BYTE*>(inTexels), &wicTextureBitmap);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFactory->CreateBitmapFromMemory failed.");
    }

    // The wicTextureBitmap value is used for no conversion.  If the encoder
    // does not support the format in the texture, then a conversion is
    // required.
    IWICBitmapSource* wicBitmapSource = wicTextureBitmap.Get();
    ComObject<IWICFormatConverter> wicFormatConverter;
    if (!IsEqualGUID(wicSourceGUID, wicTargetGUID))
    {
        // Create a WIC format converter.
        hr = wicFactory->CreateFormatConverter(&wicFormatConverter);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicFactory->CreateFormatConverter failed.");
        }

        // Initialize the format converter to convert to the nearest format
        // supported by the encoder.
        hr = wicFormatConverter->Initialize(wicTextureBitmap.Get(), wicTargetGUID,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            throw std::runtime_error("wicFormatConverter->Initialize failed.");
        }

        // Use the format converter.
        wicBitmapSource = wicFormatConverter.Get();
    }

    // Send the pixels to the encoder.
    hr = wicFrameEncode->WriteSource(wicBitmapSource, nullptr);
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameEncode->WriteSource failed.");
    }

    // Commit the frame.
    hr = wicFrameEncode->Commit();
    if (FAILED(hr))
    {
        throw std::runtime_error("wicFrameEncode->Commit failed.");
    }

    // Commit the encoder.
    hr = wicEncoder->Commit();
    if (FAILED(hr))
    {
        throw std::runtime_error("wicEncoder->Commit failed.");
    }
}


std::array<uint32_t, WICFileIONative::NUM_FORMATS> const
WICFileIONative::msBytesPerTexel =
{
    4, // R10G10B10A2
    4, // R10G10B10_XR_BIAS_A2
    4, // R32_FLOAT
    8, // R16G16B16A16
    2, // B5G6R5
    2, // B5G5R5A1
    0, // R1 (TODO: This needs to be unsupported or code fixed to handle it.)
    1, // R8
    2, // R16
    4, // R8G8B8A8
    4, // B8G8R8A8
};

std::array<WICFileIONative::LoadFormatMap, WICFileIONative::NUM_LOAD_FORMATS> const
WICFileIONative::msLoadFormatMap =
{ {
    { R10G10B10A2, &GUID_WICPixelFormat32bppRGBA1010102, nullptr },
    { R10G10B10_XR_BIAS_A2, &GUID_WICPixelFormat32bppRGBA1010102XR, nullptr },
    { R32_FLOAT, &GUID_WICPixelFormat32bppGrayFloat, nullptr },
    { R16G16B16A16, &GUID_WICPixelFormat64bppBGRA, &GUID_WICPixelFormat64bppRGBA },
    { B5G6R5, &GUID_WICPixelFormat16bppBGR565, nullptr },
    { B5G5R5A1, &GUID_WICPixelFormat16bppBGR555, nullptr },
    { R1, &GUID_WICPixelFormatBlackWhite, &GUID_WICPixelFormat8bppGray },
    { R8, &GUID_WICPixelFormat2bppGray, &GUID_WICPixelFormat8bppGray },
    { R8, &GUID_WICPixelFormat4bppGray, &GUID_WICPixelFormat8bppGray },
    { R8, &GUID_WICPixelFormat8bppGray, nullptr },
    { R16, &GUID_WICPixelFormat16bppGray, nullptr },
    { R8G8B8A8, &GUID_WICPixelFormat32bppRGBA, nullptr },
    { R8G8B8A8, &GUID_WICPixelFormat32bppBGRA, &GUID_WICPixelFormat32bppRGBA },
    { R16G16B16A16, &GUID_WICPixelFormat64bppRGBA, nullptr }

    // B8G8R8A8 is not supported in DX11. All unmatched formats are converted
    // to R8G8B8A8.
} };

std::array<WICFileIONative::SaveFormatMap, WICFileIONative::NUM_SAVE_FORMATS> const
WICFileIONative::msSaveFormatMap =
{ {
    { R10G10B10A2, &GUID_WICPixelFormat32bppRGBA1010102 },
    { R10G10B10_XR_BIAS_A2, &GUID_WICPixelFormat32bppRGBA1010102XR },
    { R32_FLOAT, &GUID_WICPixelFormat32bppGrayFloat },
    { B5G6R5, &GUID_WICPixelFormat16bppBGR565 },
    { B5G5R5A1, &GUID_WICPixelFormat16bppBGR555 },
    { R1, &GUID_WICPixelFormatBlackWhite },
    { R8, &GUID_WICPixelFormat8bppGray },
    { R16, &GUID_WICPixelFormat16bppGray },
    { R8G8B8A8, &GUID_WICPixelFormat32bppRGBA },
    { B8G8R8A8, &GUID_WICPixelFormat32bppBGRA },
    { R16G16B16A16, &GUID_WICPixelFormat64bppRGBA }
} };
