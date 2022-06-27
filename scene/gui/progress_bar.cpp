/*************************************************************************/
/*  progress_bar.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "progress_bar.h"

#include "scene/resources/text_line.h"

Size2 ProgressBar::get_minimum_size() const {
	Ref<StyleBox> bg = get_theme_stylebox(SNAME("bg"));
	Ref<StyleBox> fg = get_theme_stylebox(SNAME("fg"));
	Ref<Font> font = get_theme_font(SNAME("font"));
	int font_size = get_theme_font_size(SNAME("font_size"));

	Size2 minimum_size = bg->get_minimum_size();
	minimum_size.height = MAX(minimum_size.height, fg->get_minimum_size().height);
	minimum_size.width = MAX(minimum_size.width, fg->get_minimum_size().width);
	if (percent_visible) {
		String txt = "100%";
		TextLine tl = TextLine(txt, font, font_size);
		minimum_size.height = MAX(minimum_size.height, bg->get_minimum_size().height + tl.get_size().y);
	} else { // this is needed, else the progressbar will collapse
		minimum_size.width = MAX(minimum_size.width, 1);
		minimum_size.height = MAX(minimum_size.height, 1);
	}
	return minimum_size;
}

void ProgressBar::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			Ref<StyleBox> bg = get_theme_stylebox(SNAME("bg"));
			Ref<StyleBox> fg = get_theme_stylebox(SNAME("fg"));
			Ref<Font> font = get_theme_font(SNAME("font"));
			int font_size = get_theme_font_size(SNAME("font_size"));
			Color font_color = get_theme_color(SNAME("font_color"));

			draw_style_box(bg, Rect2(Point2(), get_size()));

			float r = get_as_ratio();

			switch (mode) {
				case FILL_BEGIN_TO_END:
				case FILL_END_TO_BEGIN: {
					int mp = fg->get_minimum_size().width;
					int p = round(r * (get_size().width - mp));
					// We want FILL_BEGIN_TO_END to map to right to left when UI layout is RTL,
					// and left to right otherwise. And likewise for FILL_END_TO_BEGIN.
					bool right_to_left = is_layout_rtl() ? (mode == FILL_BEGIN_TO_END) : (mode == FILL_END_TO_BEGIN);
					if (p > 0) {
						if (right_to_left) {
							int p_remaining = round((1.0 - r) * (get_size().width - mp));
							draw_style_box(fg, Rect2(Point2(p_remaining, 0), Size2(p + fg->get_minimum_size().width, get_size().height)));
						} else {
							draw_style_box(fg, Rect2(Point2(0, 0), Size2(p + fg->get_minimum_size().width, get_size().height)));
						}
					}
				} break;
				case FILL_TOP_TO_BOTTOM:
				case FILL_BOTTOM_TO_TOP: {
					int mp = fg->get_minimum_size().height;
					int p = round(r * (get_size().height - mp));

					if (p > 0) {
						if (mode == FILL_TOP_TO_BOTTOM) {
							draw_style_box(fg, Rect2(Point2(0, 0), Size2(get_size().width, p + fg->get_minimum_size().height)));
						} else {
							int p_remaining = round((1.0 - r) * (get_size().height - mp));
							draw_style_box(fg, Rect2(Point2(0, p_remaining), Size2(get_size().width, p + fg->get_minimum_size().height)));
						}
					}
				} break;
				case FILL_MODE_MAX:
					break;
			}

			if (percent_visible) {
				String txt = TS->format_number(itos(int(get_as_ratio() * 100))) + TS->percent_sign();
				TextLine tl = TextLine(txt, font, font_size);
				Vector2 text_pos = (Point2(get_size().width - tl.get_size().x, get_size().height - tl.get_size().y) / 2).round();
				Color font_outline_color = get_theme_color(SNAME("font_outline_color"));
				int outline_size = get_theme_constant(SNAME("outline_size"));
				if (outline_size > 0 && font_outline_color.a > 0) {
					tl.draw_outline(get_canvas_item(), text_pos, outline_size, font_outline_color);
				}
				tl.draw(get_canvas_item(), text_pos, font_color);
			}
		} break;
	}
}

void ProgressBar::set_fill_mode(int p_fill) {
	ERR_FAIL_INDEX(p_fill, FILL_MODE_MAX);
	mode = (FillMode)p_fill;
	update();
}

int ProgressBar::get_fill_mode() {
	return mode;
}

void ProgressBar::set_percent_visible(bool p_visible) {
	if (percent_visible == p_visible) {
		return;
	}
	percent_visible = p_visible;
	update_minimum_size();
	update();
}

bool ProgressBar::is_percent_visible() const {
	return percent_visible;
}

void ProgressBar::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_fill_mode", "mode"), &ProgressBar::set_fill_mode);
	ClassDB::bind_method(D_METHOD("get_fill_mode"), &ProgressBar::get_fill_mode);
	ClassDB::bind_method(D_METHOD("set_percent_visible", "visible"), &ProgressBar::set_percent_visible);
	ClassDB::bind_method(D_METHOD("is_percent_visible"), &ProgressBar::is_percent_visible);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_mode", PROPERTY_HINT_ENUM, "Begin to End,End to Begin,Top to Bottom,Bottom to Top"), "set_fill_mode", "get_fill_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "percent_visible"), "set_percent_visible", "is_percent_visible");

	BIND_ENUM_CONSTANT(FILL_BEGIN_TO_END);
	BIND_ENUM_CONSTANT(FILL_END_TO_BEGIN);
	BIND_ENUM_CONSTANT(FILL_TOP_TO_BOTTOM);
	BIND_ENUM_CONSTANT(FILL_BOTTOM_TO_TOP);
}

ProgressBar::ProgressBar() {
	set_v_size_flags(0);
	set_step(0.01);
}
