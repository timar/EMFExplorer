#ifndef EMF_REC_ACCESS_WMF_H
#define EMF_REC_ACCESS_WMF_H

#include "EMFRecAccess.h"

// Base class for all WMF records.
// Like EMF, GDI+ strips the record header before invoking the EnumerateMetafile
// callback, so m_recInfo.Data points at the parameters and m_recInfo.DataSize is
// the parameter byte count.
class EMFRecAccessWMFRec : public EMFRecAccess
{
public:
	bool IsGDIRecord() const override { return true; }
};

// Category bases (mirroring the GDI ones)
class EMFRecAccessWMFBitmapCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryBitmap; }
};
class EMFRecAccessWMFClippingCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryClipping; }
};
class EMFRecAccessWMFControlCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryControl; }
};
class EMFRecAccessWMFDrawingCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryDrawing; }
};
class EMFRecAccessWMFEscapeCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryEscape; }
};
class EMFRecAccessWMFObjectCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryObject; }
};
class EMFRecAccessWMFObjManipulationCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryObjManipulation; }
};
class EMFRecAccessWMFStateCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryState; }
};
class EMFRecAccessWMFReservedCat : public EMFRecAccessWMFRec
{
public:
	RecCategory GetRecordCategory() const override { return RecCategoryReserved; }
};

// ---- State records --------------------------------------------------------

#define WMF_DECLARE_RECORD_BASE(_Class, _Name, _Type, _Base)              \
	class _Class : public _Base                                            \
	{                                                                     \
	public:                                                               \
		LPCWSTR GetRecordName() const override { return _Name; }          \
		emfplus::OEmfPlusRecordType GetRecordType() const override        \
		{ return emfplus::_Type; }                                        \
	private:                                                              \
		void CacheProperties(const CachePropertiesContext& ctxt) override; \
	}

#define WMF_DECLARE_RECORD_NOPROP(_Class, _Name, _Type, _Base)            \
	class _Class : public _Base                                            \
	{                                                                     \
	public:                                                               \
		LPCWSTR GetRecordName() const override { return _Name; }          \
		emfplus::OEmfPlusRecordType GetRecordType() const override        \
		{ return emfplus::_Type; }                                        \
	}

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetBkColor,            L"META_SETBKCOLOR",            WmfRecordTypeSetBkColor,            EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetBkMode,             L"META_SETBKMODE",             WmfRecordTypeSetBkMode,             EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetMapMode,            L"META_SETMAPMODE",            WmfRecordTypeSetMapMode,            EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetROP2,               L"META_SETROP2",               WmfRecordTypeSetROP2,               EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetRelAbs,             L"META_SETRELABS",             WmfRecordTypeSetRelAbs,             EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetPolyFillMode,       L"META_SETPOLYFILLMODE",       WmfRecordTypeSetPolyFillMode,       EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetStretchBltMode,     L"META_SETSTRETCHBLTMODE",     WmfRecordTypeSetStretchBltMode,     EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetTextCharExtra,      L"META_SETTEXTCHAREXTRA",      WmfRecordTypeSetTextCharExtra,      EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetTextColor,          L"META_SETTEXTCOLOR",          WmfRecordTypeSetTextColor,          EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetTextJustification,  L"META_SETTEXTJUSTIFICATION",  WmfRecordTypeSetTextJustification,  EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetWindowOrg,          L"META_SETWINDOWORG",          WmfRecordTypeSetWindowOrg,          EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetWindowExt,          L"META_SETWINDOWEXT",          WmfRecordTypeSetWindowExt,          EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetViewportOrg,        L"META_SETVIEWPORTORG",        WmfRecordTypeSetViewportOrg,        EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetViewportExt,        L"META_SETVIEWPORTEXT",        WmfRecordTypeSetViewportExt,        EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecOffsetWindowOrg,       L"META_OFFSETWINDOWORG",       WmfRecordTypeOffsetWindowOrg,       EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecScaleWindowExt,        L"META_SCALEWINDOWEXT",        WmfRecordTypeScaleWindowExt,        EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecOffsetViewportOrg,     L"META_OFFSETVIEWPORTORG",     WmfRecordTypeOffsetViewportOrg,     EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecScaleViewportExt,      L"META_SCALEVIEWPORTEXT",      WmfRecordTypeScaleViewportExt,      EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetTextAlign,          L"META_SETTEXTALIGN",          WmfRecordTypeSetTextAlign,          EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetMapperFlags,        L"META_SETMAPPERFLAGS",        WmfRecordTypeSetMapperFlags,        EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetLayout,             L"META_SETLAYOUT",             WmfRecordTypeSetLayout,             EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecResizePalette,         L"META_RESIZEPALETTE",         WmfRecordTypeResizePalette,         EMFRecAccessWMFStateCat);

// SAVEDC needs to push state for RestoreDC linkage
class EMFRecAccessWMFRecSaveDC : public EMFRecAccessWMFStateCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_SAVEDC"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeSaveDC; }
};

class EMFRecAccessWMFRecRestoreDC : public EMFRecAccessWMFStateCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_RESTOREDC"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeRestoreDC; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecRealizePalette,      L"META_REALIZEPALETTE",        WmfRecordTypeRealizePalette,        EMFRecAccessWMFStateCat);

// ---- Drawing records ------------------------------------------------------

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecLineTo,                L"META_LINETO",                WmfRecordTypeLineTo,                EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecMoveTo,                L"META_MOVETO",                WmfRecordTypeMoveTo,                EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecArc,                   L"META_ARC",                   WmfRecordTypeArc,                   EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecChord,                 L"META_CHORD",                 WmfRecordTypeChord,                 EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPie,                   L"META_PIE",                   WmfRecordTypePie,                   EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecEllipse,               L"META_ELLIPSE",               WmfRecordTypeEllipse,               EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecRectangle,             L"META_RECTANGLE",             WmfRecordTypeRectangle,             EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecRoundRect,             L"META_ROUNDRECT",             WmfRecordTypeRoundRect,             EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecFloodFill,             L"META_FLOODFILL",             WmfRecordTypeFloodFill,             EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecExtFloodFill,          L"META_EXTFLOODFILL",          WmfRecordTypeExtFloodFill,          EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetPixel,              L"META_SETPIXEL",              WmfRecordTypeSetPixel,              EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPolygon,               L"META_POLYGON",               WmfRecordTypePolygon,               EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPolyline,              L"META_POLYLINE",              WmfRecordTypePolyline,              EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPolyPolygon,           L"META_POLYPOLYGON",           WmfRecordTypePolyPolygon,           EMFRecAccessWMFDrawingCat);
class EMFRecAccessWMFRecTextOut : public EMFRecAccessWMFDrawingCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_TEXTOUT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeTextOut; }
	LPCWSTR GetRecordText() const override;
private:
	void CacheProperties(const CachePropertiesContext& ctxt) override;
	mutable CStringW m_strText;
};

class EMFRecAccessWMFRecExtTextOut : public EMFRecAccessWMFDrawingCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_EXTTEXTOUT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeExtTextOut; }
	LPCWSTR GetRecordText() const override;
private:
	void CacheProperties(const CachePropertiesContext& ctxt) override;
	mutable CStringW m_strText;
};
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPatBlt,                L"META_PATBLT",                WmfRecordTypePatBlt,                EMFRecAccessWMFBitmapCat);

// ---- Bitmap records -------------------------------------------------------

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecBitBlt,                L"META_BITBLT",                WmfRecordTypeBitBlt,                EMFRecAccessWMFBitmapCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecStretchBlt,            L"META_STRETCHBLT",            WmfRecordTypeStretchBlt,            EMFRecAccessWMFBitmapCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecDIBBitBlt,             L"META_DIBBITBLT",             WmfRecordTypeDIBBitBlt,             EMFRecAccessWMFBitmapCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecDIBStretchBlt,         L"META_DIBSTRETCHBLT",         WmfRecordTypeDIBStretchBlt,         EMFRecAccessWMFBitmapCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecStretchDIB,            L"META_STRETCHDIB",            WmfRecordTypeStretchDIB,            EMFRecAccessWMFBitmapCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetDIBToDev,           L"META_SETDIBTODEV",           WmfRecordTypeSetDIBToDev,           EMFRecAccessWMFBitmapCat);

// ---- Clipping -------------------------------------------------------------

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecOffsetClipRgn,         L"META_OFFSETCLIPRGN",         WmfRecordTypeOffsetClipRgn,         EMFRecAccessWMFClippingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecExcludeClipRect,       L"META_EXCLUDECLIPRECT",       WmfRecordTypeExcludeClipRect,       EMFRecAccessWMFClippingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecIntersectClipRect,     L"META_INTERSECTCLIPRECT",     WmfRecordTypeIntersectClipRect,     EMFRecAccessWMFClippingCat);

// ---- Object manipulation --------------------------------------------------

class EMFRecAccessWMFRecSelectObject : public EMFRecAccessWMFObjManipulationCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_SELECTOBJECT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeSelectObject; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecSelectPalette : public EMFRecAccessWMFObjManipulationCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_SELECTPALETTE"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeSelectPalette; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecSelectClipRegion : public EMFRecAccessWMFObjManipulationCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_SELECTCLIPREGION"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeSelectClipRegion; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecDeleteObject : public EMFRecAccessWMFObjManipulationCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_DELETEOBJECT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeDeleteObject; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecFillRegion,            L"META_FILLREGION",            WmfRecordTypeFillRegion,            EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecFrameRegion,           L"META_FRAMEREGION",           WmfRecordTypeFrameRegion,           EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecPaintRegion,           L"META_PAINTREGION",           WmfRecordTypePaintRegion,           EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecInvertRegion,          L"META_INVERTREGION",          WmfRecordTypeInvertRegion,          EMFRecAccessWMFDrawingCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecAnimatePalette,        L"META_ANIMATEPALETTE",        WmfRecordTypeAnimatePalette,        EMFRecAccessWMFStateCat);
WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecSetPalEntries,         L"META_SETPALENTRIES",         WmfRecordTypeSetPalEntries,         EMFRecAccessWMFStateCat);

// ---- Object creation ------------------------------------------------------

class EMFRecAccessWMFRecCreatePenIndirect : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEPENINDIRECT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreatePenIndirect; }
	bool GetRecordColor(COLORREF& cr) const override;
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecCreateBrushIndirect : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEBRUSHINDIRECT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreateBrushIndirect; }
	bool GetRecordColor(COLORREF& cr) const override;
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecCreateFontIndirect : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEFONTINDIRECT"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreateFontIndirect; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecCreatePalette : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEPALETTE"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreatePalette; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecCreatePatternBrush : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEPATTERNBRUSH"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreatePatternBrush; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecDIBCreatePatternBrush : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_DIBCREATEPATTERNBRUSH"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeDIBCreatePatternBrush; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

class EMFRecAccessWMFRecCreateRegion : public EMFRecAccessWMFObjectCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_CREATEREGION"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeCreateRegion; }
private:
	void Preprocess(EMFAccess* pEMF) override;
	void CacheProperties(const CachePropertiesContext& ctxt) override;
};

// ---- Escape ---------------------------------------------------------------

WMF_DECLARE_RECORD_BASE(EMFRecAccessWMFRecEscape,                L"META_ESCAPE",                WmfRecordTypeEscape,                EMFRecAccessWMFEscapeCat);

// ---- Reserved-but-occasionally-seen ---------------------------------------

WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecDrawText,            L"META_DRAWTEXT",              WmfRecordTypeDrawText,              EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecResetDC,             L"META_RESETDC",               WmfRecordTypeResetDC,               EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecStartDoc,            L"META_STARTDOC",              WmfRecordTypeStartDoc,              EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecStartPage,           L"META_STARTPAGE",             WmfRecordTypeStartPage,             EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecEndPage,             L"META_ENDPAGE",               WmfRecordTypeEndPage,               EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecAbortDoc,            L"META_ABORTDOC",              WmfRecordTypeAbortDoc,              EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecEndDoc,              L"META_ENDDOC",                WmfRecordTypeEndDoc,                EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecCreateBrush,         L"META_CREATEBRUSH",           WmfRecordTypeCreateBrush,           EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecCreateBitmapIndirect,L"META_CREATEBITMAPINDIRECT",  WmfRecordTypeCreateBitmapIndirect,  EMFRecAccessWMFReservedCat);
WMF_DECLARE_RECORD_NOPROP(EMFRecAccessWMFRecCreateBitmap,        L"META_CREATEBITMAP",          WmfRecordTypeCreateBitmap,          EMFRecAccessWMFReservedCat);

// META_EOF (function code 0x0000): a sentinel record marking end of metafile.
class EMFRecAccessWMFRecEOF : public EMFRecAccessWMFControlCat
{
public:
	LPCWSTR GetRecordName() const override { return L"META_EOF"; }
	emfplus::OEmfPlusRecordType GetRecordType() const override { return emfplus::WmfRecordTypeEOF; }
};

#undef WMF_DECLARE_RECORD_BASE
#undef WMF_DECLARE_RECORD_NOPROP

#endif // EMF_REC_ACCESS_WMF_H
