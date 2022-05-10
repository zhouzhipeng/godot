/*************************************************************************/
/*  global_signal.cpp                                                             */
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

#include "global_signal.h"

#include "core/os/os.h"


GlobalSignal *GlobalSignal::singleton = nullptr;

GlobalSignal *GlobalSignal::get_singleton() {
	if (!singleton) {
		memnew(GlobalSignal);
	}
	return singleton;
}



void GlobalSignal::dump(){

	if(!debug_log){
		return;
	}

	print_line(vformat("GlobalSignal::dump >>  _listeners: %s",Variant(this->_listeners).to_json_string() ));


}

void GlobalSignal::add_listener(const StringName &signal_name, const Callable &method_call)
{
	print_line(vformat("GlobalSignal::add_listener signal_name: %s , method_call: %s", signal_name, method_call.get_method()));

	if(!this->_listeners.has(signal_name)){
		this->_listeners[signal_name] = Array();
	}

	Array a = this->_listeners[signal_name];
	a.push_back(method_call);


	this->dump();
}


void GlobalSignal::add_listener_using_callable(const  Callable &signal_name_callalble, const Callable &method_call)
{
	this->add_listener(signal_name_callalble.get_method(), method_call);
}




void GlobalSignal::remove_listener(const StringName &signal_name, const Callable &method_call)
{
	print_line(vformat("GlobalSignal::remove_listener signal_name: %s , method_call: %s", signal_name, method_call.get_method()));


	if(!this->_listeners.has(signal_name)){
		print_line(vformat("GlobalSignal::remove_listener ignored, cause signal_name: %s  not found.", signal_name));
		this->dump();
		return;
	}

	Array a = _listeners[signal_name];
	if(!a.has(method_call)){
		print_line(vformat("GlobalSignal::remove_listener ignored, cause method_call: %s  not found.", method_call.get_method()));
		this->dump();
		return;
	}

	a.erase(method_call);


	this->dump();

}

Error GlobalSignal::do_callp(const StringName& signal_name, const Variant **p_args, int p_argcount, Callable::CallError &r_error)
{


	StringName signal = signal_name;

	ERR_FAIL_COND_V_MSG(!this->_listeners.has(signal), ERR_INVALID_PARAMETER, vformat("GlobalSignal::emit error : signal_name %s has no listeners", signal));

	print_line(vformat("GlobalSignal::do_callp signal_name: %s , p_args: %s, p_argcount : %s", signal_name, **p_args, p_argcount));

	

	const Variant **args = nullptr;

	int argc = p_argcount;
	if (argc) {
		args = &p_args[0];
	}

	Array a = this->_listeners[signal];
	Array invalid_index;
	for(int i=0;i<a.size();i++){
		Callable c  = a[i];

		if (c.is_valid()){
			Variant return_val;
			Callable::CallError e;
			c.call(args, argc, return_val, e);
			print_line(vformat("GlobalSignal::emit signal: %s ok", signal));
		}else{

			invalid_index.push_back(i);
			ERR_PRINT( vformat("GlobalSignal::emit error : callable is not valid for signal : %s", signal));
		}
	
	}

	//remove invalid listeners.
	for(int i=0;i<invalid_index.size();i++){
		a.remove_at(i);
	}

	r_error.error = Callable::CallError::CALL_OK;

	return OK;
}



Error GlobalSignal::do_call(const Variant **p_args, int p_argcount, Callable::CallError &r_error)
{
	StringName signal = *p_args[0];

	ERR_FAIL_COND_V_MSG(!this->_listeners.has(signal), ERR_INVALID_PARAMETER, vformat("GlobalSignal::emit error : signal_name %s has no listeners", signal));
	

	const Variant **args = nullptr;

	int argc = p_argcount - 1;
	if (argc) {
		args = &p_args[1];
	}

	return this->do_callp(signal, args, argc, r_error);
}

void GlobalSignal::set_debug(bool debug)
{
	debug_log = debug;
}


void GlobalSignal::_bind_methods() {
	ClassDB::

	ClassDB::bind_method(D_METHOD("listen", "from_callable", "method_call"), &GlobalSignal::add_listener_using_callable);
	ClassDB::bind_method(D_METHOD("add_listener", "signal_name", "method_call"), &GlobalSignal::add_listener);
	ClassDB::bind_method(D_METHOD("remove_listener", "signal_name", "method_call"), &GlobalSignal::remove_listener);
	ClassDB::bind_method(D_METHOD("set_debug", "debug"), &GlobalSignal::set_debug);

	{
		MethodInfo mi;
		mi.name = "do_call";
		mi.arguments.push_back(PropertyInfo(Variant::STRING_NAME, "signal_name"));
		ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "do_call", &GlobalSignal::do_call, mi, varray(), false);
	}

}

GlobalSignal::GlobalSignal() {
	ERR_FAIL_COND_MSG(singleton, "Singleton for GlobalSignal already exists.");
	singleton = this;
}

GlobalSignal::~GlobalSignal() {
	singleton = nullptr;
	this->_listeners.clear();
}
