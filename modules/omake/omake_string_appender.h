#ifndef OMAKE_STRING_APPENDER_H
#define OMAKE_STRING_APPENDER_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "core/string/ustring.h"
#include <cstddef>

class OmakeStringAppender {
private:
	static constexpr size_t MAX_SEGMENTS = 1024;

	struct Segment {
		const char32_t *data;
		size_t len;
		bool is_char;
	};

	Segment segments[MAX_SEGMENTS];
	size_t segment_count;
	size_t total_length;

public:
	OmakeStringAppender();
	~OmakeStringAppender() = default;

	void operator+=(const String &p_str);
	void operator+=(const char *p_cstr);

	String get_string();

	_FORCE_INLINE_ void clear() {
		segment_count = 0;
		total_length = 0;
	}

	_FORCE_INLINE_ size_t length() const {
		return total_length;
	}

	static String merge_strings(const String &a, const String &b);
	static String merge_strings2(const char *p_chr, const String &b);

private:
	void consolidate(char32_t *dest) const;
};

#endif // OMAKE_STRING_APPENDER_H
