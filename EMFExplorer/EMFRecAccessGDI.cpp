#include "pch.h"
#include "framework.h"
#include "EMFRecAccessGDI.h"
#include "EMFAccess.h"
#include "EMFStruct2Props.h"
#include "EmfStruct.h"

#undef min
#undef max

// Forward declarations for enum/flag text helpers
static CStringW MapModeText(DWORD iMode);
static CStringW BkModeText(DWORD iMode);
static CStringW PolyFillModeText(DWORD iMode);
static CStringW ROP2Text(DWORD iMode);
static CStringW StretchBltModeText(DWORD iMode);
static CStringW TextAlignText(DWORD iMode);
static CStringW ArcDirectionText(DWORD iArcDirection);
static CStringW RegionModeText(DWORD iMode);
static CStringW ModifyWorldTransformModeText(DWORD iMode);
static CStringW FloodFillModeText(DWORD iMode);
static CStringW GradientFillModeText(DWORD ulMode);
static CStringW BrushStyleText(DWORD lbStyle);
static CStringW HatchStyleText(DWORD lbHatch);
static CStringW PenStyleText(DWORD lopnStyle);
static CStringW ExtPenStyleText(DWORD elpPenStyle);
static CStringW FontWeightText(LONG lfWeight);
static CStringW CharSetText(BYTE lfCharSet);
static CStringW FontQualityText(BYTE lfQuality);
static CStringW GraphicsModeText(UINT iGraphicsMode);
static CStringW RasterOpText(DWORD dwRop);
static CStringW BlendFunctionText(DWORD dwRop);
static CStringW DIBColorsText(DWORD iUsage);
static CStringW BiCompressionText(DWORD biCompression);
static CStringW ExtTextOutOptionsText(UINT fuOptions);

const ENHMETARECORD* EMFRecAccessGDIRec::GetGDIRecord(const emfplus::OEmfPlusRecInfo& rec)
{
	if (rec.Data)
	{
		// I can't find any document for this, but it seems that this is the way to go (based on observation)
		auto pRec = (const ENHMETARECORD*)(rec.Data - sizeof(EMR));
		return pRec;
	}
	return nullptr;
}

void EMFRecAccessGDIRecHeader::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIControlCat::CacheProperties(ctxt);
	auto pRec = (const ENHMETAHEADER*)GetGDIRecord(m_recInfo);
	if (!pRec)
	{
		ASSERT(0);
		return;
	}
	ASSERT(pRec->dSignature == ENHMETA_SIGNATURE);
	m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
	m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclFrame", pRec->rclFrame));
	m_propsCached->AddValue(L"Signature", pRec->dSignature, true);
	m_propsCached->AddValue(L"nVersion", pRec->nVersion);
	m_propsCached->AddValue(L"nBytes", pRec->nBytes);
	m_propsCached->AddValue(L"nRecords", pRec->nRecords);
	m_propsCached->AddValue(L"nHandles", pRec->nHandles);
	if (pRec->nDescription && pRec->offDescription)
	{
		m_propsCached->AddText(L"Description", (LPCWSTR)((const char*)pRec + pRec->offDescription));
	}
	m_propsCached->AddValue(L"nPalEntries", pRec->nPalEntries);
	m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeSizeInt>(L"szlDevice", pRec->szlDevice));
	m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeSizeInt>(L"szlMillimeters", pRec->szlMillimeters));
	// The ENHMETAHEADER can be shorter in older EMF versions.
	// Fields cbPixelFormat/offPixelFormat/bOpenGL need data size >= 92;
	// szlMicrometers needs data size >= 100.
	if (m_recInfo.DataSize >= 92)
	{
		if (pRec->cbPixelFormat && pRec->offPixelFormat)
		{
			auto pixelFormat = (const PIXELFORMATDESCRIPTOR*)((const char*)pRec + pRec->offPixelFormat);
			auto pPixelFormatNode = m_propsCached->AddBranch(L"PixelFormat");
			EmfStruct2Properties::Build(*pixelFormat, pPixelFormatNode.get());
		}
		m_propsCached->AddValue(L"bOpenGL", pRec->bOpenGL);
	}
	if (m_recInfo.DataSize >= 100)
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeSizeInt>(L"szlMicrometers", pRec->szlMicrometers));

	auto pPlusNode = m_propsCached->AddBranch(L"GDI+ Header");
	auto& hdr = ctxt.pEMF->GetMetafileHeader();
	GetPropertiesFromGDIPlusHeader(pPlusNode.get(), hdr);
}

void EMFRecAccessGDIRecExcludeClipRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIClippingCat::CacheProperties(ctxt);
	auto pRec = (EMREXCLUDECLIPRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecRestoreDC::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRRESTOREDC*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pSaveRec = pEMF->GetGDISaveRecord(pRec->iRelative);
	if (pSaveRec)
	{
		AddLinkRecord(pSaveRec, LinkedObjTypeGraphicState, LinkedObjTypeGraphicState);
	}
}

void EMFRecAccessGDIRecRestoreDC::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRRESTOREDC*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"iRelative", pRec->iRelative);
	}
}

void EMFRecAccessGDIRecSelectObject::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRSELECTOBJECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec && !(pRec->ihObject & ENHMETA_STOCK_OBJECT))
	{
		auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihObject, false);
		if (pLinkedRec)
		{
			ASSERT(pLinkedRec->GetRecordCategory() == RecCategoryObject);
			AddLinkRecord(pLinkedRec, LinkedObjTypeObjectUnspecified, LinkedObjTypeObjManipulation);
			// TODO, link drawing records?
		}
	}
}

void EMFRecAccessGDIRecSelectObject::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRSELECTOBJECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (!pRec)
		return;
	if (pRec->ihObject & ENHMETA_STOCK_OBJECT)
	{
		auto hObj = pRec->ihObject & ~ENHMETA_STOCK_OBJECT;

		CStringW str;
		str.Format(L"%s (%08X)", EMFGDIStockObjText(hObj), pRec->ihObject);
		m_propsCached->AddText(L"ihObject", str);
	}
	else
	{
		m_propsCached->AddValue(L"ihObject", pRec->ihObject);
	}
}

void EMFRecAccessGDIRecCreatePen::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihPen, this, false);
}

bool EMFRecAccessGDIRecCreatePen::GetRecordColor(COLORREF& cr) const
{
	auto pRec = (const EMRCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		cr = pRec->lopn.lopnColor;
		return true;
	}
	return false;
}

void EMFRecAccessGDIRecCreatePen::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"ihPen", pRec->ihPen);
		auto pBranch = m_propsCached->AddBranch(L"lopn");
		pBranch->AddText(L"lopnStyle", PenStyleText(pRec->lopn.lopnStyle));
		EmfStruct2Properties::BuildField("lopnWidth", pRec->lopn.lopnWidth, pBranch.get());
		pBranch->sub.emplace_back(std::make_shared<PropertyNodeColor>(L"lopnColor",
			emfplus::OEmfPlusARGB::FromCOLORREF(pRec->lopn.lopnColor)));
	}
}

void EMFRecAccessGDIRecCreateBrushIndirect::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATEBRUSHINDIRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihBrush, this, false);
}

bool EMFRecAccessGDIRecCreateBrushIndirect::GetRecordColor(COLORREF& cr) const
{
	auto pRec = (const EMRCREATEBRUSHINDIRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		cr = pRec->lb.lbColor;
		return true;
	}
	return false;
}

void EMFRecAccessGDIRecCreateBrushIndirect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATEBRUSHINDIRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"ihBrush", pRec->ihBrush);
		auto pBranch = m_propsCached->AddBranch(L"lb");
		pBranch->AddText(L"lbStyle", BrushStyleText(pRec->lb.lbStyle));
		pBranch->sub.emplace_back(std::make_shared<PropertyNodeColor>(L"lbColor",
			emfplus::OEmfPlusARGB::FromCOLORREF(pRec->lb.lbColor)));
		if (pRec->lb.lbStyle == BS_HATCHED)
			pBranch->AddText(L"lbHatch", HatchStyleText((DWORD)pRec->lb.lbHatch));
		else
			pBranch->AddValue(L"lbHatch", (DWORD)pRec->lb.lbHatch);
	}
}

void EMFRecAccessGDIRecDeleteObject::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRDELETEOBJECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihObject, false);
	if (pLinkedRec)
	{
		ASSERT(pLinkedRec->GetRecordCategory() == RecCategoryObject);
		AddLinkRecord(pLinkedRec, LinkedObjTypeObjectUnspecified, LinkedObjTypeObjManipulation);
		// TODO, link drawing records?
	}
}

void EMFRecAccessGDIRecDeleteObject::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRDELETEOBJECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"ihObject", pRec->ihObject);
	}
}

void EMFRecAccessGDIRecSelectPalette::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRSELECTPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihPal, false);
	if (pLinkedRec)
	{
		ASSERT(pLinkedRec->GetRecordCategory() == RecCategoryObject);
		AddLinkRecord(pLinkedRec, LinkedObjTypePalette, LinkedObjTypeObjManipulation);
		// TODO, link drawing records?
	}
}

void EMFRecAccessGDIRecSelectPalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRSELECTPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecCreatePalette::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATEPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihPal, this, false);
}

void EMFRecAccessGDIRecCreatePalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATEPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetPaletteEntries::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRSETPALETTEENTRIES*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihPal, false);
	if (pLinkedRec)
	{
		AddLinkRecord(pLinkedRec, LinkedObjTypePalette, LinkedObjTypeObjManipulation);
		// TODO, link all drawing records too?
	}
}

void EMFRecAccessGDIRecSetPaletteEntries::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRSETPALETTEENTRIES*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfSetPaletteEntries(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecResizePalette::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRRESIZEPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihPal, false);
	if (pLinkedRec)
	{
		ASSERT(pLinkedRec->GetRecordCategory() == RecCategoryObject);
		AddLinkRecord(pLinkedRec, LinkedObjTypePalette, LinkedObjTypeObjManipulation);
		// TODO, link drawing records?
	}
}

void EMFRecAccessGDIRecResizePalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRRESIZEPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecGdiComment::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIRec::CacheProperties(ctxt);
	auto pRec = (const EMRCOMMENT_BASE*)GetGDIRecord(m_recInfo);
	CStringW str;
	str.Format(L"%08X", pRec->CommentIdentifier);
	m_propsCached->AddText(L"Identifier", str);
	str = L"Unknown";
	CString strText;
	switch (pRec->CommentIdentifier)
	{
	case EMR_COMMENT_EMFPLUS:
		str = L"EMF+";
		break;
	case EMR_COMMENT_EMFSPOOL:
		str = L"EMF Spool";
		break;
	case EMR_COMMENT_PUBLIC:
		{
			auto pPublicRec = (EMRCOMMENT_PUBLIC*)pRec;
			switch (pPublicRec->PublicCommentIdentifier)
			{
			case EMR_COMMENT_BEGINGROUP:
				str = L"Public: Begin group";
				break;
			case EMR_COMMENT_ENDGROUP:
				str = L"Public: End group";
				break;
			case EMR_COMMENT_WINDOWS_METAFILE:
				str = L"Public: End Windows Metafile";
				break;
			case EMR_COMMENT_MULTIFORMATS:
				str = L"Public: Multiformats";
				break;
			case EMR_COMMENT_UNICODE_STRING:
				str = L"Public: Unicode String";
				break;
			case EMR_COMMENT_UNICODE_END:
				str = L"Public: Unicode End";
				break;
			}
		}
		break;
	default:
		if (pRec->DataSize)
		{
			char szBuffer[100]{ 0 };
			strncpy_s(szBuffer, _countof(szBuffer), (char*)&pRec->CommentIdentifier, _countof(szBuffer));
			strText = szBuffer;
		}
		break;
	}
	m_propsCached->AddText(L"CommentType", str);
	if (!strText.IsEmpty())
		m_propsCached->AddText(L"Text", strText);
}

void EMFRecAccessGDIRecBitBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRBITBLT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"dwRop")
				node->text = RasterOpText(pRec->dwRop);
			else if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
		}
		if (pRec->offBmiSrc)
		{
			auto pSrcBmpHeader = (const BITMAPINFOHEADER*)((char*)pRec + pRec->offBmiSrc);
			auto pSrcBmpHeaderNode = m_propsCached->AddBranch(L"BMP");
			EmfStruct2Properties::Build(*pSrcBmpHeader, pSrcBmpHeaderNode.get());
			for (auto& node : pSrcBmpHeaderNode->sub)
			{
				if (node->name == L"biCompression")
					node->text = BiCompressionText(pSrcBmpHeader->biCompression);
			}
		}
	}
}

void EMFRecAccessGDIRecStretchBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRSTRETCHBLT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"dwRop")
				node->text = RasterOpText(pRec->dwRop);
		}
		if (pRec->offBmiSrc)
		{
			auto pSrcBmpHeader = (const BITMAPINFOHEADER*)((char*)pRec + pRec->offBmiSrc);
			auto pSrcBmpHeaderNode = m_propsCached->AddBranch(L"BMP");
			EmfStruct2Properties::Build(*pSrcBmpHeader, pSrcBmpHeaderNode.get());
			for (auto& node : pSrcBmpHeaderNode->sub)
			{
				if (node->name == L"biCompression")
					node->text = BiCompressionText(pSrcBmpHeader->biCompression);
			}
		}
	}
}

void EMFRecAccessGDIRecMaskBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRMASKBLT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"dwRop")
				node->text = RasterOpText(pRec->dwRop);
			else if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
			else if (node->name == L"iUsageMask")
				node->text = DIBColorsText(pRec->iUsageMask);
		}
	}
}

void EMFRecAccessGDIRecPlgBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRPLGBLT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
			else if (node->name == L"iUsageMask")
				node->text = DIBColorsText(pRec->iUsageMask);
		}
	}
}

void EMFRecAccessGDIRecSetDIBitsToDevice::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRSETDIBITSTODEVICE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
		}
		if (pRec->offBmiSrc)
		{
			auto pSrcBmpHeader = (const BITMAPINFOHEADER*)((char*)pRec + pRec->offBmiSrc);
			auto pSrcBmpHeaderNode = m_propsCached->AddBranch(L"BMP");
			EmfStruct2Properties::Build(*pSrcBmpHeader, pSrcBmpHeaderNode.get());
			for (auto& node : pSrcBmpHeaderNode->sub)
			{
				if (node->name == L"biCompression")
					node->text = BiCompressionText(pSrcBmpHeader->biCompression);
			}
		}
	}
}

bool EMFRecAccessGDIRecStretchDIBits::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRSTRETCHDIBITS*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (!pRec->offBmiSrc)
		return false;
	if (info)
	{
		auto pSrcBmpHeader = (const BITMAPINFOHEADER*)((char*)pRec + pRec->offBmiSrc);
		CSize szImg(pSrcBmpHeader->biWidth, pSrcBmpHeader->biHeight);
		CRect rect = info->rect;
		if (info->bCalcOnly)
		{
			CSize sz = info->GetDefaultImgPreviewSize();
			rect.right = rect.left + sz.cx;
			rect.bottom = rect.top + sz.cy;
		}
		float scale = 1.0f;
		CRect rcFit = GetFitRect(rect, szImg, true, &scale);
		info->szPreferedSize = rcFit.Size();
		if (!info->bCalcOnly)
		{
			const char* pSrcBits = (const char*)((char*)pRec + pRec->offBitsSrc);
			StretchDIBits(info->pDC->GetSafeHdc(), rcFit.left, rcFit.top, rcFit.Width(), rcFit.Height(),
				0, 0, szImg.cx, szImg.cy, pSrcBits, (BITMAPINFO*)pSrcBmpHeader, pRec->iUsageSrc, SRCCOPY);
		}
	}
	return true;
}

void EMFRecAccessGDIRecStretchDIBits::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRSTRETCHDIBITS*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"dwRop")
				node->text = RasterOpText(pRec->dwRop);
			else if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
		}
		if (pRec->offBmiSrc)
		{
			auto pSrcBmpHeader = (const BITMAPINFOHEADER*)((char*)pRec + pRec->offBmiSrc);
			auto pSrcBmpHeaderNode = m_propsCached->AddBranch(L"BMP");
			EmfStruct2Properties::Build(*pSrcBmpHeader, pSrcBmpHeaderNode.get());
			for (auto& node : pSrcBmpHeaderNode->sub)
			{
				if (node->name == L"biCompression")
					node->text = BiCompressionText(pSrcBmpHeader->biCompression);
			}
		}
	}
}

void EMFRecAccessGDIRecExtCreateFontIndirect::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMREXTCREATEFONTINDIRECTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihFont, this, false);
}

void EMFRecAccessGDIRecExtCreateFontIndirect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMREXTCREATEFONTINDIRECTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"ihFont", pRec->ihFont);
		// The EMF spec allows this record to contain either a bare LOGFONTW (92 bytes)
		// or a full EXTLOGFONTW. Check the record size to avoid reading beyond the data.
		// Note: m_recInfo.DataSize is the data size after the EMR header;
		// subtract sizeof(DWORD) for the ihFont field to get the font struct size.
		bool hasExtLogFont = (m_recInfo.DataSize >= sizeof(DWORD) + sizeof(EXTLOGFONTW));
		auto pElfw = m_propsCached->AddBranch(hasExtLogFont ? L"elfw" : L"LogFont");
		auto& lf = pRec->elfw.elfLogFont;
		auto pLf = hasExtLogFont ? pElfw->AddBranch(L"elfLogFont") : pElfw;
		pLf->AddValue(L"lfHeight", lf.lfHeight);
		pLf->AddValue(L"lfWidth", lf.lfWidth);
		pLf->AddValue(L"lfEscapement", lf.lfEscapement);
		pLf->AddValue(L"lfOrientation", lf.lfOrientation);
		pLf->AddText(L"lfWeight", FontWeightText(lf.lfWeight));
		pLf->AddValue(L"lfItalic", lf.lfItalic);
		pLf->AddValue(L"lfUnderline", lf.lfUnderline);
		pLf->AddValue(L"lfStrikeOut", lf.lfStrikeOut);
		pLf->AddText(L"lfCharSet", CharSetText(lf.lfCharSet));
		pLf->AddValue(L"lfOutPrecision", lf.lfOutPrecision);
		pLf->AddValue(L"lfClipPrecision", lf.lfClipPrecision);
		pLf->AddText(L"lfQuality", FontQualityText(lf.lfQuality));
		pLf->AddValue(L"lfPitchAndFamily", lf.lfPitchAndFamily);
		pLf->AddText(L"lfFaceName", lf.lfFaceName);
		if (hasExtLogFont)
		{
			pElfw->AddText(L"elfFullName", pRec->elfw.elfFullName);
			pElfw->AddText(L"elfStyle", pRec->elfw.elfStyle);
			pElfw->AddValue(L"elfVersion", pRec->elfw.elfVersion);
			pElfw->AddValue(L"elfStyleSize", pRec->elfw.elfStyleSize);
			pElfw->AddValue(L"elfMatch", pRec->elfw.elfMatch);
			pElfw->AddValue(L"elfCulture", pRec->elfw.elfCulture);
			EmfStruct2Properties::BuildField("elfPanose", pRec->elfw.elfPanose, pElfw.get());
		}
	}
}

LPCWSTR EMFRecAccessGDIRecExtTextOutA::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto pRec = (const EMREXTTEXTOUTA*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
		if (pRec && !(pRec->emrtext.fOptions & ETO_GLYPH_INDEX) && pRec->emrtext.nChars > 0 && pRec->emrtext.offString)
		{
			LPCSTR pszText = (LPCSTR)((const BYTE*)pRec + pRec->emrtext.offString);
			m_strText = CStringW(pszText, pRec->emrtext.nChars);
		}
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessGDIRecExtTextOutA::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMREXTTEXTOUTA*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
		m_propsCached->AddText(L"iGraphicsMode", GraphicsModeText(pRec->iGraphicsMode));
		m_propsCached->AddValue(L"exScale", pRec->exScale);
		m_propsCached->AddValue(L"eyScale", pRec->eyScale);
		EmfStruct2Properties::BuildField("emrtext", pRec->emrtext, m_propsCached.get());
		auto& pEmrtext = m_propsCached->sub.back();
		for (auto& child : pEmrtext->sub)
		{
			if (child->name == L"fOptions")
			{
				child->text = ExtTextOutOptionsText(pRec->emrtext.fOptions);
				break;
			}
		}
		if (pRec->emrtext.fOptions & ETO_GLYPH_INDEX)
		{
			if (pRec->emrtext.nChars > 0 && pRec->emrtext.offString)
			{
				auto pGlyphs = (const emfplus::u16t*)((const BYTE*)pRec + pRec->emrtext.offString);
				m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeArray>(L"Glyphs", pGlyphs, (size_t)pRec->emrtext.nChars));
			}
		}
		else
		{
			auto pszText = GetRecordText();
			if (pszText)
				m_propsCached->AddText(L"Text", pszText);
		}
	}
}

LPCWSTR EMFRecAccessGDIRecExtTextOutW::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto pRec = (const EMREXTTEXTOUTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
		if (pRec && !(pRec->emrtext.fOptions & ETO_GLYPH_INDEX) && pRec->emrtext.nChars > 0 && pRec->emrtext.offString)
		{
			LPCWSTR pszText = (LPCWSTR)((const BYTE*)pRec + pRec->emrtext.offString);
			m_strText.SetString(pszText, pRec->emrtext.nChars);
		}
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessGDIRecExtTextOutW::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMREXTTEXTOUTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
		m_propsCached->AddText(L"iGraphicsMode", GraphicsModeText(pRec->iGraphicsMode));
		m_propsCached->AddValue(L"exScale", pRec->exScale);
		m_propsCached->AddValue(L"eyScale", pRec->eyScale);
		EmfStruct2Properties::BuildField("emrtext", pRec->emrtext, m_propsCached.get());
		auto& pEmrtext = m_propsCached->sub.back();
		for (auto& child : pEmrtext->sub)
		{
			if (child->name == L"fOptions")
			{
				child->text = ExtTextOutOptionsText(pRec->emrtext.fOptions);
				break;
			}
		}
		if (pRec->emrtext.fOptions & ETO_GLYPH_INDEX)
		{
			if (pRec->emrtext.nChars > 0 && pRec->emrtext.offString)
			{
				auto pGlyphs = (const emfplus::u16t*)((const BYTE*)pRec + pRec->emrtext.offString);
				m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeArray>(L"Glyphs", pGlyphs, (size_t)pRec->emrtext.nChars));
			}
		}
		else
		{
			auto pszText = GetRecordText();
			if (pszText)
				m_propsCached->AddText(L"Text", pszText);
		}
	}
}

void EMFRecAccessGDIRecCreateMonoBrush::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATEMONOBRUSH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihBrush, this, false);
}

void EMFRecAccessGDIRecCreateMonoBrush::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATEMONOBRUSH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"iUsage")
				node->text = DIBColorsText(pRec->iUsage);
		}
	}
}

void EMFRecAccessGDIRecCreateDIBPatternBrushPt::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATEDIBPATTERNBRUSHPT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihBrush, this, false);
}

void EMFRecAccessGDIRecCreateDIBPatternBrushPt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATEDIBPATTERNBRUSHPT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"iUsage")
				node->text = DIBColorsText(pRec->iUsage);
		}
	}
}

void EMFRecAccessGDIRecExtCreatePen::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMREXTCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihPen, this, false);
}

bool EMFRecAccessGDIRecExtCreatePen::GetRecordColor(COLORREF& cr) const
{
	auto pRec = (const EMREXTCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		cr = pRec->elp.elpColor;
		return true;
	}
	return false;
}

void EMFRecAccessGDIRecExtCreatePen::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMREXTCREATEPEN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"ihPen", pRec->ihPen);
		m_propsCached->AddValue(L"offBmi", pRec->offBmi);
		m_propsCached->AddValue(L"cbBmi", pRec->cbBmi);
		m_propsCached->AddValue(L"offBits", pRec->offBits);
		m_propsCached->AddValue(L"cbBits", pRec->cbBits);
		auto pBranch = m_propsCached->AddBranch(L"elp");
		pBranch->AddText(L"elpPenStyle", ExtPenStyleText(pRec->elp.elpPenStyle));
		pBranch->AddValue(L"elpWidth", pRec->elp.elpWidth);
		pBranch->AddText(L"elpBrushStyle", BrushStyleText(pRec->elp.elpBrushStyle));
		pBranch->sub.emplace_back(std::make_shared<PropertyNodeColor>(L"elpColor",
			emfplus::OEmfPlusARGB::FromCOLORREF(pRec->elp.elpColor)));
		if (pRec->elp.elpBrushStyle == BS_HATCHED)
			pBranch->AddText(L"elpHatch", HatchStyleText((DWORD)pRec->elp.elpHatch));
		else
			pBranch->AddValue(L"elpHatch", (DWORD)pRec->elp.elpHatch);
		pBranch->AddValue(L"elpNumEntries", pRec->elp.elpNumEntries);
	}
}

void EMFRecAccessGDIRecCreateColorSpace::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATECOLORSPACE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihCS, this, false);
}

void EMFRecAccessGDIRecCreateColorSpace::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATECOLORSPACE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetColorSpace::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRSETCOLORSPACE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihCS, false);
	if (pLinkedRec)
	{
		AddLinkRecord(pLinkedRec, LinkedObjTypeColorspace, LinkedObjTypeObjManipulation);
		// TODO, link all drawing records too?
	}
}

void EMFRecAccessGDIRecSetColorSpace::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRSETCOLORSPACE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecDeleteColorSpace::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRDELETECOLORSPACE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihCS, false);
	if (pLinkedRec)
	{
		AddLinkRecord(pLinkedRec, LinkedObjTypeColorspace, LinkedObjTypeObjManipulation);
		// TODO, link all drawing records too?
	}
}

void EMFRecAccessGDIRecDeleteColorSpace::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRDELETECOLORSPACE*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecColorCorrectPalette::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCOLORCORRECTPALETTE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ihPalette, false);
	if (pLinkedRec)
		AddLinkRecord(pLinkedRec, LinkedObjTypePalette, LinkedObjTypeObjManipulation);
}

void EMFRecAccessGDIRecColorCorrectPalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjManipulationCat::CacheProperties(ctxt);
	auto pRec = (EMRCOLORCORRECTPALETTE*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecAlphaBlend::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRALPHABLEND*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (auto& node : m_propsCached->sub)
		{
			if (node->name == L"dwRop")
				node->text = BlendFunctionText(pRec->dwRop);
			else if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
		}
	}
}

void EMFRecAccessGDIRecTransparentBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIBitmapCat::CacheProperties(ctxt);
	auto pRec = (EMRTRANSPARENTBLT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
		for (size_t i = 0; i < m_propsCached->sub.size(); ++i)
		{
			auto& node = m_propsCached->sub[i];
			if (node->name == L"dwRop")
			{
				// dwRop is actually the transparent color (COLORREF)
				m_propsCached->sub[i] = std::make_shared<PropertyNodeColor>(L"dwRop",
					emfplus::OEmfPlusARGB::FromCOLORREF(pRec->dwRop));
			}
			else if (node->name == L"iUsageSrc")
				node->text = DIBColorsText(pRec->iUsageSrc);
		}
	}
}

void EMFRecAccessGDIRecCreateColorSpaceW::Preprocess(EMFAccess* pEMF)
{
	auto pRec = (EMRCREATECOLORSPACEW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	pEMF->SetObjectToTable(pRec->ihCS, this, false);
}

void EMFRecAccessGDIRecCreateColorSpaceW::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIObjectCat::CacheProperties(ctxt);
	auto pRec = (EMRCREATECOLORSPACEW*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyBezier::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYBEZIER*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline(*pRec), m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecPolyBezier::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	return m_previewHelper.DrawPreview(info, pRec, GetRecordType());
}

void EMFRecAccessGDIRecPolygon::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline(*pRec), m_propsCached.get());
	}
}

bool EMFGDIRecPolygonPreviewHelper::DrawPreview(EMFRecAccess::PreviewContext* info, const EMRPOLYGON* pRec, emfplus::OEmfPlusRecordType nType)
{
	if (!pRec || pRec->cptl == 0)
		return false;
	if (!info)
		return true;
	CRect rect = info->rect;
	if (info->bCalcOnly)
	{
		CSize sz = info->GetDefaultImgPreviewSize();
		rect.right = rect.left + sz.cx;
		rect.bottom = rect.top + sz.cy;
	}
	if (m_rcBounds.IsRectNull())
	{
		auto& rclBounds = pRec->rclBounds;
		if (rclBounds.left == 0 && rclBounds.top == 0
			&& rclBounds.top == -1 && rclBounds.bottom == -1)
		{
			auto ptMin = pRec->aptl[0];
			auto ptMax = ptMin;
			for (DWORD ii = 1; ii < pRec->cptl; ++ii)
			{
				auto& pt = pRec->aptl[ii];
				ptMin.x = std::min(ptMin.x, pt.x);
				ptMin.y = std::min(ptMin.y, pt.y);
				ptMax.x = std::max(ptMax.x, pt.x);
				ptMax.y = std::max(ptMax.y, pt.y);
			}
			m_rcBounds.SetRect(ptMin.x, ptMin.y, ptMax.x, ptMax.y);
		}
		else
		{
			m_rcBounds = (RECT&)rclBounds;
		}
	}
	CSize szBound = m_rcBounds.Size();
	szBound.cx = std::abs(szBound.cx);
	szBound.cy = std::abs(szBound.cy);
	CRect rcFit = GetFitRect(rect, szBound, true);
	info->szPreferedSize = rcFit.Size();
	if (!info->bCalcOnly)
	{
		auto pDC = info->pDC;
		int nOldMapMode = pDC->SetMapMode(MM_ISOTROPIC);
		auto oldWndExt = pDC->SetWindowExt(szBound.cx, szBound.cy);
		auto oldVwpExt = pDC->SetViewportExt(info->szPreferedSize.cx, info->szPreferedSize.cy);
		auto oldVwpOrg = pDC->SetViewportOrg(rcFit.TopLeft());
		auto oldWndOrg = pDC->SetWindowOrg(m_rcBounds.TopLeft());

		CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
		auto oldPen = pDC->SelectObject(&pen);

		switch (nType)
		{
		case EmfRecordTypePolygon:
		case EmfRecordTypePolyline:
		case EmfRecordTypePolyLineTo:
			pDC->Polygon((POINT*)pRec->aptl, (int)pRec->cptl);
			break;
		case EmfRecordTypePolyBezier:
		case EmfRecordTypePolyBezierTo:
			pDC->PolyBezier((POINT*)pRec->aptl, (int)pRec->cptl);
			break;
		}

		pDC->SelectObject(oldPen);

		pDC->SetViewportOrg(oldVwpOrg);
		pDC->SetWindowOrg(oldWndOrg);
		pDC->SetViewportExt(oldVwpExt);
		pDC->SetWindowExt(oldWndExt);
		pDC->SetMapMode(nOldMapMode);
	}
	return true;
}

bool EMFRecAccessGDIRecPolygon::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	return m_previewHelper.DrawPreview(info, pRec, GetRecordType());
}

void EMFRecAccessGDIRecPolyline::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYLINE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline(*pRec), m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecPolyline::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	return m_previewHelper.DrawPreview(info, pRec, GetRecordType());
}

void EMFRecAccessGDIRecPolyBezierTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYBEZIERTO*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline(*pRec), m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecPolyBezierTo::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	return m_previewHelper.DrawPreview(info, pRec, GetRecordType());
}

void EMFRecAccessGDIRecPolyLineTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYLINETO*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline(*pRec), m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecPolyLineTo::DrawPreview(PreviewContext* info)
{
	auto pRec = (EMRPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	return m_previewHelper.DrawPreview(info, pRec, GetRecordType());
}

void EMFRecAccessGDIRecPolyPolyline::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYPOLYLINE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyPolyline(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyPolygon::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYPOLYGON*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyPolyline(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetWindowExtEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETWINDOWEXTEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetWindowOrgEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETWINDOWORGEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetViewportExtEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETVIEWPORTEXTEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetViewportOrgEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETVIEWPORTORGEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetBrushOrgEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETBRUSHORGEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetPixelV::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRSETPIXELV*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetMapperFlags::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETMAPPERFLAGS*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetMapMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETMAPMODE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", MapModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetBkMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETBKMODE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", BkModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetPolyFillMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETPOLYFILLMODE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", PolyFillModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetROP2::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETROP2*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", ROP2Text(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetStretchBltMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETSTRETCHBLTMODE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", StretchBltModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetTextAlign::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETTEXTALIGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", TextAlignText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecSetColorAdjustment::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETCOLORADJUSTMENT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecSetTextColor::GetRecordColor(COLORREF& cr) const
{
	if (m_recInfo.Data && m_recInfo.DataSize >= sizeof(COLORREF))
	{
		cr = *(const COLORREF*)m_recInfo.Data;
		return true;
	}
	return false;
}

void EMFRecAccessGDIRecSetTextColor::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETTEXTCOLOR*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

bool EMFRecAccessGDIRecSetBkColor::GetRecordColor(COLORREF& cr) const
{
	if (m_recInfo.Data && m_recInfo.DataSize >= sizeof(COLORREF))
	{
		cr = *(const COLORREF*)m_recInfo.Data;
		return true;
	}
	return false;
}

void EMFRecAccessGDIRecSetBkColor::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETBKCOLOR*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecOffsetClipRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIClippingCat::CacheProperties(ctxt);
	auto pRec = (EMROFFSETCLIPRGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecMoveToEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRMOVETOEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecIntersectClipRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIClippingCat::CacheProperties(ctxt);
	auto pRec = (EMRINTERSECTCLIPRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecScaleViewportExtEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSCALEVIEWPORTEXTEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecScaleWindowExtEx::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSCALEWINDOWEXTEX*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetWorldTransform::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDITransformCat::CacheProperties(ctxt);
	auto pRec = (EMRSETWORLDTRANSFORM*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecModifyWorldTransform::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDITransformCat::CacheProperties(ctxt);
	auto pRec = (EMRMODIFYWORLDTRANSFORM*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::BuildField("xform", pRec->xform, m_propsCached.get());
		m_propsCached->AddText(L"iMode", ModifyWorldTransformModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecAngleArc::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRANGLEARC*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecEllipse::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRELLIPSE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecRectangle::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRRECTANGLE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecRoundRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRROUNDRECT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecArc::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRARC*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecChord::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRCHORD*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPie::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPIE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecExtFloodFill::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMREXTFLOODFILL*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::BuildField("ptlStart", pRec->ptlStart, m_propsCached.get());
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeColor>(L"crColor",
			emfplus::OEmfPlusARGB::FromCOLORREF(pRec->crColor)));
		m_propsCached->AddText(L"iMode", FloodFillModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecLineTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRLINETO*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecArcTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRARCTO*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyDraw::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYDRAW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyDraw(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetArcDirection::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETARCDIRECTION*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iArcDirection", ArcDirectionText(pRec->iArcDirection));
	}
}

void EMFRecAccessGDIRecSetMiterLimit::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETMITERLIMIT*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecFillPath::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRFILLPATH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecStrokeAndFillPath::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRSTROKEANDFILLPATH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecStrokePath::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRSTROKEPATH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSelectClipPath::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIClippingCat::CacheProperties(ctxt);
	auto pRec = (EMRSELECTCLIPPATH*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddText(L"iMode", RegionModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecFillRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRFILLRGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecFrameRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRFRAMERGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecInvertRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRINVERTRGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPaintRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPAINTRGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecExtSelectClipRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIClippingCat::CacheProperties(ctxt);
	auto pRec = (EMREXTSELECTCLIPRGN*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->AddValue(L"cbRgnData", pRec->cbRgnData);
		m_propsCached->AddText(L"iMode", RegionModeText(pRec->iMode));
	}
}

void EMFRecAccessGDIRecPolyBezier16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYBEZIER16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolygon16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYGON16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyline16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYLINE16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyBezierTo16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYBEZIERTO16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolylineTo16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYLINETO16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyPolyline16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYPOLYLINE16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyPolygon16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYPOLYGON16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyPolyline16(*pRec), m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPolyDraw16::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYDRAW16*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(emfgdi::OEmfPolyDraw16(*pRec), m_propsCached.get());
	}
}

LPCWSTR EMFRecAccessGDIRecPolyTextOutA::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto pRec = (const EMRPOLYTEXTOUTA*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
		if (pRec)
		{
			for (LONG ii = 0; ii < pRec->cStrings; ++ii)
			{
				auto& emt = pRec->aemrtext[ii];
				if (!(emt.fOptions & ETO_GLYPH_INDEX) && emt.nChars > 0 && emt.offString)
				{
					if (!m_strText.IsEmpty())
						m_strText += L' ';
					LPCSTR pszText = (LPCSTR)((const BYTE*)pRec + emt.offString);
					m_strText += CStringW(pszText, emt.nChars);
				}
			}
		}
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessGDIRecPolyTextOutA::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYTEXTOUTA*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
		m_propsCached->AddText(L"iGraphicsMode", GraphicsModeText(pRec->iGraphicsMode));
		m_propsCached->AddValue(L"exScale", pRec->exScale);
		m_propsCached->AddValue(L"eyScale", pRec->eyScale);
		m_propsCached->AddValue(L"cStrings", pRec->cStrings);
		for (LONG ii = 0; ii < pRec->cStrings; ++ii)
		{
			auto& emt = pRec->aemrtext[ii];
			CStringW branchName;
			branchName.Format(L"aemrtext[%d]", ii);
			EmfStruct2Properties::BuildField(CStringA(branchName), emt, m_propsCached.get());
			auto& pBranch = m_propsCached->sub.back();
			for (auto& child : pBranch->sub)
			{
				if (child->name == L"fOptions")
				{
					child->text = ExtTextOutOptionsText(emt.fOptions);
					break;
				}
			}
		}
		auto pszText = GetRecordText();
		if (pszText)
			m_propsCached->AddText(L"Text", pszText);
	}
}

LPCWSTR EMFRecAccessGDIRecPolyTextOutW::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto pRec = (const EMRPOLYTEXTOUTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
		if (pRec)
		{
			for (LONG ii = 0; ii < pRec->cStrings; ++ii)
			{
				auto& emt = pRec->aemrtext[ii];
				if (!(emt.fOptions & ETO_GLYPH_INDEX) && emt.nChars > 0 && emt.offString)
				{
					if (!m_strText.IsEmpty())
						m_strText += L' ';
					LPCWSTR pszText = (LPCWSTR)((const BYTE*)pRec + emt.offString);
					m_strText += CStringW(pszText, emt.nChars);
				}
			}
		}
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessGDIRecPolyTextOutW::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRPOLYTEXTOUTW*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
		m_propsCached->AddText(L"iGraphicsMode", GraphicsModeText(pRec->iGraphicsMode));
		m_propsCached->AddValue(L"exScale", pRec->exScale);
		m_propsCached->AddValue(L"eyScale", pRec->eyScale);
		m_propsCached->AddValue(L"cStrings", pRec->cStrings);
		for (LONG ii = 0; ii < pRec->cStrings; ++ii)
		{
			auto& emt = pRec->aemrtext[ii];
			CStringW branchName;
			branchName.Format(L"aemrtext[%d]", ii);
			EmfStruct2Properties::BuildField(CStringA(branchName), emt, m_propsCached.get());
			auto& pBranch = m_propsCached->sub.back();
			for (auto& child : pBranch->sub)
			{
				if (child->name == L"fOptions")
				{
					child->text = ExtTextOutOptionsText(emt.fOptions);
					break;
				}
			}
		}
		auto pszText = GetRecordText();
		if (pszText)
			m_propsCached->AddText(L"Text", pszText);
	}
}

// EMR_SMALLTEXTOUT (record type 108) - undocumented structure
// Data layout (after EMR header): x, y, cChars, fuOptions, iGraphicsMode, exScale, eyScale,
// rclBounds (always present), then text data
struct EMRSMALLTEXTOUT_DATA
{
	LONG    x;
	LONG    y;
	UINT    cChars;
	UINT    fuOptions;
	UINT    iGraphicsMode;
	FLOAT   exScale;
	FLOAT   eyScale;
	RECTL   rclBounds;
};
#define ETO_SMALL_CHARS 0x200

static CStringW MapModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	LPCWSTR label = nullptr;
	switch (iMode)
	{
	case MM_TEXT:		label = L"MM_TEXT"; break;
	case MM_LOMETRIC:	label = L"MM_LOMETRIC"; break;
	case MM_HIMETRIC:	label = L"MM_HIMETRIC"; break;
	case MM_LOENGLISH:	label = L"MM_LOENGLISH"; break;
	case MM_HIENGLISH:	label = L"MM_HIENGLISH"; break;
	case MM_TWIPS:		label = L"MM_TWIPS"; break;
	case MM_ISOTROPIC:	label = L"MM_ISOTROPIC"; break;
	case MM_ANISOTROPIC:label = L"MM_ANISOTROPIC"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW BkModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	if (iMode == TRANSPARENT)	str += L"  TRANSPARENT";
	else if (iMode == OPAQUE)	str += L"  OPAQUE";
	return str;
}

static CStringW PolyFillModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	if (iMode == ALTERNATE)	str += L"  ALTERNATE";
	else if (iMode == WINDING)	str += L"  WINDING";
	return str;
}

static CStringW ROP2Text(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	LPCWSTR label = nullptr;
	switch (iMode)
	{
	case R2_BLACK:		label = L"R2_BLACK"; break;
	case R2_NOTMERGEPEN:label = L"R2_NOTMERGEPEN"; break;
	case R2_MASKNOTPEN:	label = L"R2_MASKNOTPEN"; break;
	case R2_NOTCOPYPEN:	label = L"R2_NOTCOPYPEN"; break;
	case R2_MASKPENNOT:	label = L"R2_MASKPENNOT"; break;
	case R2_NOT:		label = L"R2_NOT"; break;
	case R2_XORPEN:		label = L"R2_XORPEN"; break;
	case R2_NOTMASKPEN:	label = L"R2_NOTMASKPEN"; break;
	case R2_MASKPEN:	label = L"R2_MASKPEN"; break;
	case R2_NOTXORPEN:	label = L"R2_NOTXORPEN"; break;
	case R2_NOP:		label = L"R2_NOP"; break;
	case R2_MERGENOTPEN:label = L"R2_MERGENOTPEN"; break;
	case R2_COPYPEN:	label = L"R2_COPYPEN"; break;
	case R2_MERGEPENNOT:label = L"R2_MERGEPENNOT"; break;
	case R2_MERGEPEN:	label = L"R2_MERGEPEN"; break;
	case R2_WHITE:		label = L"R2_WHITE"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW StretchBltModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	LPCWSTR label = nullptr;
	switch (iMode)
	{
	case BLACKONWHITE:	label = L"BLACKONWHITE"; break;
	case WHITEONBLACK:	label = L"WHITEONBLACK"; break;
	case COLORONCOLOR:	label = L"COLORONCOLOR"; break;
	case HALFTONE:		label = L"HALFTONE"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW TextAlignText(DWORD iMode)
{
	CStringW str;
	str.Format(L"0x%04X", iMode);
	CStringW flags;
	// Horizontal: TA_LEFT=0, TA_RIGHT=2, TA_CENTER=6
	DWORD h = iMode & (TA_CENTER);
	if (h == TA_CENTER)			flags = L"TA_CENTER";
	else if (h == TA_RIGHT)		flags = L"TA_RIGHT";
	else						flags = L"TA_LEFT";
	// Vertical: TA_TOP=0, TA_BOTTOM=8, TA_BASELINE=24
	DWORD v = iMode & (TA_BASELINE);
	if (v == TA_BASELINE)		{ flags += L" | TA_BASELINE"; }
	else if (v == TA_BOTTOM)	{ flags += L" | TA_BOTTOM"; }
	// TA_TOP is default (0), only show if no other flags
	else						{ flags += L" | TA_TOP"; }
	if (iMode & TA_UPDATECP)	flags += L" | TA_UPDATECP";
	if (iMode & TA_RTLREADING)	flags += L" | TA_RTLREADING";
	str += L"  ";
	str += flags;
	return str;
}

static CStringW ArcDirectionText(DWORD iArcDirection)
{
	CStringW str;
	str.Format(L"%u", iArcDirection);
	if (iArcDirection == AD_COUNTERCLOCKWISE)	str += L"  AD_COUNTERCLOCKWISE";
	else if (iArcDirection == AD_CLOCKWISE)		str += L"  AD_CLOCKWISE";
	return str;
}

static CStringW RegionModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	LPCWSTR label = nullptr;
	switch (iMode)
	{
	case RGN_AND:	label = L"RGN_AND"; break;
	case RGN_OR:	label = L"RGN_OR"; break;
	case RGN_XOR:	label = L"RGN_XOR"; break;
	case RGN_DIFF:	label = L"RGN_DIFF"; break;
	case RGN_COPY:	label = L"RGN_COPY"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW ModifyWorldTransformModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	LPCWSTR label = nullptr;
	switch (iMode)
	{
	case MWT_IDENTITY:		label = L"MWT_IDENTITY"; break;
	case MWT_LEFTMULTIPLY:	label = L"MWT_LEFTMULTIPLY"; break;
	case MWT_RIGHTMULTIPLY:	label = L"MWT_RIGHTMULTIPLY"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW FloodFillModeText(DWORD iMode)
{
	CStringW str;
	str.Format(L"%u", iMode);
	if (iMode == FLOODFILLBORDER)		str += L"  FLOODFILLBORDER";
	else if (iMode == FLOODFILLSURFACE)	str += L"  FLOODFILLSURFACE";
	return str;
}

static CStringW GradientFillModeText(DWORD ulMode)
{
	CStringW str;
	str.Format(L"%u", ulMode);
	LPCWSTR label = nullptr;
	switch (ulMode)
	{
	case GRADIENT_FILL_RECT_H:	label = L"GRADIENT_FILL_RECT_H"; break;
	case GRADIENT_FILL_RECT_V:	label = L"GRADIENT_FILL_RECT_V"; break;
	case GRADIENT_FILL_TRIANGLE:label = L"GRADIENT_FILL_TRIANGLE"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW BrushStyleText(DWORD lbStyle)
{
	CStringW str;
	str.Format(L"%u", lbStyle);
	LPCWSTR label = nullptr;
	switch (lbStyle)
	{
	case BS_SOLID:			label = L"BS_SOLID"; break;
	case BS_NULL:			label = L"BS_NULL"; break;
	case BS_HATCHED:		label = L"BS_HATCHED"; break;
	case BS_PATTERN:		label = L"BS_PATTERN"; break;
	case BS_DIBPATTERN:		label = L"BS_DIBPATTERN"; break;
	case BS_DIBPATTERNPT:	label = L"BS_DIBPATTERNPT"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW HatchStyleText(DWORD lbHatch)
{
	CStringW str;
	str.Format(L"%u", lbHatch);
	LPCWSTR label = nullptr;
	switch (lbHatch)
	{
	case HS_HORIZONTAL:	label = L"HS_HORIZONTAL"; break;
	case HS_VERTICAL:	label = L"HS_VERTICAL"; break;
	case HS_FDIAGONAL:	label = L"HS_FDIAGONAL"; break;
	case HS_BDIAGONAL:	label = L"HS_BDIAGONAL"; break;
	case HS_CROSS:		label = L"HS_CROSS"; break;
	case HS_DIAGCROSS:	label = L"HS_DIAGCROSS"; break;
	// EMF-spec hatch styles (not in Windows SDK headers)
	case 6:				label = L"HS_SOLIDCLR"; break;
	case 7:				label = L"HS_DITHEREDCLR"; break;
	case 8:				label = L"HS_SOLIDTEXTCLR"; break;
	case 9:				label = L"HS_DITHEREDTEXTCLR"; break;
	case 10:			label = L"HS_SOLIDBKCLR"; break;
	case 11:			label = L"HS_DITHEREDBKCLR"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW PenStyleText(DWORD lopnStyle)
{
	CStringW str;
	str.Format(L"%u", lopnStyle);
	LPCWSTR label = nullptr;
	switch (lopnStyle)
	{
	case PS_SOLID:		label = L"PS_SOLID"; break;
	case PS_DASH:		label = L"PS_DASH"; break;
	case PS_DOT:		label = L"PS_DOT"; break;
	case PS_DASHDOT:	label = L"PS_DASHDOT"; break;
	case PS_DASHDOTDOT:	label = L"PS_DASHDOTDOT"; break;
	case PS_NULL:		label = L"PS_NULL"; break;
	case PS_INSIDEFRAME:label = L"PS_INSIDEFRAME"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW ExtPenStyleText(DWORD elpPenStyle)
{
	CStringW str;
	str.Format(L"0x%08X", elpPenStyle);
	CStringW flags;
	// Type (PS_SOLID..PS_INSIDEFRAME, PS_USERSTYLE, PS_ALTERNATE)
	DWORD type = elpPenStyle & PS_STYLE_MASK;
	switch (type)
	{
	case PS_SOLID:			flags = L"PS_SOLID"; break;
	case PS_DASH:			flags = L"PS_DASH"; break;
	case PS_DOT:			flags = L"PS_DOT"; break;
	case PS_DASHDOT:		flags = L"PS_DASHDOT"; break;
	case PS_DASHDOTDOT:		flags = L"PS_DASHDOTDOT"; break;
	case PS_NULL:			flags = L"PS_NULL"; break;
	case PS_INSIDEFRAME:	flags = L"PS_INSIDEFRAME"; break;
	case PS_USERSTYLE:		flags = L"PS_USERSTYLE"; break;
	case PS_ALTERNATE:		flags = L"PS_ALTERNATE"; break;
	}
	// End cap
	DWORD endcap = elpPenStyle & PS_ENDCAP_MASK;
	if (endcap == PS_ENDCAP_ROUND)		flags += L" | PS_ENDCAP_ROUND";
	else if (endcap == PS_ENDCAP_SQUARE)flags += L" | PS_ENDCAP_SQUARE";
	else if (endcap == PS_ENDCAP_FLAT)	flags += L" | PS_ENDCAP_FLAT";
	// Join
	DWORD join = elpPenStyle & PS_JOIN_MASK;
	if (join == PS_JOIN_ROUND)		flags += L" | PS_JOIN_ROUND";
	else if (join == PS_JOIN_BEVEL)	flags += L" | PS_JOIN_BEVEL";
	else if (join == PS_JOIN_MITER)	flags += L" | PS_JOIN_MITER";
	// Type (cosmetic/geometric)
	DWORD geotype = elpPenStyle & PS_TYPE_MASK;
	if (geotype == PS_GEOMETRIC)	flags += L" | PS_GEOMETRIC";
	else							flags += L" | PS_COSMETIC";
	if (!flags.IsEmpty())
	{
		str += L"  ";
		str += flags;
	}
	return str;
}

static CStringW FontWeightText(LONG lfWeight)
{
	CStringW str;
	str.Format(L"%d", lfWeight);
	LPCWSTR label = nullptr;
	switch (lfWeight)
	{
	case FW_THIN:		label = L"FW_THIN"; break;
	case FW_EXTRALIGHT:	label = L"FW_EXTRALIGHT"; break;
	case FW_LIGHT:		label = L"FW_LIGHT"; break;
	case FW_NORMAL:		label = L"FW_NORMAL"; break;
	case FW_MEDIUM:		label = L"FW_MEDIUM"; break;
	case FW_SEMIBOLD:	label = L"FW_SEMIBOLD"; break;
	case FW_BOLD:		label = L"FW_BOLD"; break;
	case FW_EXTRABOLD:	label = L"FW_EXTRABOLD"; break;
	case FW_HEAVY:		label = L"FW_HEAVY"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW CharSetText(BYTE lfCharSet)
{
	CStringW str;
	str.Format(L"%u", lfCharSet);
	LPCWSTR label = nullptr;
	switch (lfCharSet)
	{
	case ANSI_CHARSET:			label = L"ANSI_CHARSET"; break;
	case DEFAULT_CHARSET:		label = L"DEFAULT_CHARSET"; break;
	case SYMBOL_CHARSET:		label = L"SYMBOL_CHARSET"; break;
	case SHIFTJIS_CHARSET:		label = L"SHIFTJIS_CHARSET"; break;
	case HANGUL_CHARSET:		label = L"HANGUL_CHARSET"; break;
	case GB2312_CHARSET:		label = L"GB2312_CHARSET"; break;
	case CHINESEBIG5_CHARSET:	label = L"CHINESEBIG5_CHARSET"; break;
	case OEM_CHARSET:			label = L"OEM_CHARSET"; break;
	case JOHAB_CHARSET:			label = L"JOHAB_CHARSET"; break;
	case HEBREW_CHARSET:		label = L"HEBREW_CHARSET"; break;
	case ARABIC_CHARSET:		label = L"ARABIC_CHARSET"; break;
	case GREEK_CHARSET:			label = L"GREEK_CHARSET"; break;
	case TURKISH_CHARSET:		label = L"TURKISH_CHARSET"; break;
	case VIETNAMESE_CHARSET:	label = L"VIETNAMESE_CHARSET"; break;
	case THAI_CHARSET:			label = L"THAI_CHARSET"; break;
	case EASTEUROPE_CHARSET:	label = L"EASTEUROPE_CHARSET"; break;
	case RUSSIAN_CHARSET:		label = L"RUSSIAN_CHARSET"; break;
	case MAC_CHARSET:			label = L"MAC_CHARSET"; break;
	case BALTIC_CHARSET:		label = L"BALTIC_CHARSET"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW FontQualityText(BYTE lfQuality)
{
	CStringW str;
	str.Format(L"%u", lfQuality);
	LPCWSTR label = nullptr;
	switch (lfQuality)
	{
	case DEFAULT_QUALITY:			label = L"DEFAULT_QUALITY"; break;
	case DRAFT_QUALITY:				label = L"DRAFT_QUALITY"; break;
	case PROOF_QUALITY:				label = L"PROOF_QUALITY"; break;
	case NONANTIALIASED_QUALITY:	label = L"NONANTIALIASED_QUALITY"; break;
	case ANTIALIASED_QUALITY:		label = L"ANTIALIASED_QUALITY"; break;
	case CLEARTYPE_QUALITY:			label = L"CLEARTYPE_QUALITY"; break;
	case CLEARTYPE_NATURAL_QUALITY:	label = L"CLEARTYPE_NATURAL_QUALITY"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

static CStringW ExtTextOutOptionsText(UINT fuOptions)
{
	CStringW str;
	str.Format(L"0x%08X", fuOptions);
	CStringW flags;
	if (fuOptions & ETO_OPAQUE)		flags += L"ETO_OPAQUE";
	if (fuOptions & ETO_CLIPPED)	{ if (!flags.IsEmpty()) flags += L" | "; flags += L"ETO_CLIPPED"; }
	if (fuOptions & ETO_GLYPH_INDEX){ if (!flags.IsEmpty()) flags += L" | "; flags += L"ETO_GLYPH_INDEX"; }
	if (fuOptions & ETO_RTLREADING)	{ if (!flags.IsEmpty()) flags += L" | "; flags += L"ETO_RTLREADING"; }
	if (fuOptions & ETO_SMALL_CHARS){ if (!flags.IsEmpty()) flags += L" | "; flags += L"ETO_SMALL_CHARS (ANSI)"; }
	if (fuOptions & ETO_PDY)		{ if (!flags.IsEmpty()) flags += L" | "; flags += L"ETO_PDY"; }
	if (!flags.IsEmpty())
	{
		str += L"  ";
		str += flags;
	}
	if (!(fuOptions & ETO_SMALL_CHARS))
	{
		if (!flags.IsEmpty()) str += L" | ";
		else str += L"  ";
		str += L"Unicode";
	}
	return str;
}

static CStringW GraphicsModeText(UINT iGraphicsMode)
{
	CStringW str;
	str.Format(L"%u", iGraphicsMode);
	if (iGraphicsMode == GM_COMPATIBLE)
		str += L"  GM_COMPATIBLE";
	else if (iGraphicsMode == GM_ADVANCED)
		str += L"  GM_ADVANCED";
	return str;
}

static CStringW RasterOpText(DWORD dwRop)
{
	CStringW str;
	// Check for high-bit flags first
	CStringW flags;
	DWORD baseRop = dwRop;
	if (dwRop & 0x80000000)
	{
		flags = L"NOMIRRORBITMAP";
		baseRop &= ~0x80000000;
	}
	if (dwRop & 0x40000000)
	{
		if (!flags.IsEmpty()) flags += L" | ";
		flags += L"CAPTUREBLT";
		baseRop &= ~0x40000000;
	}
	str.Format(L"0x%08X", dwRop);
	LPCWSTR label = nullptr;
	switch (baseRop)
	{
	case SRCCOPY:		label = L"SRCCOPY"; break;
	case SRCPAINT:		label = L"SRCPAINT"; break;
	case SRCAND:		label = L"SRCAND"; break;
	case SRCINVERT:		label = L"SRCINVERT"; break;
	case SRCERASE:		label = L"SRCERASE"; break;
	case NOTSRCCOPY:	label = L"NOTSRCCOPY"; break;
	case NOTSRCERASE:	label = L"NOTSRCERASE"; break;
	case MERGECOPY:		label = L"MERGECOPY"; break;
	case MERGEPAINT:	label = L"MERGEPAINT"; break;
	case PATCOPY:		label = L"PATCOPY"; break;
	case PATPAINT:		label = L"PATPAINT"; break;
	case PATINVERT:		label = L"PATINVERT"; break;
	case DSTINVERT:		label = L"DSTINVERT"; break;
	case BLACKNESS:		label = L"BLACKNESS"; break;
	case WHITENESS:		label = L"WHITENESS"; break;
	}
	if (label || !flags.IsEmpty())
	{
		str += L"  ";
		if (!flags.IsEmpty())
		{
			str += flags;
			if (label) { str += L" | "; str += label; }
		}
		else if (label)
			str += label;
	}
	return str;
}

static CStringW BlendFunctionText(DWORD dwRop)
{
	// dwRop is a packed BLENDFUNCTION: byte 0=BlendOp, 1=BlendFlags, 2=SourceConstantAlpha, 3=AlphaFormat
	BYTE blendOp = (BYTE)(dwRop & 0xFF);
	BYTE blendFlags = (BYTE)((dwRop >> 8) & 0xFF);
	BYTE srcAlpha = (BYTE)((dwRop >> 16) & 0xFF);
	BYTE alphaFormat = (BYTE)((dwRop >> 24) & 0xFF);
	CStringW str;
	if (blendOp == AC_SRC_OVER)
		str = L"AC_SRC_OVER";
	else
		str.Format(L"BlendOp=%u", blendOp);
	if (blendFlags != 0)
	{
		CStringW tmp;
		tmp.Format(L", BlendFlags=%u", blendFlags);
		str += tmp;
	}
	CStringW tmp;
	tmp.Format(L", Alpha=%u", srcAlpha);
	str += tmp;
	if (alphaFormat & AC_SRC_ALPHA)
		str += L", AC_SRC_ALPHA";
	else if (alphaFormat != 0)
	{
		tmp.Format(L", AlphaFormat=%u", alphaFormat);
		str += tmp;
	}
	return str;
}

static CStringW DIBColorsText(DWORD iUsage)
{
	CStringW str;
	str.Format(L"%u", iUsage);
	if (iUsage == DIB_RGB_COLORS)
		str += L"  DIB_RGB_COLORS";
	else if (iUsage == DIB_PAL_COLORS)
		str += L"  DIB_PAL_COLORS";
	return str;
}

static CStringW BiCompressionText(DWORD biCompression)
{
	CStringW str;
	str.Format(L"%u", biCompression);
	LPCWSTR label = nullptr;
	switch (biCompression)
	{
	case BI_RGB:		label = L"BI_RGB"; break;
	case BI_RLE8:		label = L"BI_RLE8"; break;
	case BI_RLE4:		label = L"BI_RLE4"; break;
	case BI_BITFIELDS:	label = L"BI_BITFIELDS"; break;
	case BI_JPEG:		label = L"BI_JPEG"; break;
	case BI_PNG:		label = L"BI_PNG"; break;
	}
	if (label) { str += L"  "; str += label; }
	return str;
}

LPCWSTR EMFRecAccessGDIRecSmallTextOut::GetRecordText() const
{
	if (m_strText.IsEmpty() && m_recInfo.Data && m_recInfo.DataSize >= sizeof(EMRSMALLTEXTOUT_DATA))
	{
		auto pRec = (const EMRSMALLTEXTOUT_DATA*)m_recInfo.Data;
		if (pRec->cChars > 0 && !(pRec->fuOptions & ETO_GLYPH_INDEX))
		{
			auto pText = m_recInfo.Data + sizeof(EMRSMALLTEXTOUT_DATA);
			if (pRec->fuOptions & ETO_SMALL_CHARS)
			{
				if (sizeof(EMRSMALLTEXTOUT_DATA) + pRec->cChars <= m_recInfo.DataSize)
					m_strText = CStringW((LPCSTR)pText, pRec->cChars);
			}
			else
			{
				if (sizeof(EMRSMALLTEXTOUT_DATA) + pRec->cChars * sizeof(WCHAR) <= m_recInfo.DataSize)
					m_strText.SetString((LPCWSTR)pText, pRec->cChars);
			}
		}
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessGDIRecSmallTextOut::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	if (m_recInfo.Data && m_recInfo.DataSize >= sizeof(EMRSMALLTEXTOUT_DATA))
	{
		auto pRec = (const EMRSMALLTEXTOUT_DATA*)m_recInfo.Data;
		m_propsCached->AddValue(L"x", pRec->x);
		m_propsCached->AddValue(L"y", pRec->y);
		m_propsCached->AddValue(L"cChars", pRec->cChars);
		m_propsCached->AddText(L"fuOptions", ExtTextOutOptionsText(pRec->fuOptions));
		m_propsCached->AddText(L"iGraphicsMode", GraphicsModeText(pRec->iGraphicsMode));
		m_propsCached->AddValue(L"exScale", pRec->exScale);
		m_propsCached->AddValue(L"eyScale", pRec->eyScale);
		if (pRec->fuOptions & ETO_GLYPH_INDEX)
		{
			if (pRec->cChars > 0)
			{
				auto pGlyphs = (const emfplus::u16t*)(m_recInfo.Data + sizeof(EMRSMALLTEXTOUT_DATA));
				if (sizeof(EMRSMALLTEXTOUT_DATA) + pRec->cChars * sizeof(emfplus::u16t) <= m_recInfo.DataSize)
					m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeArray>(L"Glyphs", pGlyphs, (size_t)pRec->cChars));
			}
		}
		else
		{
			auto pszText = GetRecordText();
			if (pszText)
				m_propsCached->AddText(L"Text", pszText);
		}
	}
}

void EMFRecAccessGDIRecSetICMMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETICMMODE*)EMFRecAccessGDIRec::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecPixelFormat::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRPIXELFORMAT*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetICMProfileA::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETICMPROFILEA*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetICMProfileW::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETICMPROFILEW*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecSetLayout::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRSETLAYOUT*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}

void EMFRecAccessGDIRecGradientFill::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIDrawingCat::CacheProperties(ctxt);
	auto pRec = (EMRGRADIENTFILL*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(L"rclBounds", pRec->rclBounds));
		m_propsCached->AddValue(L"nVer", pRec->nVer);
		m_propsCached->AddValue(L"nTri", pRec->nTri);
		m_propsCached->AddText(L"ulMode", GradientFillModeText(pRec->ulMode));
	}
}

void EMFRecAccessGDIRecColorMatchToTargetW::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessGDIStateCat::CacheProperties(ctxt);
	auto pRec = (EMRCOLORMATCHTOTARGET*)EMFRecAccessGDIObjectCat::GetGDIRecord(m_recInfo);
	if (pRec)
	{
		EmfStruct2Properties::Build(*pRec, m_propsCached.get());
	}
}
