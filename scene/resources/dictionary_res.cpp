/*************************************************************************/
/*  dictionary_res.cpp                                                  */
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

#include "dictionary_res.h"




void DictionaryRes::set_data(Dictionary data)
{
	this->data=data;
}

Dictionary DictionaryRes::get_data() const
{
	return data;
}

void DictionaryRes::_bind_methods() {
//	ClassDB::bind_method(D_METHOD("has_setting", "name"), &DictionaryRes::has_setting);
	
	ClassDB::bind_method(D_METHOD("set_data", "data"), &DictionaryRes::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &DictionaryRes::get_data);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE,"save your key&value here."), "set_data", "get_data");
	//ADD_PROPERTY(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT | PROPERTY_USAGE_DEFAULT), "set_constant_value", "get_constant_value");

}

DictionaryRes::DictionaryRes() {
	data["your key here"] = "your value(any valid Variant values)";
}

DictionaryRes::~DictionaryRes() {
	data.clear();
}
