/*************************************************************************/
/*  global_call.h                                                               */
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

#ifndef GLOBAL_CALL_H
#define GLOBAL_CALL_H

#include "core/object/class_db.h"

// This GlobalCall class conforms with as many of the ISO 8601 standards as possible.
// * As per ISO 8601:2004 4.3.2.1, all dates follow the Proleptic Gregorian
//   calendar. As such, the day before 1582-10-15 is 1582-10-14, not 1582-10-04.
//   See: https://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar
// * As per ISO 8601:2004 3.4.2 and 4.1.2.4, the year before 1 AD (aka 1 BC)
//   is number "0", with the year before that (2 BC) being "-1", etc.
// Conversion methods assume "the same timezone", and do not handle DST.
// Leap seconds are not handled, they must be done manually if desired.
// Suffixes such as "Z" are not handled, you need to strip them away manually.

class GlobalCall : public Object {
	GDCLASS(GlobalCall, Object);
	static void _bind_methods();
	static GlobalCall *singleton;

private:
	Dictionary _callables;
public:
	static GlobalCall *get_singleton();

	bool has_call(const StringName &call_name);

	Error set_call(const StringName &call_name, const Callable &callable);

	Error unset_call(const StringName &call_name);

	Callable get_call(const StringName &call_name);


	GlobalCall();
	virtual ~GlobalCall();
};

#endif // GLOBAL_CALL_H
