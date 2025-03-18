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

public:
	OmakeStringAppender();
	~OmakeStringAppender() = default;

	void operator+=(const String &p_str);
	void operator+=(const char *p_cstr);

	String get_string();

	_FORCE_INLINE_ void clear() {
		buffer.resize(1024);
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
