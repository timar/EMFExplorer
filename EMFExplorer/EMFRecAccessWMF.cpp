#include "pch.h"
#include "framework.h"
#include "EMFRecAccessWMF.h"
#include "EMFAccess.h"
#include "EMFStruct2Props.h"
#include "WmfStruct.h"
#include "EMFRecAccessGDITextHelpers.h"

#undef min
#undef max

using namespace wmf;

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

namespace
{
	// MS-WMF [2.1.1.20] MixMode (background mode)
	CStringW WmfBkModeText(uint16_t mode)
	{
		// Same numeric values as GDI BkMode
		return BkModeText((DWORD)mode);
	}

	// MS-WMF [2.1.1.16] MapMode
	CStringW WmfMapModeText(uint16_t mode)
	{
		return MapModeText((DWORD)mode);
	}

	// MS-WMF [2.1.1.27] PolyFillMode (1=ALTERNATE, 2=WINDING)
	CStringW WmfPolyFillModeText(uint16_t mode)
	{
		return PolyFillModeText((DWORD)mode);
	}

	// MS-WMF binary RasterOp values [2.1.1.2] (different from EMF ROP3 values).
	// These are the 16-bit DrawMode codes for SetROP2.
	CStringW WmfROP2Text(uint16_t mode)
	{
		return ROP2Text((DWORD)mode);
	}

	CStringW WmfStretchModeText(uint16_t mode)
	{
		return StretchBltModeText((DWORD)mode);
	}

	CStringW WmfTextAlignText(uint16_t align)
	{
		return TextAlignText((DWORD)align);
	}

	// MS-WMF [2.1.1.20] FloodFill type (FLOODFILLBORDER vs FLOODFILLSURFACE)
	CStringW WmfFloodFillModeText(uint16_t mode)
	{
		return FloodFillModeText((DWORD)mode);
	}

	CStringW WmfBrushStyleText(uint16_t style)	{ return BrushStyleText((DWORD)style); }
	CStringW WmfHatchStyleText(uint16_t style)	{ return HatchStyleText((DWORD)style); }
	CStringW WmfPenStyleText(uint16_t style)	{ return PenStyleText((DWORD)style); }
	CStringW WmfFontWeightText(int16_t  w)		{ return FontWeightText((LONG)w); }
	CStringW WmfCharSetText(uint8_t  v)			{ return CharSetText(v); }
	CStringW WmfFontQualityText(uint8_t  v)		{ return FontQualityText(v); }
	CStringW WmfRasterOpText(uint32_t r)		{ return RasterOpText(r); }
	CStringW WmfDIBColorsText(uint16_t v)		{ return DIBColorsText((DWORD)v); }

	CStringW WmfLayoutText(uint16_t layout)
	{
		CStringW str;
		str.Format(L"0x%04X", layout);
		CStringW flags;
		if (layout & LAYOUT_RTL)								flags = L"LAYOUT_RTL";
		if (layout & LAYOUT_BTT)								{ if (!flags.IsEmpty()) flags += L" | "; flags += L"LAYOUT_BTT"; }
		if (layout & LAYOUT_VBH)								{ if (!flags.IsEmpty()) flags += L" | "; flags += L"LAYOUT_VBH"; }
		if (layout & LAYOUT_BITMAPORIENTATIONPRESERVED)			{ if (!flags.IsEmpty()) flags += L" | "; flags += L"LAYOUT_BITMAPORIENTATIONPRESERVED"; }
		if (!flags.IsEmpty()) { str += L"  "; str += flags; }
		return str;
	}

	// MS-WMF [2.1.1.6] ExtTextOutOptions / fwOpts. WMF uses a smaller subset
	// than EMF; reuse the EMF helper for the common bits.
	CStringW WmfExtTextOutOptionsText(uint16_t opts)
	{
		return ExtTextOutOptionsText((UINT)opts);
	}

	// Add params common to record-data display
	void AddPoint(PropertyNode* pNode, LPCWSTR name, int16_t x, int16_t y)
	{
		auto pBranch = pNode->AddBranch(name);
		pBranch->AddValue(L"x", (int)x);
		pBranch->AddValue(L"y", (int)y);
	}

	void AddRect(PropertyNode* pNode, LPCWSTR name, int16_t L_, int16_t T_, int16_t R_, int16_t B_)
	{
		pNode->sub.emplace_back(std::make_shared<PropertyNodeRectInt>(name, (LONG)L_, (LONG)T_, (LONG)R_, (LONG)B_));
	}

	void AddColor(PropertyNode* pNode, LPCWSTR name, uint32_t crColor)
	{
		pNode->sub.emplace_back(std::make_shared<PropertyNodeColor>(name,
			emfplus::OEmfPlusARGB::FromCOLORREF((COLORREF)crColor)));
	}

	// Read params with a bounded view over rec.Data so we never read past it.
	struct WmfParamView
	{
		const uint8_t*	data;
		size_t			size;

		template <typename T>
		bool Read(size_t off, T& out) const
		{
			if (off + sizeof(T) > size) return false;
			memcpy(&out, data + off, sizeof(T));
			return true;
		}

		template <typename T>
		const T* As(size_t off = 0, size_t need = sizeof(T)) const
		{
			if (off + need > size) return nullptr;
			return reinterpret_cast<const T*>(data + off);
		}
	};

	WmfParamView View(const emfplus::OEmfPlusRecInfo& rec)
	{
		return WmfParamView{ (const uint8_t*)rec.Data, (size_t)rec.DataSize };
	}
} // namespace

// -----------------------------------------------------------------------
// State records (simple)
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecSetBkColor::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetBkColor>();
	if (pRec) AddColor(m_propsCached.get(), L"ColorRef", pRec->ColorRef);
}

void EMFRecAccessWMFRecSetBkMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetBkMode>();
	if (pRec) m_propsCached->AddText(L"BkMode", WmfBkModeText(pRec->BkMode));
}

void EMFRecAccessWMFRecSetMapMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetMapMode>();
	if (pRec) m_propsCached->AddText(L"MapMode", WmfMapModeText(pRec->MapMode));
}

void EMFRecAccessWMFRecSetROP2::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetROP2>();
	if (pRec) m_propsCached->AddText(L"DrawMode", WmfROP2Text(pRec->DrawMode));
}

void EMFRecAccessWMFRecSetRelAbs::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetRelAbs>();
	if (pRec) m_propsCached->AddValue(L"Mode", pRec->Mode);
}

void EMFRecAccessWMFRecSetPolyFillMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetPolyFillMode>();
	if (pRec) m_propsCached->AddText(L"PolyFillMode", WmfPolyFillModeText(pRec->Mode));
}

void EMFRecAccessWMFRecSetStretchBltMode::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetStretchBltMode>();
	if (pRec) m_propsCached->AddText(L"StretchMode", WmfStretchModeText(pRec->Mode));
}

void EMFRecAccessWMFRecSetTextCharExtra::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetTextCharExtra>();
	if (pRec) m_propsCached->AddValue(L"CharExtra", (int)pRec->CharExtra);
}

void EMFRecAccessWMFRecSetTextColor::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetTextColor>();
	if (pRec) AddColor(m_propsCached.get(), L"ColorRef", pRec->ColorRef);
}

void EMFRecAccessWMFRecSetTextJustification::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetTextJustification>();
	if (pRec)
	{
		m_propsCached->AddValue(L"BreakExtra", (int)pRec->BreakExtra);
		m_propsCached->AddValue(L"BreakCount", (int)pRec->BreakCount);
	}
}

static void WmfCachePoint(std::shared_ptr<PropertyNode>& pNode,
	const emfplus::OEmfPlusRecInfo& rec, LPCWSTR fieldName)
{
	auto pRec = View(rec).As<OWmfSetWindowOrg>();
	if (pRec) AddPoint(pNode.get(), fieldName, pRec->X, pRec->Y);
}

void EMFRecAccessWMFRecSetWindowOrg::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Origin");
}
void EMFRecAccessWMFRecSetWindowExt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Extent");
}
void EMFRecAccessWMFRecSetViewportOrg::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Origin");
}
void EMFRecAccessWMFRecSetViewportExt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Extent");
}
void EMFRecAccessWMFRecOffsetWindowOrg::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Offset");
}
void EMFRecAccessWMFRecOffsetViewportOrg::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	WmfCachePoint(m_propsCached, m_recInfo, L"Offset");
}

void EMFRecAccessWMFRecScaleWindowExt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfScaleWindowExt>();
	if (pRec)
	{
		m_propsCached->AddValue(L"yDenom", (int)pRec->yDenom);
		m_propsCached->AddValue(L"yNum",   (int)pRec->yNum);
		m_propsCached->AddValue(L"xDenom", (int)pRec->xDenom);
		m_propsCached->AddValue(L"xNum",   (int)pRec->xNum);
	}
}

void EMFRecAccessWMFRecScaleViewportExt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfScaleViewportExt>();
	if (pRec)
	{
		m_propsCached->AddValue(L"yDenom", (int)pRec->yDenom);
		m_propsCached->AddValue(L"yNum",   (int)pRec->yNum);
		m_propsCached->AddValue(L"xDenom", (int)pRec->xDenom);
		m_propsCached->AddValue(L"xNum",   (int)pRec->xNum);
	}
}

void EMFRecAccessWMFRecSetTextAlign::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetTextAlign>();
	if (pRec) m_propsCached->AddText(L"TextAlign", WmfTextAlignText(pRec->TextAlign));
}

void EMFRecAccessWMFRecSetMapperFlags::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetMapperFlags>();
	if (pRec) m_propsCached->AddValue(L"MapperValues", pRec->MapperValues, true);
}

void EMFRecAccessWMFRecSetLayout::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetLayout>();
	if (pRec) m_propsCached->AddText(L"Layout", WmfLayoutText(pRec->Layout));
}

void EMFRecAccessWMFRecResizePalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfResizePalette>();
	if (pRec) m_propsCached->AddValue(L"NumberOfEntries", (int)pRec->NumberOfEntries);
}

// -----------------------------------------------------------------------
// SaveDC / RestoreDC linkage
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecRestoreDC::Preprocess(EMFAccess* pEMF)
{
	auto pRec = View(m_recInfo).As<OWmfRestoreDC>();
	if (!pRec) return;
	auto pSaveRec = pEMF->GetGDISaveRecord(pRec->nSavedDC);
	if (pSaveRec)
		AddLinkRecord(pSaveRec, LinkedObjTypeGraphicState, LinkedObjTypeGraphicState);
}

void EMFRecAccessWMFRecRestoreDC::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfRestoreDC>();
	if (pRec) m_propsCached->AddValue(L"nSavedDC", (int)pRec->nSavedDC);
}

// -----------------------------------------------------------------------
// Drawing records
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecLineTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfLineTo>();
	if (pRec) AddPoint(m_propsCached.get(), L"Point", pRec->X, pRec->Y);
}

void EMFRecAccessWMFRecMoveTo::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfMoveTo>();
	if (pRec) AddPoint(m_propsCached.get(), L"Point", pRec->X, pRec->Y);
}

#define WMF_DEFINE_RECT_DRAWING(_Class) \
	void _Class::CacheProperties(const CachePropertiesContext& ctxt) \
	{ \
		EMFRecAccessWMFDrawingCat::CacheProperties(ctxt); \
		auto pRec = View(m_recInfo).As<OWmfRectangle>(); \
		if (pRec) AddRect(m_propsCached.get(), L"Rect", pRec->Left, pRec->Top, pRec->Right, pRec->Bottom); \
	}

WMF_DEFINE_RECT_DRAWING(EMFRecAccessWMFRecEllipse);
WMF_DEFINE_RECT_DRAWING(EMFRecAccessWMFRecRectangle);

void EMFRecAccessWMFRecRoundRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfRoundRect>();
	if (pRec)
	{
		AddRect(m_propsCached.get(), L"Rect", pRec->Left, pRec->Top, pRec->Right, pRec->Bottom);
		m_propsCached->AddValue(L"Width",  (int)pRec->Width);
		m_propsCached->AddValue(L"Height", (int)pRec->Height);
	}
}

#define WMF_DEFINE_ARCLIKE(_Class) \
	void _Class::CacheProperties(const CachePropertiesContext& ctxt) \
	{ \
		EMFRecAccessWMFDrawingCat::CacheProperties(ctxt); \
		auto pRec = View(m_recInfo).As<OWmfArc>(); \
		if (pRec) { \
			AddRect(m_propsCached.get(), L"Rect", pRec->Left, pRec->Top, pRec->Right, pRec->Bottom); \
			AddPoint(m_propsCached.get(), L"Start", pRec->XStartArc, pRec->YStartArc); \
			AddPoint(m_propsCached.get(), L"End",   pRec->XEndArc,   pRec->YEndArc); \
		} \
	}

WMF_DEFINE_ARCLIKE(EMFRecAccessWMFRecArc);
WMF_DEFINE_ARCLIKE(EMFRecAccessWMFRecChord);
WMF_DEFINE_ARCLIKE(EMFRecAccessWMFRecPie);

void EMFRecAccessWMFRecFloodFill::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfFloodFill>();
	if (pRec)
	{
		AddColor(m_propsCached.get(), L"ColorRef", pRec->ColorRef);
		AddPoint(m_propsCached.get(), L"Point", pRec->X, pRec->Y);
	}
}

void EMFRecAccessWMFRecExtFloodFill::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfExtFloodFill>();
	if (pRec)
	{
		m_propsCached->AddText(L"Mode", WmfFloodFillModeText(pRec->Mode));
		AddColor(m_propsCached.get(), L"ColorRef", pRec->ColorRef);
		AddPoint(m_propsCached.get(), L"Point", pRec->X, pRec->Y);
	}
}

void EMFRecAccessWMFRecSetPixel::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetPixel>();
	if (pRec)
	{
		AddColor(m_propsCached.get(), L"ColorRef", pRec->ColorRef);
		AddPoint(m_propsCached.get(), L"Point", pRec->X, pRec->Y);
	}
}

void EMFRecAccessWMFRecPatBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfPatBlt>();
	if (pRec)
	{
		m_propsCached->AddText(L"RasterOp", WmfRasterOpText(pRec->RasterOp));
		AddPoint(m_propsCached.get(), L"Origin", pRec->XLeft, pRec->YLeft);
		m_propsCached->AddValue(L"Width",  (int)pRec->Width);
		m_propsCached->AddValue(L"Height", (int)pRec->Height);
	}
}

static void AddWmfPolyPoints(PropertyNode* pNode, LPCWSTR name, const WmfPointS* pts, size_t n)
{
	if (n == 0) { pNode->AddValue(name, (int)0); return; }
	auto pBranch = pNode->AddBranch(name);
	pBranch->text.Format(L"%zu points", n);
	for (size_t i = 0; i < n; ++i)
	{
		CStringW idx; idx.Format(L"[%zu]", i);
		AddPoint(pBranch.get(), idx, pts[i].x, pts[i].y);
	}
}

void EMFRecAccessWMFRecPolygon::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pHdr = v.As<OWmfPolygonHeader>();
	if (!pHdr) return;
	const auto n = (size_t)(pHdr->NumberOfPoints < 0 ? 0 : pHdr->NumberOfPoints);
	auto pPts = v.As<WmfPointS>(sizeof(*pHdr), n * sizeof(WmfPointS));
	m_propsCached->AddValue(L"NumberOfPoints", (int)pHdr->NumberOfPoints);
	if (pPts) AddWmfPolyPoints(m_propsCached.get(), L"Points", pPts, n);
}

void EMFRecAccessWMFRecPolyline::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pHdr = v.As<OWmfPolylineHeader>();
	if (!pHdr) return;
	const auto n = (size_t)(pHdr->NumberOfPoints < 0 ? 0 : pHdr->NumberOfPoints);
	auto pPts = v.As<WmfPointS>(sizeof(*pHdr), n * sizeof(WmfPointS));
	m_propsCached->AddValue(L"NumberOfPoints", (int)pHdr->NumberOfPoints);
	if (pPts) AddWmfPolyPoints(m_propsCached.get(), L"Points", pPts, n);
}

void EMFRecAccessWMFRecPolyPolygon::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pHdr = v.As<OWmfPolyPolygonHeader>();
	if (!pHdr) return;
	const size_t nPolys = pHdr->NumberOfPolygons;
	auto pCounts = v.As<uint16_t>(sizeof(*pHdr), nPolys * sizeof(uint16_t));
	if (!pCounts) { m_propsCached->AddValue(L"NumberOfPolygons", (int)nPolys); return; }
	size_t totalPts = 0;
	for (size_t i = 0; i < nPolys; ++i) totalPts += pCounts[i];
	auto pPts = v.As<WmfPointS>(sizeof(*pHdr) + nPolys * sizeof(uint16_t), totalPts * sizeof(WmfPointS));
	m_propsCached->AddValue(L"NumberOfPolygons", (int)nPolys);
	auto pPolys = m_propsCached->AddBranch(L"Polygons");
	const WmfPointS* p = pPts;
	for (size_t i = 0; i < nPolys && p; ++i)
	{
		CStringW idx; idx.Format(L"[%zu]", i);
		auto pPoly = pPolys->AddBranch(idx);
		size_t k = pCounts[i];
		pPoly->text.Format(L"%zu points", k);
		AddWmfPolyPoints(pPoly.get(), L"Points", p, k);
		p += k;
	}
}

// -----------------------------------------------------------------------
// Text records
// -----------------------------------------------------------------------

LPCWSTR EMFRecAccessWMFRecTextOut::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto v = View(m_recInfo);
		auto pHdr = v.As<OWmfTextOutHeader>();
		if (!pHdr) return nullptr;
		auto n = pHdr->StringLength;
		if (n <= 0) return nullptr;
		auto pStr = v.As<char>(sizeof(*pHdr), (size_t)n);
		if (pStr)
			m_strText = CStringW(pStr, (int)n);
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessWMFRecTextOut::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pHdr = v.As<OWmfTextOutHeader>();
	if (!pHdr) return;
	const auto strLen = pHdr->StringLength;
	m_propsCached->AddValue(L"StringLength", (int)strLen);
	auto pszText = GetRecordText();
	if (pszText) m_propsCached->AddText(L"String", pszText);
	// After string (padded to even length), YStart and XStart follow.
	const size_t padded = (strLen <= 0) ? 0 : ((strLen + 1) & ~1);
	const size_t off = sizeof(*pHdr) + padded;
	int16_t YStart = 0, XStart = 0;
	if (v.Read(off, YStart) && v.Read(off + sizeof(int16_t), XStart))
		AddPoint(m_propsCached.get(), L"Origin", XStart, YStart);
}

LPCWSTR EMFRecAccessWMFRecExtTextOut::GetRecordText() const
{
	if (m_strText.IsEmpty())
	{
		auto v = View(m_recInfo);
		auto pHdr = v.As<OWmfExtTextOutHeader>();
		if (!pHdr) return nullptr;
		auto n = pHdr->StringLength;
		if (n <= 0 || (pHdr->fwOpts & ETO_GLYPH_INDEX)) return nullptr;
		size_t off = sizeof(*pHdr);
		if (pHdr->fwOpts & (ETO_OPAQUE | ETO_CLIPPED)) off += 4 * sizeof(int16_t);
		auto pStr = v.As<char>(off, (size_t)n);
		if (pStr)
			m_strText = CStringW(pStr, (int)n);
	}
	return m_strText.IsEmpty() ? nullptr : (LPCWSTR)m_strText;
}

void EMFRecAccessWMFRecExtTextOut::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pHdr = v.As<OWmfExtTextOutHeader>();
	if (!pHdr) return;

	AddPoint(m_propsCached.get(), L"Origin", pHdr->X, pHdr->Y);
	m_propsCached->AddValue(L"StringLength", (int)pHdr->StringLength);
	m_propsCached->AddText(L"fwOpts", WmfExtTextOutOptionsText(pHdr->fwOpts));

	size_t off = sizeof(*pHdr);
	if (pHdr->fwOpts & (ETO_OPAQUE | ETO_CLIPPED))
	{
		int16_t L_ = 0, T_ = 0, R_ = 0, B_ = 0;
		if (v.Read(off, L_) &&
			v.Read(off + 2, T_) &&
			v.Read(off + 4, R_) &&
			v.Read(off + 6, B_))
		{
			AddRect(m_propsCached.get(), L"Rectangle", L_, T_, R_, B_);
		}
		off += 4 * sizeof(int16_t);
	}

	if (pHdr->StringLength > 0)
	{
		if (pHdr->fwOpts & ETO_GLYPH_INDEX)
		{
			auto pGlyphs = v.As<uint16_t>(off, pHdr->StringLength * sizeof(uint16_t));
			if (pGlyphs)
				m_propsCached->sub.emplace_back(std::make_shared<PropertyNodeArray>(L"Glyphs", pGlyphs, (size_t)pHdr->StringLength));
		}
		else
		{
			auto pszText = GetRecordText();
			if (pszText) m_propsCached->AddText(L"String", pszText);
		}
	}
}

// -----------------------------------------------------------------------
// Bitmap records
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecBitBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfBitBlt>();
	if (!pRec) return;
	m_propsCached->AddText(L"RasterOp", WmfRasterOpText(pRec->RasterOp));
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"Width",  (int)pRec->Width);
	m_propsCached->AddValue(L"Height", (int)pRec->Height);
}

void EMFRecAccessWMFRecStretchBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfStretchBlt>();
	if (!pRec) return;
	m_propsCached->AddText(L"RasterOp", WmfRasterOpText(pRec->RasterOp));
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	m_propsCached->AddValue(L"SrcWidth",  (int)pRec->SrcWidth);
	m_propsCached->AddValue(L"SrcHeight", (int)pRec->SrcHeight);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"DestWidth",  (int)pRec->DestWidth);
	m_propsCached->AddValue(L"DestHeight", (int)pRec->DestHeight);
}

void EMFRecAccessWMFRecDIBBitBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfDIBBitBlt>();
	if (!pRec) return;
	m_propsCached->AddText(L"RasterOp", WmfRasterOpText(pRec->RasterOp));
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"Width",  (int)pRec->Width);
	m_propsCached->AddValue(L"Height", (int)pRec->Height);
}

void EMFRecAccessWMFRecDIBStretchBlt::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfDIBStretchBlt>();
	if (!pRec) return;
	m_propsCached->AddText(L"RasterOp", WmfRasterOpText(pRec->RasterOp));
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	m_propsCached->AddValue(L"SrcWidth",  (int)pRec->SrcWidth);
	m_propsCached->AddValue(L"SrcHeight", (int)pRec->SrcHeight);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"DestWidth",  (int)pRec->DestWidth);
	m_propsCached->AddValue(L"DestHeight", (int)pRec->DestHeight);
}

void EMFRecAccessWMFRecStretchDIB::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfStretchDIB>();
	if (!pRec) return;
	m_propsCached->AddText(L"RasterOp",   WmfRasterOpText(pRec->RasterOp));
	m_propsCached->AddText(L"ColorUsage", WmfDIBColorsText(pRec->ColorUsage));
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	m_propsCached->AddValue(L"SrcWidth",  (int)pRec->SrcWidth);
	m_propsCached->AddValue(L"SrcHeight", (int)pRec->SrcHeight);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"DestWidth",  (int)pRec->DestWidth);
	m_propsCached->AddValue(L"DestHeight", (int)pRec->DestHeight);
}

void EMFRecAccessWMFRecSetDIBToDev::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFBitmapCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetDIBToDev>();
	if (!pRec) return;
	m_propsCached->AddText(L"ColorUsage", WmfDIBColorsText(pRec->ColorUsage));
	m_propsCached->AddValue(L"ScanCount", (int)pRec->ScanCount);
	m_propsCached->AddValue(L"StartScan", (int)pRec->StartScan);
	AddPoint(m_propsCached.get(), L"SrcOrigin",  pRec->XSrc,  pRec->YSrc);
	AddPoint(m_propsCached.get(), L"DestOrigin", pRec->XDest, pRec->YDest);
	m_propsCached->AddValue(L"Width",  (int)pRec->Width);
	m_propsCached->AddValue(L"Height", (int)pRec->Height);
}

// -----------------------------------------------------------------------
// Clipping records
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecOffsetClipRgn::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFClippingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfOffsetClipRgn>();
	if (pRec) AddPoint(m_propsCached.get(), L"Offset", pRec->X, pRec->Y);
}

void EMFRecAccessWMFRecExcludeClipRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFClippingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfExcludeClipRect>();
	if (pRec) AddRect(m_propsCached.get(), L"Rect", pRec->Left, pRec->Top, pRec->Right, pRec->Bottom);
}

void EMFRecAccessWMFRecIntersectClipRect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFClippingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfIntersectClipRect>();
	if (pRec) AddRect(m_propsCached.get(), L"Rect", pRec->Left, pRec->Top, pRec->Right, pRec->Bottom);
}

// -----------------------------------------------------------------------
// Object selection / deletion
// -----------------------------------------------------------------------

static void AddSelectObjectIndex(PropertyNode* pNode, EMFRecAccess* pSelf, EMFAccess* pEMF, uint16_t index)
{
	pNode->AddValue(L"ObjectIndex", (unsigned)index);
}

void EMFRecAccessWMFRecSelectObject::Preprocess(EMFAccess* pEMF)
{
	auto pRec = View(m_recInfo).As<OWmfSelectObject>();
	if (!pRec) return;
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ObjectIndex, false);
	if (pLinkedRec)
		AddLinkRecord(pLinkedRec, LinkedObjTypeObjectUnspecified, LinkedObjTypeObjManipulation);
}

void EMFRecAccessWMFRecSelectObject::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjManipulationCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSelectObject>();
	if (pRec) m_propsCached->AddValue(L"ObjectIndex", (unsigned)pRec->ObjectIndex);
}

void EMFRecAccessWMFRecSelectPalette::Preprocess(EMFAccess* pEMF)
{
	auto pRec = View(m_recInfo).As<OWmfSelectPalette>();
	if (!pRec) return;
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->PaletteIndex, false);
	if (pLinkedRec)
		AddLinkRecord(pLinkedRec, LinkedObjTypePalette, LinkedObjTypeObjManipulation);
}

void EMFRecAccessWMFRecSelectPalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjManipulationCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSelectPalette>();
	if (pRec) m_propsCached->AddValue(L"PaletteIndex", (unsigned)pRec->PaletteIndex);
}

void EMFRecAccessWMFRecSelectClipRegion::Preprocess(EMFAccess* pEMF)
{
	auto pRec = View(m_recInfo).As<OWmfSelectClipRegion>();
	if (!pRec) return;
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->RegionIndex, false);
	if (pLinkedRec)
		AddLinkRecord(pLinkedRec, LinkedObjTypeRegion, LinkedObjTypeObjManipulation);
}

void EMFRecAccessWMFRecSelectClipRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjManipulationCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSelectClipRegion>();
	if (pRec) m_propsCached->AddValue(L"RegionIndex", (unsigned)pRec->RegionIndex);
}

void EMFRecAccessWMFRecDeleteObject::Preprocess(EMFAccess* pEMF)
{
	auto pRec = View(m_recInfo).As<OWmfDeleteObject>();
	if (!pRec) return;
	auto pLinkedRec = pEMF->GetObjectCreationRecord(pRec->ObjectIndex, false);
	if (pLinkedRec)
		AddLinkRecord(pLinkedRec, LinkedObjTypeObjectUnspecified, LinkedObjTypeObjManipulation);
	pEMF->ClearWMFObject(pRec->ObjectIndex);
}

void EMFRecAccessWMFRecDeleteObject::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjManipulationCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfDeleteObject>();
	if (pRec) m_propsCached->AddValue(L"ObjectIndex", (unsigned)pRec->ObjectIndex);
}

// -----------------------------------------------------------------------
// Region helpers
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecFillRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfFillRegion>();
	if (pRec)
	{
		m_propsCached->AddValue(L"RegionIndex", (unsigned)pRec->RegionIndex);
		m_propsCached->AddValue(L"BrushIndex",  (unsigned)pRec->BrushIndex);
	}
}

void EMFRecAccessWMFRecFrameRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfFrameRegion>();
	if (pRec)
	{
		m_propsCached->AddValue(L"RegionIndex", (unsigned)pRec->RegionIndex);
		m_propsCached->AddValue(L"BrushIndex",  (unsigned)pRec->BrushIndex);
		m_propsCached->AddValue(L"Width",  (int)pRec->Width);
		m_propsCached->AddValue(L"Height", (int)pRec->Height);
	}
}

void EMFRecAccessWMFRecPaintRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfPaintRegion>();
	if (pRec) m_propsCached->AddValue(L"RegionIndex", (unsigned)pRec->RegionIndex);
}

void EMFRecAccessWMFRecInvertRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFDrawingCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfInvertRegion>();
	if (pRec) m_propsCached->AddValue(L"RegionIndex", (unsigned)pRec->RegionIndex);
}

void EMFRecAccessWMFRecAnimatePalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfAnimatePalette>();
	if (pRec)
	{
		m_propsCached->AddValue(L"Start",           (unsigned)pRec->Start);
		m_propsCached->AddValue(L"NumberOfEntries", (unsigned)pRec->NumberOfEntries);
	}
}

void EMFRecAccessWMFRecSetPalEntries::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFStateCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfSetPaletteEntries>();
	if (pRec)
	{
		m_propsCached->AddValue(L"Start",           (unsigned)pRec->Start);
		m_propsCached->AddValue(L"NumberOfEntries", (unsigned)pRec->NumberOfEntries);
	}
}

// -----------------------------------------------------------------------
// Object creation
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecCreatePenIndirect::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

bool EMFRecAccessWMFRecCreatePenIndirect::GetRecordColor(COLORREF& cr) const
{
	auto pRec = View(m_recInfo).As<OWmfLogPen>();
	if (!pRec) return false;
	cr = (COLORREF)pRec->ColorRef;
	return true;
}

void EMFRecAccessWMFRecCreatePenIndirect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfLogPen>();
	if (!pRec) return;
	auto pBranch = m_propsCached->AddBranch(L"LogPen");
	pBranch->AddText(L"PenStyle", WmfPenStyleText(pRec->PenStyle));
	auto pWidth = pBranch->AddBranch(L"Width");
	pWidth->AddValue(L"x", (int)pRec->WidthX);
	pWidth->AddValue(L"y", (int)pRec->WidthY);
	AddColor(pBranch.get(), L"ColorRef", pRec->ColorRef);
}

void EMFRecAccessWMFRecCreateBrushIndirect::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

bool EMFRecAccessWMFRecCreateBrushIndirect::GetRecordColor(COLORREF& cr) const
{
	auto pRec = View(m_recInfo).As<OWmfLogBrush>();
	if (!pRec) return false;
	cr = (COLORREF)pRec->ColorRef;
	return true;
}

void EMFRecAccessWMFRecCreateBrushIndirect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	auto pRec = View(m_recInfo).As<OWmfLogBrush>();
	if (!pRec) return;
	auto pBranch = m_propsCached->AddBranch(L"LogBrush");
	pBranch->AddText(L"BrushStyle", WmfBrushStyleText(pRec->BrushStyle));
	AddColor(pBranch.get(), L"ColorRef", pRec->ColorRef);
	pBranch->AddText(L"BrushHatch", WmfHatchStyleText(pRec->BrushHatch));
}

void EMFRecAccessWMFRecCreateFontIndirect::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

void EMFRecAccessWMFRecCreateFontIndirect::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	auto pRec = v.As<OWmfLogFont>(0, /* require at least the fixed part */ 18);
	if (!pRec) return;
	auto pBranch = m_propsCached->AddBranch(L"LogFont");
	pBranch->AddValue(L"Height",        (int)pRec->Height);
	pBranch->AddValue(L"Width",         (int)pRec->Width);
	pBranch->AddValue(L"Escapement",    (int)pRec->Escapement);
	pBranch->AddValue(L"Orientation",   (int)pRec->Orientation);
	pBranch->AddText (L"Weight",        WmfFontWeightText(pRec->Weight));
	pBranch->AddValue(L"Italic",        (unsigned)pRec->Italic);
	pBranch->AddValue(L"Underline",     (unsigned)pRec->Underline);
	pBranch->AddValue(L"StrikeOut",     (unsigned)pRec->StrikeOut);
	pBranch->AddText (L"CharSet",       WmfCharSetText(pRec->CharSet));
	pBranch->AddValue(L"OutPrecision",  (unsigned)pRec->OutPrecision);
	pBranch->AddValue(L"ClipPrecision", (unsigned)pRec->ClipPrecision);
	pBranch->AddText (L"Quality",       WmfFontQualityText(pRec->Quality));
	pBranch->AddValue(L"PitchAndFamily",(unsigned)pRec->PitchAndFamily, true);
	// FaceName: ANSI, up to 32 chars, may be NUL-terminated and the buffer
	// may be shorter than 32 bytes (variable LogFont length).
	const size_t maxName = 32;
	const size_t fixedLen = 18;
	size_t avail = (v.size > fixedLen) ? std::min<size_t>(maxName, v.size - fixedLen) : 0;
	const char* pName = (const char*)v.data + fixedLen;
	size_t nameLen = 0;
	while (nameLen < avail && pName[nameLen]) ++nameLen;
	pBranch->AddText(L"FaceName", CStringW(pName, (int)nameLen));
}

void EMFRecAccessWMFRecCreatePalette::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

void EMFRecAccessWMFRecCreatePalette::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	auto pHdr = View(m_recInfo).As<OWmfLogPaletteHeader>();
	if (!pHdr) return;
	m_propsCached->AddValue(L"Version",         pHdr->Version, true);
	m_propsCached->AddValue(L"NumberOfEntries", (unsigned)pHdr->NumberOfEntries);
}

void EMFRecAccessWMFRecCreatePatternBrush::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

void EMFRecAccessWMFRecCreatePatternBrush::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	m_propsCached->AddValue(L"PatternDataSize", m_recInfo.DataSize);
}

void EMFRecAccessWMFRecDIBCreatePatternBrush::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

void EMFRecAccessWMFRecDIBCreatePatternBrush::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	auto v = View(m_recInfo);
	uint16_t Style = 0, ColorUsage = 0;
	if (v.Read(0, Style) && v.Read(2, ColorUsage))
	{
		m_propsCached->AddText(L"Style",      WmfBrushStyleText(Style));
		m_propsCached->AddText(L"ColorUsage", WmfDIBColorsText(ColorUsage));
	}
	m_propsCached->AddValue(L"DataSize", m_recInfo.DataSize);
}

void EMFRecAccessWMFRecCreateRegion::Preprocess(EMFAccess* pEMF)
{
	pEMF->AddWMFObject(this);
}

void EMFRecAccessWMFRecCreateRegion::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFObjectCat::CacheProperties(ctxt);
	m_propsCached->AddValue(L"RegionDataSize", m_recInfo.DataSize);
}

// -----------------------------------------------------------------------
// Escape
// -----------------------------------------------------------------------

void EMFRecAccessWMFRecEscape::CacheProperties(const CachePropertiesContext& ctxt)
{
	EMFRecAccessWMFEscapeCat::CacheProperties(ctxt);
	auto pHdr = View(m_recInfo).As<OWmfEscapeHeader>();
	if (!pHdr) return;
	m_propsCached->AddValue(L"EscapeFunction", (unsigned)pHdr->EscapeFunction, true);
	m_propsCached->AddValue(L"ByteCount",      (unsigned)pHdr->ByteCount);
}
