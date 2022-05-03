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



	print_line(vformat("GlobalSignal::dump >> _emitters: %s , _listeners: %s",Variant(this->_emitters).to_json_string(),  Variant(this->_listeners).to_json_string() ));

	Array keys = this->_emitters.keys();
	for(int i=0; i<keys.size(); i++){
		StringName n = keys[i];
		Array emitters = this->_emitters[n];
		for(int j=0; j<emitters.size(); j++){
			Object* e = emitters[j];
			List<Connection> conns;
			e-> get_signal_connection_list(n, &conns);

			Array arr;
			for (Connection c : conns){
				arr.push_back(c);	
			}

			print_line(vformat("GlobalSignal::dump signal connections >>  signal : %s , connections : %s", n, arr));

		}
	}


}


void GlobalSignal::_connect_emitter_to_listeners(const StringName &signal_name, Object* emitter)
{
	Array listeners = this->_listeners[signal_name];
	for(int i=0;i<listeners.size();i++){
		Callable c = listeners[i];
		emitter->connect(signal_name, c);
	}
}

void GlobalSignal::_connect_listener_to_emitters(const StringName &signal_name, const Callable &method_call)
{
	Array emitters = this->_emitters[signal_name];
	for(int i=0;i<emitters.size();i++){
		Object* c = emitters[i];
		c->connect(signal_name, method_call);
	}
}

void GlobalSignal::add_emitter(const StringName &signal_name, Object* emitter)
{
	print_line(vformat("GlobalSignal::add_emitter signal_name: %s , emitter: %s", signal_name, emitter->to_string()));

	if(!this->_emitters.has(signal_name)){
		_emitters[signal_name] = Array();
	}

	Array a  = this->_emitters[signal_name];
	a.push_back(emitter);

	if(this->_listeners.has(signal_name)){
		this->_connect_emitter_to_listeners(signal_name, emitter);
	}

	this->dump();
}

void GlobalSignal::add_listener(const StringName &signal_name, const Callable &method_call)
{
	print_line(vformat("GlobalSignal::add_listener signal_name: %s , method_call: %s", signal_name, method_call.get_method()));

	if(!this->_listeners.has(signal_name)){
		this->_listeners[signal_name] = Array();
	}

	Array a = this->_listeners[signal_name];
	a.push_back(method_call);

	if(this->_emitters.has(signal_name)){
		this->_connect_listener_to_emitters(signal_name, method_call);
	}

	this->dump();
}


void GlobalSignal::remove_emitter(const StringName &signal_name, Object* emitter)
{
	print_line(vformat("GlobalSignal::remove_emitter signal_name: %s , emitter: %s", signal_name, emitter->to_string()));
	if(!this->_emitters.has(signal_name)){
		print_line(vformat("GlobalSignal::remove_emitter ignored, cause signal_name: %s  not found.", signal_name));
		this->dump();
		return;
	}

	Array a = this->_emitters[signal_name];
	if(!a.has(emitter)){
		print_line(vformat("GlobalSignal::remove_emitter ignored, cause emitter: %s  not found.", emitter->to_string()));
		this->dump();
		return;
	}

	a.erase(emitter);

	
	if(_listeners.has(signal_name)){
		//Array index;
		Array listeners = _listeners[signal_name];
		for(int i=0;i<listeners.size();i++){
			Callable c = listeners[i];
			if(emitter->is_connected(signal_name, c)){
				emitter->disconnect(signal_name, c);
				//index.push_back(i);
			}
		}

	/*	for(int j=0;j<index.size();j++){
			listeners.remove_at(index[j]);
		}*/
	}

	this->dump();
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

	if(this->_emitters.has(signal_name)){
		//Array index;
		Array emitters =  _emitters[signal_name];
		for(int i=0;i<emitters.size();i++){
			Object* c = emitters[i];
			if(c->is_connected(signal_name, method_call)){
				c->disconnect(signal_name, method_call);
				//index.push_back(i);
			}

		}

		//for(int j=0;j<index.size();j++){
		//	emitters.remove_at(index[j]);
		//}
	}


	this->dump();

}


void GlobalSignal::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_listener", "signal_name", "method_call"), &GlobalSignal::add_listener);
	ClassDB::bind_method(D_METHOD("add_emitter", "signal_name", "emitter"), &GlobalSignal::add_emitter);
	ClassDB::bind_method(D_METHOD("remove_emitter", "signal_name", "emitter"), &GlobalSignal::remove_emitter);
	ClassDB::bind_method(D_METHOD("remove_listener", "signal_name", "method_call"), &GlobalSignal::remove_listener);
}

GlobalSignal::GlobalSignal() {
	ERR_FAIL_COND_MSG(singleton, "Singleton for GlobalSignal already exists.");
	singleton = this;
}

GlobalSignal::~GlobalSignal() {
	singleton = nullptr;
	this->_emitters.clear();
	this->_listeners.clear();
}
