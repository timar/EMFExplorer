#ifndef EMF_STRUCT2PROPS_H
#define EMF_STRUCT2PROPS_H

#include "EmfPlusStruct.h"
#include "EmfStruct.h"

using namespace emfplus;

#define GS_VISITABLE_STRUCT		VISITABLE_STRUCT
#include "visit_struct.hpp"
#include "EmfStructVisit.h"
#include "GdiplusEnums.h"

static inline LPCWSTR EMFPlusHatchStyleText(OHatchStyle val)
{
	static const LPCWSTR aText[] = {
		L"StyleHorizontal",
		L"StyleVertical",
		L"StyleForwardDiagonal",
		L"StyleBackwardDiagonal",
		L"StyleLargeGrid",
		L"StyleDiagonalCross",
		L"Style05Percent",
		L"Style10Percent",
		L"Style20Percent",
		L"Style25Percent",
		L"Style30Percent",
		L"Style40Percent",
		L"Style50Percent",
		L"Style60Percent",
		L"Style70Percent",
		L"Style75Percent",
		L"Style80Percent",
		L"Style90Percent",
		L"StyleLightDownwardDiagonal",
		L"StyleLightUpwardDiagonal",
		L"StyleDarkDownwardDiagonal",
		L"StyleDarkUpwardDiagonal",
		L"StyleWideDownwardDiagonal",
		L"StyleWideUpwardDiagonal",
		L"StyleLightVertical",
		L"StyleLightHorizontal",
		L"StyleNarrowVertical",
		L"StyleNarrowHorizontal",
		L"StyleDarkVertical",
		L"StyleDarkHorizontal",
		L"StyleDashedDownwardDiagonal",
		L"StyleDashedUpwardDiagonal",
		L"StyleDashedHorizontal",
		L"StyleDashedVertical",
		L"StyleSmallConfetti",
		L"StyleLargeConfetti",
		L"StyleZigZag",
		L"StyleWave",
		L"StyleDiagonalBrick",
		L"StyleHorizontalBrick",
		L"StyleWeave",
		L"StylePlaid",
		L"StyleDivot",
		L"StyleDottedGrid",
		L"StyleDottedDiamond",
		L"StyleShingle",
		L"StyleTrellis",
		L"StyleSphere",
		L"StyleSmallGrid",
		L"StyleSmallCheckerBoard",
		L"StyleLargeCheckerBoard",
		L"StyleOutlinedDiamond",
		L"StyleSolidDiamond"
	};
	return aText[(int)val];
}

static inline LPCWSTR EMFPlusWrapModeText(OWrapMode val)
{
	static const LPCWSTR aText[] = {
		L"Tile",
		L"TileFlipX",
		L"TileFlipY",
		L"TileFlipXY",
		L"Clamp"
	};
	return aText[(int)val];
}

static inline LPCWSTR EMFPlusBrushTypeText(OBrushType val)
{
	static const LPCWSTR aText[] = {
		L"SolidColor",
		L"HatchFill",
		L"TextureFill",
		L"PathGradient",
		L"LinearGradient"
	};
	return aText[(int)val];
}

static inline LPCWSTR EMFPlusImageDataTypeText(OImageDataType type)
{
	static const LPCWSTR aText[] = {
		L"Unknown", L"Bitmap", L"Metafile"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusBitmapTypeText(OBitmapDataType type)
{
	static const LPCWSTR aText[] = {
		L"Pixel", L"Compressed"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusUnitTypeText(OUnitType type)
{
	static const LPCWSTR aText[] = {
		L"World", L"Display", L"Pixel", L"Point", L"Inch", L"Document", L"Millimeter"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusLineJoinTypeText(OLineJoinType type)
{
	static const LPCWSTR aText[] = {
		L"Miter", L"Bevel", L"Round", L"MiterClipped"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusLineStyleText(OLineStyle type)
{
	static const LPCWSTR aText[] = {
		L"Solid", L"Dash", L"Dot", L"DashDot", L"DashDotDot", L"Custom"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusDashedLineCapText(ODashedLineCap type)
{
	static const LPCWSTR aText[] = {
		L"Flat", L"Invalid", L"Round", L"Triangle"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusPenAlignmentText(OPenAlignment type)
{
	static const LPCWSTR aText[] = {
		L"Center", L"Inset", L"Left", L"Outset", L"Right"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusCustomLineCapDataTypeText(OCustomLineCapDataType type)
{
	static const LPCWSTR aText[] = {
		L"Default", L"AdjustableArrow"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusStringAlignmentText(OStringAlignment type)
{
	static const LPCWSTR aText[] = {
		L"Near", L"Center", L"Far"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusStringDigitSubstitutionText(OStringDigitSubstitution type)
{
	static const LPCWSTR aText[] = {
		L"User", L"None", L"National", L"Traditional"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusHotkeyPrefixText(OHotkeyPrefix type)
{
	static const LPCWSTR aText[] = {
		L"None", L"Show", L"Hide"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusStringTrimmingText(OStringTrimming type)
{
	static const LPCWSTR aText[] = {
		L"None", L"Character", L"Word", L"EllipsisCharacter", L"EllipsisWord", L"EllipsisPath"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusImageAttributesClampTypeText(OEmfPlusImageAttributes::ClampType type)
{
	static const LPCWSTR aText[] = {
		L"Rect", L"Bitmap"
	};
	return aText[(int)type];
}

static inline LPCWSTR EMFPlusRegionNodeDataTypeText(ORegionNodeDataType type)
{
	switch (type)
	{
	case ORegionNodeDataTypeAnd:
		return L"And";
	case ORegionNodeDataTypeOr:
		return L"Or";
	case ORegionNodeDataTypeXor:
		return L"Xor";
	case ORegionNodeDataTypeExclude:
		return L"Exclude";
	case ORegionNodeDataTypeComplement:
		return L"Complement";
	case ORegionNodeDataTypeRect:
		return L"Rect";
	case ORegionNodeDataTypePath:
		return L"Path";
	case ORegionNodeDataTypeEmpty:
		return L"Empty";
	case ORegionNodeDataTypeInfinite:
		return L"infinite";
	}
	return L"Unknown";
}

static inline LPCWSTR EMFPlusLinkedObjText(EMFRecAccess::LinkedObjType val)
{
	switch (val)
	{
	case EMFRecAccess::LinkedObjTypeInvalid:
		return L"Invalid";
	case EMFRecAccess::LinkedObjTypeBrush:
		return L"Brush";
	case EMFRecAccess::LinkedObjTypePen:
		return L"Pen";
	case EMFRecAccess::LinkedObjTypePath:
		return L"Path";
	case EMFRecAccess::LinkedObjTypeRegion:
		return L"Region";
	case EMFRecAccess::LinkedObjTypeImage:
		return L"Image";
	case EMFRecAccess::LinkedObjTypeFont:
		return L"Font";
	case EMFRecAccess::LinkedObjTypeStringFormat:
		return L"StringFormat";
	case EMFRecAccess::LinkedObjTypeImageAttributes:
		return L"ImageAttributes";
	case EMFRecAccess::LinkedObjTypeCustomLineCap:
		return L"CustomLineCap";
	case EMFRecAccess::LinkedObjTypeDrawingRecord:
		return L"DrawingRecord";
	case EMFRecAccess::LinkedObjTypeGraphicState:
		return L"GraphicState";
	default:
		break;
	}
	return L"Unknown";
}

static inline LPCWSTR EMFGDIStockObjText(DWORD val)
{
	static const LPCWSTR aText[] = {
		L"WHITE_BRUSH",
		L"LTGRAY_BRUSH",
		L"GRAY_BRUSH",
		L"DKGRAY_BRUSH",
		L"BLACK_BRUSH",
		L"NULL_BRUSH",
		L"WHITE_PEN",
		L"BLACK_PEN",
		L"NULL_PEN",
		L"Unknown",
		L"OEM_FIXED_FONT",
		L"ANSI_FIXED_FONT",
		L"ANSI_VAR_FONT",
		L"SYSTEM_FONT",
		L"DEVICE_DEFAULT_FONT",
		L"DEFAULT_PALETTE",
		L"SYSTEM_FIXED_FONT",
	};
	return aText[val];
}

static inline LPCWSTR EMFPlusMetafileTypeText(OMetafileDataType type)
{
	static const LPCWSTR aText[] = {
		L"Invalid", L"Wmf", L"WmfPlaceable", L"Emf", L"EmfPlusOnly", L"EmfPlusDual",
	};
	return aText[(int)type];
}

struct EmfStruct2Properties
{
	template <typename ValT>
	static inline void Build(const ValT& obj, PropertyNode* pNode)
	{
		visit_struct::for_each(obj, [&](const char* name, auto& value)
			{
				BuildField(name, value, pNode);
			});
	}

	template <typename ValT>
	static inline void BuildField(const char* name, const ValT& value, PropertyNode* pNode)
	{
		if constexpr (data_access::is_optional_wrapper_v<ValT>)
		{
			if (value.is_enabled())
				BuildField(name, value.get(), pNode);
		}
		else if constexpr (std::is_class_v<ValT>)
		{
			if constexpr (data_access::is_vector<ValT>::value || data_access::is_array_wrapper<ValT>::value)
			{
				pNode->sub.emplace_back(std::make_shared<PropertyNodeArray>(CStringW(name), value));
			}
			else
			{
				auto pBranchNode = pNode->AddBranch(CStringW(name));
				if (pBranchNode)
					Build(value, pBranchNode.get());
			}
		}
		else if constexpr (std::is_arithmetic_v<ValT>)
		{
			if constexpr (std::is_same_v<ValT, DWORD>)
			{
				if (strstr(name, "Color"))
				{
					pNode->sub.emplace_back(std::make_shared<PropertyNodeColor>(CStringW(name), 
						emfplus::OEmfPlusARGB::FromCOLORREF(value)));
					return;
				}
			}
			pNode->AddValue(CStringW(name), value);
		}
		else if constexpr (std::is_enum_v<ValT>)
		{
			CStringW str(name);
			pNode->AddText(str, GetEnumText(value));
		}
	}
	template <typename ValT>
	static inline void BuildField(const char* name, const std::unique_ptr<ValT>& value, PropertyNode* pNode)
	{
		if (value)
			BuildField(name, *value, pNode);
	}

	static inline void BuildField(const char* name, const OEmfPlusARGB& value, PropertyNode* pNode)
	{
		pNode->sub.emplace_back(std::make_shared<PropertyNodeColor>(CStringW(name), value));
	}

	static inline void BuildField(const char* name, const XFORM& value, PropertyNode* pNode)
	{
		pNode->sub.emplace_back(std::make_shared<PropertyNodePlusTransform>(CStringW(name), (emfplus::OEmfPlusTransformMatrix&)(value)));
	}

	static inline void BuildField(const char* name, const OEmfPlusPointDataArray& value, PropertyNode* pNode)
	{
		pNode->sub.emplace_back(std::make_shared<PropertyNodePlusPointDataArray>(CStringW(name), value));
	}

	static inline void BuildField(const char* name, const std::wstring& value, PropertyNode* pNode)
	{
		pNode->AddText(CStringW(name), value.c_str());
	}

	static inline LPCWSTR GetEnumText(OBrushType value)
	{
		return EMFPlusBrushTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OHatchStyle value)
	{
		return EMFPlusHatchStyleText(value);
	}
	static inline LPCWSTR GetEnumText(OWrapMode value)
	{
		return EMFPlusWrapModeText(value);
	}
	static inline LPCWSTR GetEnumText(OImageDataType value)
	{
		return EMFPlusImageDataTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OBitmapDataType value)
	{
		return EMFPlusBitmapTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OMetafileDataType value)
	{
		return EMFPlusMetafileTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OUnitType value)
	{
		return EMFPlusUnitTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OLineJoinType value)
	{
		return EMFPlusLineJoinTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OLineStyle value)
	{
		return EMFPlusLineStyleText(value);
	}
	static inline LPCWSTR GetEnumText(ODashedLineCap value)
	{
		return EMFPlusDashedLineCapText(value);
	}
	static inline LPCWSTR GetEnumText(OPenAlignment value)
	{
		return EMFPlusPenAlignmentText(value);
	}
	static inline LPCWSTR GetEnumText(OCustomLineCapDataType value)
	{
		return EMFPlusCustomLineCapDataTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OStringAlignment value)
	{
		return EMFPlusStringAlignmentText(value);
	}
	static inline LPCWSTR GetEnumText(OStringDigitSubstitution value)
	{
		return EMFPlusStringDigitSubstitutionText(value);
	}
	static inline LPCWSTR GetEnumText(OHotkeyPrefix value)
	{
		return EMFPlusHotkeyPrefixText(value);
	}
	static inline LPCWSTR GetEnumText(OStringTrimming value)
	{
		return EMFPlusStringTrimmingText(value);
	}
	static inline LPCWSTR GetEnumText(ORegionNodeDataType value)
	{
		return EMFPlusRegionNodeDataTypeText(value);
	}
	static inline LPCWSTR GetEnumText(OEmfPlusImageAttributes::ClampType value)
	{
		return EMFPlusImageAttributesClampTypeText(value);
	}
	static inline CStringW GetEnumText(OLineCapType value)
	{
		LPCWSTR label = nullptr;
		switch (value)
		{
		case OLineCapType::Flat:			label = L"Flat"; break;
		case OLineCapType::Square:			label = L"Square"; break;
		case OLineCapType::Round:			label = L"Round"; break;
		case OLineCapType::Triangle:		label = L"Triangle"; break;
		case OLineCapType::NoAnchor:		label = L"NoAnchor"; break;
		case OLineCapType::SquareAnchor:	label = L"SquareAnchor"; break;
		case OLineCapType::RoundAnchor:		label = L"RoundAnchor"; break;
		case OLineCapType::DiamondAnchor:	label = L"DiamondAnchor"; break;
		case OLineCapType::ArrowAnchor:		label = L"ArrowAnchor"; break;
		case OLineCapType::Custom:			label = L"Custom"; break;
		default: break;
		}
		if (label) return CStringW(label);
		return GetEnumTextAsHex((u32t)value);
	}
	static inline CStringW GetEnumText(OPixelFormat value)
	{
		LPCWSTR label = nullptr;
		switch (value)
		{
		case OPixelFormat::FormatUndefined:			label = L"Undefined"; break;
		case OPixelFormat::Format1bppIndexed:		label = L"1bppIndexed"; break;
		case OPixelFormat::Format4bppIndexed:		label = L"4bppIndexed"; break;
		case OPixelFormat::Format8bppIndexed:		label = L"8bppIndexed"; break;
		case OPixelFormat::Format16bppGrayScale:	label = L"16bppGrayScale"; break;
		case OPixelFormat::Format16bppRGB555:		label = L"16bppRGB555"; break;
		case OPixelFormat::Format16bppRGB565:		label = L"16bppRGB565"; break;
		case OPixelFormat::Format16bppARGB1555:		label = L"16bppARGB1555"; break;
		case OPixelFormat::Format24bppRGB:			label = L"24bppRGB"; break;
		case OPixelFormat::Format32bppRGB:			label = L"32bppRGB"; break;
		case OPixelFormat::Format32bppARGB:			label = L"32bppARGB"; break;
		case OPixelFormat::Format32bppPARGB:		label = L"32bppPARGB"; break;
		case OPixelFormat::Format48bppRGB:			label = L"48bppRGB"; break;
		case OPixelFormat::Format64bppARGB:			label = L"64bppARGB"; break;
		case OPixelFormat::Format64bppPARGB:		label = L"64bppPARGB"; break;
		default: break;
		}
		if (label) return CStringW(label);
		return GetEnumTextAsHex((u32t)value);
	}
	static inline CStringW GetEnumText(OBrushData value)
	{
		CStringW str;
		str.Format(L"0x%08X", (u32t)value);
		CStringW flags;
		u32t v = (u32t)value;
		if (v & (u32t)OBrushData::Path)				{ flags += L"Path"; }
		if (v & (u32t)OBrushData::Transform)			{ if (!flags.IsEmpty()) flags += L" | "; flags += L"Transform"; }
		if (v & (u32t)OBrushData::PresetColors)			{ if (!flags.IsEmpty()) flags += L" | "; flags += L"PresetColors"; }
		if (v & (u32t)OBrushData::BlendFactorsH)		{ if (!flags.IsEmpty()) flags += L" | "; flags += L"BlendFactorsH"; }
		if (v & (u32t)OBrushData::BlendFactorsV)		{ if (!flags.IsEmpty()) flags += L" | "; flags += L"BlendFactorsV"; }
		if (v & (u32t)OBrushData::FocusScales)			{ if (!flags.IsEmpty()) flags += L" | "; flags += L"FocusScales"; }
		if (v & (u32t)OBrushData::IsGammaCorrected)		{ if (!flags.IsEmpty()) flags += L" | "; flags += L"IsGammaCorrected"; }
		if (v & (u32t)OBrushData::DoNotTransform)		{ if (!flags.IsEmpty()) flags += L" | "; flags += L"DoNotTransform"; }
		if (!flags.IsEmpty()) { str += L"  "; str += flags; }
		return str;
	}
	static inline CStringW GetEnumTextAsHex(u32t value)
	{
		CStringW str;
		str.Format(L"%08X", value);
		return str;
	}
};

#endif // !EMF_STRUCT2PROPS_H
