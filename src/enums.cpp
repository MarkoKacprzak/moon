/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * enums.cpp: various enumerated types + enum -> str helpers
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include "enums.h"

// XXX enums are contained in these headers and should be moved to
// enums.h
#include "brush.h"
#include "clock.h"
#include "stylus.h"

using namespace Moonlight;

static GHashTable *enum_map = NULL;

#define MAP_ENUM_FULL(n,v) { (n), (v) }

// use this when the name is the same as the quoted value
#define MAP_NAME(n)          MAP_ENUM_FULL(#n, n)

// use these when the name is the same as the quoted value, but with the enum type prefixed
#define MAP_ENUM(t,n)        MAP_ENUM_FULL(#n, t##n)

#define END_MAPPING          { NULL, 0 }

typedef struct {
	const char *name;
	int value;
} enum_map_t;

static enum_map_t alignment_x_map [] = {
	MAP_ENUM (AlignmentX, Left),
	MAP_ENUM (AlignmentX, Center),
	MAP_ENUM (AlignmentX, Right),
	END_MAPPING
};

static enum_map_t alignment_y_map [] = {
	MAP_ENUM (AlignmentY, Top),
	MAP_ENUM (AlignmentY, Center),
	MAP_ENUM (AlignmentY, Bottom),
	END_MAPPING
};

static enum_map_t binding_mode_map [] = {
	MAP_ENUM (BindingMode, OneWay),
	MAP_ENUM (BindingMode, OneTime),
	MAP_ENUM (BindingMode, TwoWay),
	END_MAPPING
};

static enum_map_t brush_mapping_mode_map [] = {
	MAP_ENUM (BrushMappingMode, Absolute),
	MAP_ENUM (BrushMappingMode, RelativeToBoundingBox),
	END_MAPPING
};

static enum_map_t color_interpolation_mode_map [] = {
	MAP_ENUM (ColorInterpolationMode, ScRgbLinearInterpolation),
	MAP_ENUM (ColorInterpolationMode, SRgbLinearInterpolation),
	END_MAPPING
};

static enum_map_t cursors_map [] = {
	MAP_ENUM (CursorType, Default),
	MAP_ENUM (CursorType, Arrow),
	MAP_ENUM (CursorType, Hand),
	MAP_ENUM (CursorType, Wait),
	MAP_ENUM (CursorType, IBeam),
	MAP_ENUM (CursorType, Stylus),
	MAP_ENUM (CursorType, Eraser),
	MAP_ENUM (CursorType, SizeNS),
	MAP_ENUM (CursorType, SizeWE),
	MAP_ENUM (CursorType, SizeNESW),
	MAP_ENUM (CursorType, SizeNWSE),
	MAP_ENUM (CursorType, None),
	END_MAPPING
};

static enum_map_t elevated_permissions_map [] = {
	MAP_ENUM (ElevatedPermissions, None),
	MAP_ENUM (ElevatedPermissions, Required),
	END_MAPPING
};

static enum_map_t error_type_map [] = {
	MAP_NAME (NoError),
	MAP_NAME (UnknownError),
	MAP_NAME (InitializeError),
	MAP_NAME (ParserError),
	MAP_NAME (ObjectModelError),
	MAP_NAME (RuntimeError),
	MAP_NAME (DownloadError),
	MAP_NAME (MediaError),
	MAP_NAME (ImageError),
	END_MAPPING
};

static enum_map_t fill_behavior_map [] = {
	MAP_ENUM (FillBehavior, HoldEnd),
	MAP_ENUM (FillBehavior, Stop),
	END_MAPPING
};

static enum_map_t fill_rule_map [] = {
	MAP_ENUM (FillRule, EvenOdd),
	MAP_ENUM (FillRule, Nonzero),
	END_MAPPING
};

static enum_map_t font_stretches_map [] = {
	MAP_ENUM (FontStretches, UltraCondensed),
	MAP_ENUM (FontStretches, ExtraCondensed),
	MAP_ENUM (FontStretches, Condensed),
	MAP_ENUM (FontStretches, SemiCondensed),
	MAP_ENUM (FontStretches, Normal),
	MAP_ENUM (FontStretches, Medium),
	MAP_ENUM (FontStretches, SemiExpanded),
	MAP_ENUM (FontStretches, Expanded),
	MAP_ENUM (FontStretches, ExtraExpanded),
	MAP_ENUM (FontStretches, UltraExpanded),
	END_MAPPING
};

static enum_map_t install_state_map [] = {
	MAP_ENUM (InstallState, NotInstalled),
	MAP_ENUM (InstallState, Installing),
	MAP_ENUM (InstallState, Installed),
	MAP_ENUM (InstallState, InstallFailed),
	END_MAPPING
};

static enum_map_t font_styles_map [] = {
	MAP_ENUM (FontStyles, Normal),
	MAP_ENUM (FontStyles, Oblique),
	MAP_ENUM (FontStyles, Italic),
	END_MAPPING
};

static enum_map_t font_weights_map [] = {
	MAP_ENUM (FontWeights, Thin),
	MAP_ENUM (FontWeights, ExtraLight),
	MAP_ENUM_FULL ("UltraLight", FontWeightsExtraLight),  // deprecated as of July 2007 
	MAP_ENUM (FontWeights, Light),
	MAP_ENUM (FontWeights, Normal),
	MAP_ENUM_FULL ("Regular", FontWeightsNormal),         // deprecated as of July 2007 
	MAP_ENUM (FontWeights, Medium),
	MAP_ENUM (FontWeights, SemiBold),
	MAP_ENUM_FULL ("DemiBold", FontWeightsSemiBold),      // deprecated as of July 2007 
	MAP_ENUM (FontWeights, Bold),
	MAP_ENUM (FontWeights, ExtraBold),
	MAP_ENUM_FULL ("UltraBold", FontWeightsExtraBold),    // deprecated as of July 2007 
 	MAP_ENUM (FontWeights, Black),
	MAP_ENUM_FULL ("Heavy", FontWeightsBlack),            // deprecated as of July 2007 
	MAP_ENUM (FontWeights, ExtraBlack),
	MAP_ENUM_FULL ("UltraBlack", FontWeightsExtraBlack),  // deprecated as of July 2007 
	END_MAPPING
};

static enum_map_t line_stacking_strategy_map [] = {
	MAP_ENUM (LineStackingStrategy, MaxHeight),
	MAP_ENUM (LineStackingStrategy, BlockLineHeight),
	END_MAPPING
};

static enum_map_t horizontal_alignment_map [] = {
	MAP_ENUM (HorizontalAlignment, Left),
	MAP_ENUM (HorizontalAlignment, Center),
	MAP_ENUM (HorizontalAlignment, Right),
	MAP_ENUM (HorizontalAlignment, Stretch),
	END_MAPPING
};

static enum_map_t gradient_spread_method_map [] = {
	MAP_ENUM (GradientSpreadMethod, Pad),
	MAP_ENUM (GradientSpreadMethod, Reflect),
	MAP_ENUM (GradientSpreadMethod, Repeat),
	END_MAPPING
};

static enum_map_t keyboard_navigation_mode_map [] = {
	MAP_ENUM (KeyboardNavigationMode, Local),
	MAP_ENUM (KeyboardNavigationMode, Cycle),
	MAP_ENUM (KeyboardNavigationMode, Once),
	END_MAPPING
};

static enum_map_t media_element_state_map [] = {
	MAP_ENUM (MediaElementState, Closed),
	MAP_ENUM (MediaElementState, Opening),
	MAP_ENUM (MediaElementState, Buffering),
	MAP_ENUM (MediaElementState, Playing),
	MAP_ENUM (MediaElementState, Paused),
	MAP_ENUM (MediaElementState, Stopped),
	MAP_ENUM (MediaElementState, Individualizing),
	MAP_ENUM (MediaElementState, AcquiringLicense),
	END_MAPPING
};

static enum_map_t pen_line_cap_map [] = {
	MAP_ENUM (PenLineCap, Flat),
	MAP_ENUM (PenLineCap, Square),
	MAP_ENUM (PenLineCap, Round),
	MAP_ENUM (PenLineCap, Triangle),
	END_MAPPING
};

static enum_map_t pen_line_join_map [] = {
	MAP_ENUM (PenLineJoin, Miter),
	MAP_ENUM (PenLineJoin, Bevel),
	MAP_ENUM (PenLineJoin, Round),
	END_MAPPING
};

static enum_map_t stretch_map [] = {
	MAP_ENUM (Stretch, None),
	MAP_ENUM (Stretch, Fill),
	MAP_ENUM (Stretch, Uniform),
	MAP_ENUM (Stretch, UniformToFill),
	END_MAPPING
};

static enum_map_t stretch_direction_map [] = {
	MAP_ENUM (StretchDirection, UpOnly),
	MAP_ENUM (StretchDirection, DownOnly),
	MAP_ENUM (StretchDirection, Both),
	END_MAPPING
};

static enum_map_t style_simulations_map [] = {
	MAP_ENUM (StyleSimulations, None),
	MAP_ENUM_FULL ("BoldSimulation", StyleSimulationsBold),
	MAP_ENUM_FULL ("ItalicSimulation", StyleSimulationsItalic),
	MAP_ENUM_FULL ("BoldItalicSimulation", StyleSimulationsBoldItalic),
	END_MAPPING
};

static enum_map_t sweep_direction_map [] = {
	MAP_ENUM (SweepDirection, Counterclockwise),
	MAP_ENUM (SweepDirection, Clockwise),
	END_MAPPING
};

static enum_map_t tablet_device_type_map [] = {
	MAP_ENUM (TabletDeviceType, Mouse),
	MAP_ENUM (TabletDeviceType, Stylus),
	MAP_ENUM (TabletDeviceType, Touch),
	END_MAPPING
};

static enum_map_t text_alignment_map [] = {
	MAP_ENUM (TextAlignment, Center),
	MAP_ENUM (TextAlignment, Left),
	MAP_ENUM (TextAlignment, Right),
	MAP_ENUM (TextAlignment, Justify),
	END_MAPPING
};

static enum_map_t text_decorations_map [] = {
	MAP_ENUM (TextDecorations, None),
	MAP_ENUM (TextDecorations, Underline),
	END_MAPPING
};

static enum_map_t text_wrapping_map [] = {
	MAP_ENUM (TextWrapping, Wrap),
	MAP_ENUM (TextWrapping, NoWrap),
	MAP_ENUM (TextWrapping, WrapWithOverflow),
	END_MAPPING
};

static enum_map_t scrollbar_visibility_map [] = {
	MAP_ENUM (ScrollBarVisibility, Disabled),
	MAP_ENUM (ScrollBarVisibility, Auto),
	MAP_ENUM (ScrollBarVisibility, Hidden),
	MAP_ENUM (ScrollBarVisibility, Visible),
	END_MAPPING
};

static enum_map_t vertical_alignment_map [] = {
	MAP_ENUM (VerticalAlignment, Top),
	MAP_ENUM (VerticalAlignment, Center),
	MAP_ENUM (VerticalAlignment, Bottom),
	MAP_ENUM (VerticalAlignment, Stretch),
	END_MAPPING
};

static enum_map_t visibility_map [] = {
	MAP_ENUM (Visibility, Visible),
	MAP_ENUM (Visibility, Collapsed),
	END_MAPPING
};

static enum_map_t orientation_map [] = {
	MAP_ENUM (Orientation, Vertical),
	MAP_ENUM (Orientation, Horizontal),
	END_MAPPING
};

static enum_map_t cross_domain_access_map [] = {
	MAP_ENUM (CrossDomainAccess, NoAccess), 
	MAP_ENUM (CrossDomainAccess, ScriptableOnly),
	END_MAPPING 
};

static enum_map_t grid_unit_type_map [] = {
	MAP_ENUM (GridUnitType, Auto),
	MAP_ENUM (GridUnitType, Pixel),
	MAP_ENUM (GridUnitType, Star),
	END_MAPPING 
};

static enum_map_t easing_mode_map [] = {
	MAP_ENUM_FULL ("EaseIn", EasingModeIn), 
	MAP_ENUM_FULL ("EaseOut", EasingModeOut),
	MAP_ENUM_FULL ("EaseInOut", EasingModeInOut),
	END_MAPPING 
};

static enum_map_t log_source_map [] = {
	MAP_ENUM_FULL ("RequestLog", LogSourceRequestLog),
	MAP_ENUM_FULL ("Stop", LogSourceStop),
	MAP_ENUM_FULL ("Seek", LogSourceSeek),
	MAP_ENUM_FULL ("Pause", LogSourcePause),
	MAP_ENUM_FULL ("SourceChanged", LogSourceSourceChanged),
	MAP_ENUM_FULL ("EndOfStream", LogSourceEndOfStream),
	MAP_ENUM_FULL ("MediaElementShutdown", LogSourceMediaElementShutdown),
	MAP_ENUM_FULL ("RuntimeShutdown", LogSourceRuntimeShutdown),
	END_MAPPING
};

static enum_map_t flow_direction_map [] = {
	MAP_ENUM (FlowDirection, LeftToRight),
	MAP_ENUM (FlowDirection, RightToLeft),
	END_MAPPING
};

static enum_map_t logical_direction_map [] = {
	MAP_ENUM (LogicalDirection, Backward),
	MAP_ENUM (LogicalDirection, Forward),
	END_MAPPING
};

static enum_map_t text_hinting_mode_map [] = {
	MAP_ENUM (TextHintingMode, Fixed),
	MAP_ENUM (TextHintingMode, Animated),
	END_MAPPING
};

static enum_map_t bitmap_create_options_map [] = {
	MAP_ENUM (BitmapCreateOptions, None),
	MAP_ENUM (BitmapCreateOptions, DelayCreation),
	MAP_ENUM (BitmapCreateOptions, IgnoreImageCache),
	END_MAPPING
};

static enum_map_t moon_pixel_format_map [] = {
	MAP_ENUM_FULL ("None", MoonPixelFormatNone),
	MAP_ENUM_FULL ("RGB32", MoonPixelFormatRGB32),
	MAP_ENUM_FULL ("RGBA32", MoonPixelFormatRGBA32),
	MAP_ENUM_FULL ("YUV420P", MoonPixelFormatYUV420P),
	END_MAPPING
};

static enum_map_t input_method_state_map [] = {
	MAP_ENUM (InputMethodState, Off),
	MAP_ENUM (InputMethodState, On),
	MAP_ENUM (InputMethodState, DoNotCare),
	END_MAPPING
};

static enum_map_t ime_conversion_mode_values_map [] = {
	MAP_ENUM (ImeConversionModeValues, Native),
	MAP_ENUM (ImeConversionModeValues, Katakana),
	MAP_ENUM (ImeConversionModeValues, FullShape),
	MAP_ENUM (ImeConversionModeValues, Roman),
	MAP_ENUM (ImeConversionModeValues, CharCode),
	MAP_ENUM (ImeConversionModeValues, NoConversion),
	MAP_ENUM (ImeConversionModeValues, Eudc),
	MAP_ENUM (ImeConversionModeValues, Symbol),
	MAP_ENUM (ImeConversionModeValues, Fixed),
	MAP_ENUM (ImeConversionModeValues, Alphanumeric),
	MAP_ENUM (ImeConversionModeValues, DoNotCare),
	END_MAPPING
};

static enum_map_t window_state_map [] = {
	MAP_ENUM (WindowState, Normal),
	MAP_ENUM (WindowState, Minimized),
	MAP_ENUM (WindowState, Maximized),
	END_MAPPING
};

static enum_map_t window_startup_location_map [] = {
	MAP_ENUM (WindowStartupLocation, CenterScreen),
	MAP_ENUM (WindowStartupLocation, Manual),
	END_MAPPING
};

static enum_map_t window_style_map [] = {
	MAP_ENUM (WindowStyle, SingleBorderWindow),
	MAP_ENUM (WindowStyle, None),
	MAP_ENUM (WindowStyle, BorderlessRoundCornersWindow),
	END_MAPPING
};

static enum_map_t text_trimming_map [] = {
	MAP_ENUM (TextTrimming, None),
	MAP_ENUM (TextTrimming, WordEllipsis),
	END_MAPPING
};

static void
initialize_enums (void)
{
	enum_map = g_hash_table_new (g_str_hash, g_str_equal);
	
	g_hash_table_insert (enum_map, (char *) "AlignmentX", alignment_x_map);
	g_hash_table_insert (enum_map, (char *) "AlignmentY", alignment_y_map);

	g_hash_table_insert (enum_map, (char *) "BrushMappingMode", brush_mapping_mode_map);

	g_hash_table_insert (enum_map, (char *) "ColorInterpolationMode", color_interpolation_mode_map);
	g_hash_table_insert (enum_map, (char *) "CursorType", cursors_map);
	g_hash_table_insert (enum_map, (char *) "ErrorType", error_type_map);
	g_hash_table_insert (enum_map, (char *) "FillBehavior", fill_behavior_map);
	g_hash_table_insert (enum_map, (char *) "FillRule", fill_rule_map);
	g_hash_table_insert (enum_map, (char *) "FontStretch", font_stretches_map);
	g_hash_table_insert (enum_map, (char *) "FontStyle", font_styles_map);
	g_hash_table_insert (enum_map, (char *) "FontWeight", font_weights_map);

	g_hash_table_insert (enum_map, (char *) "GradientSpreadMethod", gradient_spread_method_map);

	g_hash_table_insert (enum_map, (char *) "PenLineCap", pen_line_cap_map);
	g_hash_table_insert (enum_map, (char *) "PenLineJoin", pen_line_join_map);
	
	g_hash_table_insert (enum_map, (char *) "Stretch", stretch_map);
	g_hash_table_insert (enum_map, (char *) "StyleSimulations", style_simulations_map);
	g_hash_table_insert (enum_map, (char *) "SweepDirection", sweep_direction_map);
	g_hash_table_insert (enum_map, (char *) "TabletDeviceType", tablet_device_type_map);
	g_hash_table_insert (enum_map, (char *) "TextDecorations", text_decorations_map);
	g_hash_table_insert (enum_map, (char *) "TextWrapping", text_wrapping_map);
	g_hash_table_insert (enum_map, (char *) "Visibility", visibility_map);

	g_hash_table_insert (enum_map, (char *) "BindingMode", binding_mode_map);
	g_hash_table_insert (enum_map, (char *) "CrossDomainAccess", cross_domain_access_map);
	g_hash_table_insert (enum_map, (char *) "ScrollBarVisibility", scrollbar_visibility_map);
	g_hash_table_insert (enum_map, (char *) "LineStackingStrategy", line_stacking_strategy_map);
	g_hash_table_insert (enum_map, (char *) "HorizontalAlignment", horizontal_alignment_map);
	g_hash_table_insert (enum_map, (char *) "VerticalAlignment", vertical_alignment_map);
	g_hash_table_insert (enum_map, (char *) "TextAlignment", text_alignment_map);
	g_hash_table_insert (enum_map, (char *) "Orientation", orientation_map);

	g_hash_table_insert (enum_map, (char *) "KeyboardNavigationMode", keyboard_navigation_mode_map);

	g_hash_table_insert (enum_map, (char *) "MediaElementState", media_element_state_map);
	g_hash_table_insert (enum_map, (char *) "GridUnitType", grid_unit_type_map);

	g_hash_table_insert (enum_map, (char *) "EasingMode", easing_mode_map);
	
	g_hash_table_insert (enum_map, (char *) "LogSource", log_source_map);
	g_hash_table_insert (enum_map, (char *) "FlowDirection", flow_direction_map);
	g_hash_table_insert (enum_map, (char *) "LogicalDirection", logical_direction_map);
	g_hash_table_insert (enum_map, (char *) "StretchDirection", stretch_direction_map);
	g_hash_table_insert (enum_map, (char *) "ImeConversionModeValues", ime_conversion_mode_values_map);
	g_hash_table_insert (enum_map, (char *) "InputMethodState", input_method_state_map);
	g_hash_table_insert (enum_map, (char *) "WindowState", window_state_map);
	g_hash_table_insert (enum_map, (char *) "WindowStartupLocation", window_startup_location_map);
	g_hash_table_insert (enum_map, (char *) "WindowStyle", window_style_map);

	g_hash_table_insert (enum_map, (char *) "TextHintingMode", text_hinting_mode_map);

	g_hash_table_insert (enum_map, (char *) "BitmapCreateOptions", bitmap_create_options_map);
	
	g_hash_table_insert (enum_map, (char *) "MoonPixelFormat", moon_pixel_format_map);
	g_hash_table_insert (enum_map, (char *) "InstallState", install_state_map);
	g_hash_table_insert (enum_map, (char *) "TextTrimming", text_trimming_map);
	g_hash_table_insert (enum_map, (char *) "ElevatedPermissions", elevated_permissions_map);
}

static int
enum_from_str (const enum_map_t *emu, const char *str)
{
	int i;
	for (i = 0; emu [i].name; i++) {
		if (!g_ascii_strcasecmp (emu [i].name, str))
			return emu [i].value;
	}

	return -1;
}

static const char*
str_from_enum (const enum_map_t *emu, int e)
{
	int i;
	for (i = 0; emu [i].name; i++) {
		if (emu [i].value == e)
			return emu [i].name;
	}

	return NULL;
}

//
// Converts a string representing an enum (ie. "FillEnd") to int.
// Returns -1 if no property or enum found.
//
int 
enums_str_to_int (const char *prop_name, const char *str)
{
	if (enum_map == NULL)
		initialize_enums ();

	enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, prop_name);
	if (! emu)
		return -1;

	return enum_from_str (emu, str);
}

//
// Converts an int enum to a string representation.
// Returns NULL if no match found.
//
const char *
enums_int_to_str (const char *prop_name, int e)
{
	if (enum_map == NULL)
		initialize_enums ();

	enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, prop_name);
	if (! emu)
		return NULL;

	return str_from_enum (emu, e);
}

bool
enums_is_enum_name (const char *enum_name)
{
	if (enum_map == NULL)
		initialize_enums ();

	return g_hash_table_lookup (enum_map, enum_name) != NULL;
}
