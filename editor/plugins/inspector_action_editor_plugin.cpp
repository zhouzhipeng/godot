/*************************************************************************/
/*  inspector_action_editor_plugin.cpp                                   */
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

#include "inspector_action_editor_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_property_name_processor.h"

bool EditorInspectorActionPlugin::can_handle(Object *p_object) {
	Ref<Script> scr = Object::cast_to<Script>(p_object->get_script());

	if (scr == nullptr)
		return false;

	return true;
}

void EditorInspectorActionPlugin::update_action_icon() {
	action_button->set_icon(action_button->get_theme_icon(action_icon, SNAME("EditorIcons")));
}

void EditorInspectorActionPlugin::call_action(Object *p_object, String p_path) {
	Ref<Script> scr = Object::cast_to<Script>(p_object->get_script());
	if (!scr->is_tool()) {
		print_error("Could not call action, assigned script must be set as a tool script.");
		return;
	}

	Variant ret;
	p_object->get_script_instance()->get(StringName(p_path), ret);

	Callable *callable = VariantGetInternalPtr<Callable>::get_ptr(&ret);
	Callable::CallError ce;
	Variant v;

	if (callable->is_valid()) {
		callable->call(nullptr, 0, v, ce);
		ERR_FAIL_COND_MSG(ce.error != Callable::CallError::CALL_OK, "Could not call action. Error: " + Variant::get_callable_error_text(*callable, nullptr, 0, ce));
	} else {
		print_error("Could not call action. Callable invalid.");
	}
}

bool EditorInspectorActionPlugin::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const uint32_t p_usage, const bool p_wide) {
	if (p_type == Variant::Type::CALLABLE) {
		if (!p_hint_text.begins_with("action"))
			return false;
		action_button = memnew(Button(EditorPropertyNameProcessor::get_singleton()->process_name(p_path, EditorPropertyNameProcessor::STYLE_CAPITALIZED)));
		action_button->set_theme(EditorNode::get_singleton()->get_gui_base()->get_theme());

		String icon = p_hint_text.substr(6);
		if (!icon.is_empty()) {
			action_icon = StringName(icon);
			action_button->connect(SNAME("theme_changed"), callable_mp(this, &EditorInspectorActionPlugin::update_action_icon));
			action_button->connect(SNAME("tree_entered"), callable_mp(this, &EditorInspectorActionPlugin::update_action_icon));
		}
		add_custom_control(action_button);

		action_button->connect(SNAME("pressed"), callable_bind(callable_mp(this, &EditorInspectorActionPlugin::call_action), p_object, Variant(p_path)));
		return true;
	} else {
		return false;
	}
}

InspectorActionPlugin::InspectorActionPlugin() {
	Ref<EditorInspectorActionPlugin> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}
