/*************************************************************************/
/*  global_call.cpp                                                             */
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

#include "global_call.h"

#include "core/os/os.h"


GlobalCall *GlobalCall::singleton = nullptr;

GlobalCall *GlobalCall::get_singleton() {
	if (!singleton) {
		memnew(GlobalCall);
	}
	return singleton;
}

bool GlobalCall::has_call(const StringName &call_name){
	return _callables.has(call_name);
}

Error GlobalCall::set_call(const StringName &call_name, const Callable &callable)
{
	if(this->_callables.has(call_name)){
		String msg = vformat("GlobalCall::set_call ignored, cause call_name: %s  has already existed.", call_name);
		print_line(msg);
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER, msg );
		
	}

	if(!callable.is_valid()){
		String msg =  vformat("GlobalCall::set_call ignored, cause callable: %s  is invalid.", callable.get_method());
		print_line(msg);
		ERR_FAIL_V_MSG(ERR_INVALID_PARAMETER,msg);
	}

	//get method signature
	List<MethodInfo> plist;
	callable.get_object()->get_method_list(&plist);
	Dictionary m;
	for(MethodInfo method : plist){
		if(method.name == callable.get_method()){
			m = method;
			break;
		}
	}

	print_line(vformat("GlobalCall::set_call call_name: %s , callable: %s, method : %s", call_name, callable, m));

	_callables[call_name] = callable;

	return OK;
}

Error GlobalCall::unset_call(const StringName &call_name)
{

	ERR_FAIL_COND_V_MSG(!this->_callables.has(call_name), ERR_INVALID_PARAMETER, vformat("GlobalCall::set_call ignored, cause call_name: %s  has already existed.", call_name));

	print_line(vformat("GlobalCall::unset_call call_name: %s", call_name));

	_callables.erase(call_name);

	return OK;
}


Callable GlobalCall::get_call(const StringName &call_name)
{

	ERR_FAIL_COND_V_MSG(!this->has_call(call_name), Callable(), vformat("GlobalCall::get_call failed, cause call_name : %s  is not existed. ", call_name));

	return _callables[call_name];	
}


void GlobalCall::_bind_methods() {
	
	ClassDB::bind_method(D_METHOD("has_call", "call_name"), &GlobalCall::has_call);
	ClassDB::bind_method(D_METHOD("set_call", "call_name", "callable"), &GlobalCall::set_call);
	ClassDB::bind_method(D_METHOD("unset_call", "call_name"), &GlobalCall::unset_call);
	ClassDB::bind_method(D_METHOD("get_call", "call_name"), &GlobalCall::get_call);
}

GlobalCall::GlobalCall() {
	ERR_FAIL_COND_MSG(singleton, "Singleton for GlobalCall already exists.");
	singleton = this;
}

GlobalCall::~GlobalCall() {
	singleton = nullptr;
	this->_callables.clear();
}
