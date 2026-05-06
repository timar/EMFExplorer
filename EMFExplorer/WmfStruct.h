#ifndef WMF_STRUCT_H
#define WMF_STRUCT_H

// On-disk parameter layouts for WMF records, per [MS-WMF].
// When Graphics::EnumerateMetafile() walks a WMF metafile, the callback
// receives `data` pointing at the parameters of each record (i.e. just past
// the 6-byte rdSize+rdFunction header), and `dataSize` as the size of those
// parameters in bytes. Every struct here describes that parameter portion.
//
// Per [MS-WMF] 2.3.5.x and 2.3.4.x records were originally recorded by
// pushing arguments on a stack, so multi-field params usually appear in
// reverse-of-call order (e.g. LineTo: y, x).

#include <cstdint>

namespace wmf
{

#pragma pack(push, 1)

	// MS-WMF [2.2.2.16] PointS: x first, then y.
	// Note that some record-bodies (e.g. META_LINETO, META_MOVETO) instead
	// store {Y, X} in file order — see those records' param structs.
	struct WmfPointS
	{
		int16_t x;
		int16_t y;
	};

	struct WmfRectS
	{
		// File order matches MS-WMF Rect16 (2.2.2.18): Left, Top, Right, Bottom.
		int16_t Left;
		int16_t Top;
		int16_t Right;
		int16_t Bottom;
	};

	// 2.3.5 State Records ----------------------------------------------------

	// Note: many of these records contain a trailing 2-byte Reserved field
	// in some MS-WMF implementations. We omit it from the structs because it's
	// "MUST be ignored" and not all writers include it; keeping the structs at
	// the minimum size lets WmfParamView::As<>() succeed for both layouts.
	struct OWmfSetBkColor			{ uint32_t ColorRef; };
	struct OWmfSetBkMode			{ uint16_t BkMode; };
	struct OWmfSetMapMode			{ uint16_t MapMode; };
	struct OWmfSetROP2				{ uint16_t DrawMode; };
	struct OWmfSetRelAbs			{ uint16_t Mode; };
	struct OWmfSetPolyFillMode		{ uint16_t Mode; };
	struct OWmfSetStretchBltMode	{ uint16_t Mode; };
	struct OWmfSetTextCharExtra		{ int16_t  CharExtra; };
	struct OWmfSetTextColor			{ uint32_t ColorRef; };
	struct OWmfSetTextJustification	{ int16_t BreakExtra; int16_t BreakCount; };
	struct OWmfSetWindowOrg			{ int16_t  Y; int16_t X; };
	struct OWmfSetWindowExt			{ int16_t  Y; int16_t X; };
	struct OWmfSetViewportOrg		{ int16_t  Y; int16_t X; };
	struct OWmfSetViewportExt		{ int16_t  Y; int16_t X; };
	struct OWmfOffsetWindowOrg		{ int16_t  Y; int16_t X; };
	struct OWmfScaleWindowExt		{ int16_t  yDenom; int16_t yNum; int16_t xDenom; int16_t xNum; };
	struct OWmfOffsetViewportOrg	{ int16_t  Y; int16_t X; };
	struct OWmfScaleViewportExt		{ int16_t  yDenom; int16_t yNum; int16_t xDenom; int16_t xNum; };
	struct OWmfSetTextAlign			{ uint16_t TextAlign; };
	struct OWmfSetMapperFlags		{ uint32_t MapperValues; };
	struct OWmfSetLayout			{ uint16_t Layout; };
	struct OWmfRestoreDC			{ int16_t  nSavedDC; };
	struct OWmfSaveDC				{ /* no params */ };
	struct OWmfRealizePalette		{ /* no params */ };
	struct OWmfResizePalette		{ uint16_t NumberOfEntries; };

	// 2.3.3 Drawing Records --------------------------------------------------

	struct OWmfLineTo				{ int16_t Y; int16_t X; };
	struct OWmfMoveTo				{ int16_t Y; int16_t X; };
	struct OWmfArc
	{
		int16_t YEndArc;   int16_t XEndArc;
		int16_t YStartArc; int16_t XStartArc;
		int16_t Bottom;    int16_t Right;
		int16_t Top;       int16_t Left;
	};
	struct OWmfChord
	{
		int16_t YRadial2; int16_t XRadial2;
		int16_t YRadial1; int16_t XRadial1;
		int16_t Bottom;   int16_t Right;
		int16_t Top;      int16_t Left;
	};
	struct OWmfPie
	{
		int16_t YRadial2; int16_t XRadial2;
		int16_t YRadial1; int16_t XRadial1;
		int16_t Bottom;   int16_t Right;
		int16_t Top;      int16_t Left;
	};
	struct OWmfEllipse		{ int16_t Bottom; int16_t Right; int16_t Top; int16_t Left; };
	struct OWmfRectangle	{ int16_t Bottom; int16_t Right; int16_t Top; int16_t Left; };
	struct OWmfRoundRect
	{
		int16_t Height; int16_t Width;
		int16_t Bottom; int16_t Right; int16_t Top; int16_t Left;
	};
	struct OWmfFloodFill	{ uint32_t ColorRef; int16_t Y; int16_t X; };
	struct OWmfExtFloodFill	{ uint16_t Mode; uint32_t ColorRef; int16_t Y; int16_t X; };
	struct OWmfSetPixel		{ uint32_t ColorRef; int16_t Y; int16_t X; };
	struct OWmfPatBlt
	{
		uint32_t RasterOp;
		int16_t Height; int16_t Width;
		int16_t YLeft;  int16_t XLeft;
	};
	struct OWmfPolygonHeader	// followed by NumberOfPoints PointS records
	{
		int16_t NumberOfPoints;
	};
	struct OWmfPolylineHeader	// followed by NumberOfPoints PointS records
	{
		int16_t NumberOfPoints;
	};
	struct OWmfPolyPolygonHeader	// followed by aPolyCounts[NumberOfPolygons], then points[]
	{
		uint16_t NumberOfPolygons;
		// uint16_t aPolyCounts[NumberOfPolygons];
		// WmfPointS aPoints[Sum(aPolyCounts)];
	};
	struct OWmfTextOutHeader	// followed by string bytes (StringLength), padding, YStart, XStart
	{
		int16_t StringLength;
		// char String[StringLength + (StringLength%2)];
		// int16_t YStart;
		// int16_t XStart;
	};
	struct OWmfExtTextOutHeader
	{
		int16_t Y;
		int16_t X;
		int16_t StringLength;
		uint16_t fwOpts;
		// optional Rect (4 SHORTs) when fwOpts & (ETO_OPAQUE|ETO_CLIPPED)
		// followed by char String[]
		// followed by int16_t Dx[StringLength] (optional)
	};

	// 2.3.6 Object Manipulation ---------------------------------------------

	struct OWmfDeleteObject			{ uint16_t ObjectIndex; };
	struct OWmfSelectObject			{ uint16_t ObjectIndex; };
	struct OWmfSelectPalette		{ uint16_t PaletteIndex; };
	struct OWmfSelectClipRegion		{ uint16_t RegionIndex; };
	struct OWmfFillRegion			{ uint16_t RegionIndex; uint16_t BrushIndex; };
	struct OWmfFrameRegion
	{
		uint16_t RegionIndex;
		uint16_t BrushIndex;
		int16_t Height;
		int16_t Width;
	};
	struct OWmfPaintRegion			{ uint16_t RegionIndex; };
	struct OWmfInvertRegion			{ uint16_t RegionIndex; };
	struct OWmfOffsetClipRgn		{ int16_t Y; int16_t X; };
	struct OWmfExcludeClipRect		{ int16_t Bottom; int16_t Right; int16_t Top; int16_t Left; };
	struct OWmfIntersectClipRect	{ int16_t Bottom; int16_t Right; int16_t Top; int16_t Left; };
	struct OWmfAnimatePalette
	{
		uint16_t Start;
		uint16_t NumberOfEntries;
		// PALETTEENTRY entries follow
	};
	struct OWmfSetPaletteEntries
	{
		uint16_t Start;
		uint16_t NumberOfEntries;
		// PALETTEENTRY entries follow
	};

	// 2.3.4 Bitmap Records ---------------------------------------------------

	struct OWmfBitBlt
	{
		uint32_t RasterOp;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  Reserved;
		int16_t  Height;
		int16_t  Width;
		int16_t  YDest;
		int16_t  XDest;
		// Bitmap16 follows
	};
	struct OWmfStretchBlt
	{
		uint32_t RasterOp;
		int16_t  SrcHeight;
		int16_t  SrcWidth;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  Reserved;
		int16_t  DestHeight;
		int16_t  DestWidth;
		int16_t  YDest;
		int16_t  XDest;
		// Bitmap16 follows
	};
	struct OWmfDIBBitBlt
	{
		uint32_t RasterOp;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  Reserved;
		int16_t  Height;
		int16_t  Width;
		int16_t  YDest;
		int16_t  XDest;
		// DIB follows (BITMAPINFOHEADER + pixel bits)
	};
	struct OWmfDIBStretchBlt
	{
		uint32_t RasterOp;
		int16_t  SrcHeight;
		int16_t  SrcWidth;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  Reserved;
		int16_t  DestHeight;
		int16_t  DestWidth;
		int16_t  YDest;
		int16_t  XDest;
		// DIB follows
	};
	struct OWmfStretchDIB
	{
		uint32_t RasterOp;
		uint16_t ColorUsage;
		int16_t  SrcHeight;
		int16_t  SrcWidth;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  DestHeight;
		int16_t  DestWidth;
		int16_t  YDest;
		int16_t  XDest;
		// DIB follows
	};
	struct OWmfSetDIBToDev
	{
		uint16_t ColorUsage;
		uint16_t ScanCount;
		uint16_t StartScan;
		int16_t  YSrc;
		int16_t  XSrc;
		int16_t  Height;
		int16_t  Width;
		int16_t  YDest;
		int16_t  XDest;
		// DIB follows
	};

	// 2.3.7 Object creation --------------------------------------------------

	// MS-WMF [2.2.1.6] LogPen / LogPenEx (16-bit on disk)
	struct OWmfLogPen
	{
		uint16_t PenStyle;
		int16_t  WidthX;
		int16_t  WidthY;	// unused, present for spec parity
		uint32_t ColorRef;
	};
	// MS-WMF [2.2.2.10] LogBrush
	struct OWmfLogBrush
	{
		uint16_t BrushStyle;
		uint32_t ColorRef;
		uint16_t BrushHatch;
	};
	// MS-WMF [2.2.2.13] LogFont (size 18 + face name up to 32)
	struct OWmfLogFont
	{
		int16_t  Height;
		int16_t  Width;
		int16_t  Escapement;
		int16_t  Orientation;
		int16_t  Weight;
		uint8_t  Italic;
		uint8_t  Underline;
		uint8_t  StrikeOut;
		uint8_t  CharSet;
		uint8_t  OutPrecision;
		uint8_t  ClipPrecision;
		uint8_t  Quality;
		uint8_t  PitchAndFamily;
		char     FaceName[32];
	};
	// MS-WMF [2.2.1.3] LogPalette header (followed by PALETTEENTRY entries)
	struct OWmfLogPaletteHeader
	{
		uint16_t Version;
		uint16_t NumberOfEntries;
	};

	struct OWmfEscapeHeader
	{
		uint16_t EscapeFunction;
		uint16_t ByteCount;
		// followed by data of ByteCount bytes
	};

#pragma pack(pop)

} // namespace wmf

#endif // WMF_STRUCT_H
