#ifndef OMAKE_STRING_APPENDER_H
#define OMAKE_STRING_APPENDER_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

//#include "core/string/ustring.h"
#include <cstddef>

class Node;

class OmakeStringAppender {
private:
	String buffer;
	size_t str_len;
	char32_t *ret_ptrw;
	int _buffer_size;
	int _inc_buffer_size;

public:
	OmakeStringAppender(const int buffer_size, const int inc_buffer_size);
	//OmakeStringAppender(String arg, const int buffer_size, const int inc_buffer_size);
	//OmakeStringAppender(const char* arg, const int buffer_size, const int inc_buffer_size);
	~OmakeStringAppender();

	void _grow_buffer(size_t len);

	void operator+=(const String &p_str);
	void operator+=(const char *p_cstr);

	void replace(const String &p_key, const String &p_with);
	void replace(const char* p_key, const char* p_with);
	void _shift_buffer_left(int pos, int count);
	void _shift_buffer_right(int pos, int count);
	String get_string();

	_FORCE_INLINE_ void clear() {
		buffer.resize(_buffer_size);
		ret_ptrw = buffer.ptrw();
		str_len = 0;
	}

	_FORCE_INLINE_ size_t length() const {
		return str_len;
	}

	static String merge_strings(const String &a, const String &b);
	static String merge_strings2(const char *p_chr, const String &b);
};

#endif // OMAKE_STRING_APPENDER_H
