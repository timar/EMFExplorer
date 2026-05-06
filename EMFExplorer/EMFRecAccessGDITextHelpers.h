#ifndef EMF_REC_ACCESS_GDI_TEXT_HELPERS_H
#define EMF_REC_ACCESS_GDI_TEXT_HELPERS_H

// Shared enum/flag text helpers implemented in EMFRecAccessGDI.cpp.
// EMFRecAccessWMF.cpp reuses them for the WMF records that share semantics
// with the EMF originals (background/poly-fill/stretch modes, ROP codes,
// LOGFONT/LOGPEN/LOGBRUSH styles, etc.).

CStringW MapModeText(DWORD iMode);
CStringW BkModeText(DWORD iMode);
CStringW PolyFillModeText(DWORD iMode);
CStringW ROP2Text(DWORD iMode);
CStringW StretchBltModeText(DWORD iMode);
CStringW TextAlignText(DWORD iMode);
CStringW ArcDirectionText(DWORD iArcDirection);
CStringW RegionModeText(DWORD iMode);
CStringW ModifyWorldTransformModeText(DWORD iMode);
CStringW FloodFillModeText(DWORD iMode);
CStringW GradientFillModeText(DWORD ulMode);
CStringW BrushStyleText(DWORD lbStyle);
CStringW HatchStyleText(DWORD lbHatch);
CStringW PenStyleText(DWORD lopnStyle);
CStringW ExtPenStyleText(DWORD elpPenStyle);
CStringW FontWeightText(LONG lfWeight);
CStringW CharSetText(BYTE lfCharSet);
CStringW FontQualityText(BYTE lfQuality);
CStringW GraphicsModeText(UINT iGraphicsMode);
CStringW RasterOpText(DWORD dwRop);
CStringW BlendFunctionText(DWORD dwRop);
CStringW DIBColorsText(DWORD iUsage);
CStringW BiCompressionText(DWORD biCompression);
CStringW ExtTextOutOptionsText(UINT fuOptions);

#endif // EMF_REC_ACCESS_GDI_TEXT_HELPERS_H
