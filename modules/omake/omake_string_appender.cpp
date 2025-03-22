
#include "omake_string_appender.h"

OmakeStringAppender::OmakeStringAppender(const int buffer_size, const int inc_buffer_size) {
	_buffer_size = buffer_size;
	_inc_buffer_size = inc_buffer_size;
	buffer.resize(_buffer_size);
	//for (int i = 0; i < _buffer_size; i++) {
	//	buffer[i] = 'X';
	//}
	ret_ptrw = buffer.ptrw();
	str_len = 0;
}
/*
// FIXME: Make the default buffer size the same as the arg length?
OmakeStringAppender::OmakeStringAppender(String arg, const int buffer_size, const int inc_buffer_size) {
	_buffer_size = buffer_size;
	_inc_buffer_size = inc_buffer_size;
	buffer = arg;//std::move(arg);
	ret_ptrw = buffer.ptrw();
	str_len = buffer.length();
}

// FIXME: Make the default buffer size the same as the arg length?
OmakeStringAppender::OmakeStringAppender(const char* arg, const int buffer_size, const int inc_buffer_size) {
	_buffer_size = buffer_size;
	_inc_buffer_size = inc_buffer_size;
	buffer = String(arg);
	ret_ptrw = buffer.ptrw();
	str_len = buffer.length();
}
*/

OmakeStringAppender::~OmakeStringAppender() {
	buffer.resize(0);
	ret_ptrw = buffer.ptrw();
	str_len = 0;
}

void OmakeStringAppender::_grow_buffer(size_t len) {
	while ((size_t)buffer.size() <= str_len + len) {
		buffer.resize(buffer.size() + str_len + len + _inc_buffer_size);
		ret_ptrw = buffer.ptrw();
	}
}

void OmakeStringAppender::operator+=(const String &p_str) {
	size_t len = p_str.length();
	if (len == 0) {
		return;
	}

	_grow_buffer(len);

	memcpy(ret_ptrw + str_len, p_str.ptr(), len * sizeof(char32_t));
	str_len += len;
}

void OmakeStringAppender::operator+=(const char *p_cstr) {
	if (!p_cstr) {
		return;
	}
	size_t len = strlen(p_cstr);
	if (len == 0) {
		return;
	}

	_grow_buffer(len);

	char32_t *dest = ret_ptrw + str_len;
	for (size_t j = 0; j < len; ++j) {
		dest[j] = (char32_t)(unsigned char)p_cstr[j];
	}
	str_len += len;
}

void OmakeStringAppender::replace(const String &p_key, const String &p_with) {
	int search_from = 0;
	int result_pos = 0;

	int key_len = p_key.length();
	int with_len = p_with.length();
	int size_diff = with_len - key_len;
	const char32_t *with_ptr = p_with.ptr();
//	fprintf(stderr, "!!! size_diff: %d\n", size_diff); fflush(stderr);

	while ((result_pos = buffer.find(p_key, search_from)) >= 0) {
//		fprintf(stderr, "!!! buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Grow
		if (size_diff > 0) {
			_grow_buffer(size_diff);
			_shift_buffer_right(result_pos, size_diff);
//			fprintf(stderr, "!!! grow buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Shrink
		} else if (size_diff < 0) {
			_shift_buffer_left(result_pos, -size_diff);
//			fprintf(stderr, "!!! shrink buffer.length: %d\n", buffer.length()); fflush(stderr);
		}

		// Copy with string into buffer
		ret_ptrw = buffer.ptrw();
		memcpy(ret_ptrw + result_pos, with_ptr, with_len * sizeof(char32_t));

		search_from = result_pos + with_len;
		str_len -= key_len;
		str_len += with_len;
	}
}

void OmakeStringAppender::replace(const char* p_key, const char* p_with) {
	int search_from = 0;
	int result_pos = 0;

	int key_len = strlen(p_key);
	int with_len = strlen(p_with);
	int size_diff = with_len - key_len;
//	fprintf(stderr, "!!! size_diff: %d\n", size_diff); fflush(stderr);

	while ((result_pos = buffer.find(p_key, search_from)) >= 0) {
//		fprintf(stderr, "!!! buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Grow
		if (size_diff > 0) {
			_grow_buffer(size_diff);
			_shift_buffer_right(result_pos, size_diff);
//			fprintf(stderr, "!!! grow buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Shrink
		} else if (size_diff < 0) {
			_shift_buffer_left(result_pos, -size_diff);
//			fprintf(stderr, "!!! shrink buffer.length: %d\n", buffer.length()); fflush(stderr);
		}

		// Copy with string into buffer
		ret_ptrw = buffer.ptrw();
		char32_t *dest = ret_ptrw + result_pos;
		for (int i = 0; i < with_len; ++i) {
//			fprintf(stderr, "!!! i: %d\n", i); fflush(stderr);
			dest[i] = (char32_t)(unsigned char)p_with[i];
		}

		search_from = result_pos + with_len;
		str_len -= key_len;
		str_len += with_len;
	}
}

void OmakeStringAppender::_shift_buffer_left(int pos, int count) {
	CowData<char32_t> &data = buffer._cowdata;
	//for (int i = 0; i < count; i++) {
	//	data.remove_at(pos);
	//}
	char32_t* p = data.ptrw();
	//memcpy(p + pos, p + pos + count, str_len * sizeof(char32_t));
	// abcXXXdefg
	// XXX000defg
	for (int i = pos; i < pos + count; i++) {
		p[i - count] = p[i];
	}
}

void OmakeStringAppender::_shift_buffer_right(int pos, int count) {
	CowData<char32_t> &data = buffer._cowdata;
	//for (int i = 0; i < count; i++) {
	//	data.insert(pos, 0);
	//}
	char32_t* p = data.ptrw();
	//memcpy(p + pos, p + pos + count, str_len * sizeof(char32_t));
	// abcXXXdefg
	// abc000XXXg
	for (int i = pos + count; i > pos; i--) {
		p[i + count] = p[i];
	}
}

String OmakeStringAppender::get_string() {
	buffer.resize(str_len + 1);
	ret_ptrw = buffer.ptrw();
	ret_ptrw[str_len] = '\0';
	//return buffer.substr(0, str_len);
	return buffer;
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

	const char *src = p_chr; //p_cstr.c_str;
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
