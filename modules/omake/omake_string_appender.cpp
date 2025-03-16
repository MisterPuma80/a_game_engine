
#include "omake_string_appender.h"

OmakeStringAppender::OmakeStringAppender() :
		segment_count(0),
		total_length(0) {
}

void OmakeStringAppender::operator+=(const String &p_str) {
	int len = p_str.length();
	if (len == 0) {
		return;
	}

	if (segment_count < MAX_SEGMENTS) {
		Segment &seg = segments[segment_count++];
		seg.data = p_str.ptr();
		seg.len = len;
		seg.is_char = false;
		total_length += seg.len;
	}
}

void OmakeStringAppender::operator+=(const char *p_cstr) {
	// FIXME: Do something here when we are joining too many strings
	// Maybe consolidate them?
	if (!p_cstr || segment_count >= MAX_SEGMENTS) {
		return;
	}

	size_t len = strlen(p_cstr);
	if (len == 0) {
		return;
	}

	Segment &seg = segments[segment_count++];
	seg.data = reinterpret_cast<const char32_t *>(p_cstr);
	seg.len = len;
	seg.is_char = true;
	total_length += len;
}

String OmakeStringAppender::get_string() {
	String result;
	result.resize(total_length);
	consolidate(result.ptrw());

	segment_count = 0;
	total_length = 0;
	return result;
}

void OmakeStringAppender::consolidate(char32_t *dest) const {
	size_t offset = 0;
	for (size_t i = 0; i < segment_count; ++i) {
		const Segment &seg = segments[i];
		if (seg.is_char) {
			const char *src = reinterpret_cast<const char *>(seg.data);
			const size_t len = seg.len;
			for (size_t j = 0; j < len; ++j) {
				// FIXME: Investigate the consistency of this. As it is done 4 different ways inside String
				dest[offset + j] = (char32_t)(unsigned char)src[j];
			}
		} else {
			memcpy(dest + offset, seg.data, seg.len * sizeof(char32_t));
		}
		offset += seg.len;
	}
}

String OmakeStringAppender::merge_strings(const String &a, const String &b) {
	const int a_len = a.length();
	const int b_len = b.length();
	if (a_len == 0 && b_len == 0) {
		return String();
	}
	if (a_len == 0) {
		return b;
	}
	if (b_len == 0) {
		return a;
	}

	String ret;
	ret.resize(a_len + b_len + 1);
	char32_t *ret_ptrw = ret.ptrw();

	memcpy(ret_ptrw, a.ptr(), a_len * sizeof(char32_t));
	ret_ptrw += a_len;

	memcpy(ret_ptrw, b.ptr(), b_len * sizeof(char32_t));
	ret_ptrw += b_len;

	*ret_ptrw = 0;

	return ret;
}

String OmakeStringAppender::merge_strings2(const char *p_chr, const String &b) {
	const bool is_a_empty = (*p_chr == '\0');
	const int b_len = b.length();
	if (is_a_empty && b_len == 0) {
		return String();
	} else if (is_a_empty) {
		return b;
	} else if (b_len == 0) {
		return String(p_chr);
	}

	//const StrRange<char> &p_cstr = StrRange<char>::from_c_str(p_chr);
	//const int a_len = p_cstr.len;
	const int a_len = strlen(p_chr);

	String ret;
	ret.resize(a_len + b_len + 1);
	char32_t *ret_ptrw = ret.ptrw();

	const char *src = p_chr;//p_cstr.c_str;
	const char *end = src + a_len;
	for (; src < end; ++src, ++ret_ptrw) {
		// FIXME: Investigate the consistency of this. As it is done 4 different ways inside String
		*ret_ptrw = static_cast<uint8_t>(*src);
	}

	memcpy(ret_ptrw, b.ptr(), b_len * sizeof(char32_t));
	ret_ptrw += b_len;

	*ret_ptrw = 0;

	return ret;
}

void OmakeStringAppender::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_string"), &OmakeStringAppender::get_string);
	ClassDB::bind_static_method("OmakeStringAppender", D_METHOD("merge_strings", "a", "b"), &OmakeStringAppender::merge_strings);
}
