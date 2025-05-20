
#include "omake_string_appender.h"

OmakeStringAppender::OmakeStringAppender(const int buffer_size, const int inc_buffer_size) {
	_buffer_size = buffer_size;
	_inc_buffer_size = inc_buffer_size;
	buffer.resize(_buffer_size);
	ret_ptrw = buffer.ptrw();
	str_len = 0;
/*
	for (int i = 0; i < _buffer_size; i++) {
		buffer[i] = '\0';
	}
*/
	//memset(ret_ptrw, '\0', _buffer_size);
	std::fill(ret_ptrw, ret_ptrw + _buffer_size, '\0');
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

void OmakeStringAppender::replace(const String &p_key, const String &p_with, bool is_logging) {
	int search_from = 0;
	int result_pos = 0;

	int key_len = p_key.length();
	int with_len = p_with.length();
	int size_diff = with_len - key_len;
	const char32_t *with_ptr = p_with.ptr();
	if (is_logging) {
		fprintf(stderr, "!!! key_len: %d\n", key_len); fflush(stderr);
		fprintf(stderr, "!!! with_len: %d\n", with_len); fflush(stderr);
		fprintf(stderr, "!!! size_diff: %d\n", size_diff); fflush(stderr);
	}

	while ((result_pos = buffer.find(p_key, search_from)) >= 0) {
		// Skip if result is outside string length and in other part of the buffer
		if (result_pos >= str_len) {
			break;
		}

		if (is_logging) {
			fprintf(stderr, "-------------------------------------------loop \n"); fflush(stderr);
			fprintf(stderr, "    search_from: %d\n", search_from); fflush(stderr);
		}

//		fprintf(stderr, "!!! buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Grow
		if (size_diff > 0) {
			_grow_buffer(size_diff);

			if (is_logging) {
				fprintf(stderr, "        \"");
				for (int i = 0; i <str_len; i++) {
					fprintf(stderr, "%s", (String("") + buffer[i]).utf8().get_data());
				}
				fprintf(stderr, "\"\n"); fflush(stderr);
			}

			int start_index = result_pos - size_diff;
			int end_index = str_len;
			int shift_count = size_diff;
			if (is_logging) {
				fprintf(stderr, "    !! start_index: %d\n", start_index); fflush(stderr);
				fprintf(stderr, "    !! end_index: %d\n", end_index); fflush(stderr);
				fprintf(stderr, "    !! shift_count: %d\n", shift_count); fflush(stderr);
			}
			_shift_buffer_right(start_index, end_index, shift_count, false);
			if (is_logging) {
				fprintf(stderr, "        \"");
				for (int i = 0; i <str_len; i++) {
					fprintf(stderr, "%s", (String("") + buffer[i]).utf8().get_data());
				}
				fprintf(stderr, "\"\n"); fflush(stderr);
			}
//			fprintf(stderr, "!!! grow buffer.length: %d\n", buffer.length()); fflush(stderr);
		// Shrink
		} else if (size_diff < 0) {
			int start_index = result_pos - size_diff;
			int end_index = str_len;
			int shift_count = -size_diff;
			if (is_logging) {
				fprintf(stderr, "    !! start_index: %d\n", start_index); fflush(stderr);
				fprintf(stderr, "    !! end_index: %d\n", end_index); fflush(stderr);
				fprintf(stderr, "    !! shift_count: %d\n", shift_count); fflush(stderr);
			}
			_shift_buffer_left(start_index, end_index, shift_count, false);
			if (is_logging) {
				fprintf(stderr, "        \"");
				for (int i = 0; i <str_len; i++) {
					fprintf(stderr, "%s", (String("") + buffer[i]).utf8().get_data());
				}
				fprintf(stderr, "\"\n"); fflush(stderr);
			}
//			fprintf(stderr, "!!! shrink buffer.length: %d\n", buffer.length()); fflush(stderr);
		}

		// Copy with string into buffer
		if (is_logging) {
			fprintf(stderr, "    result_pos: %d\n", result_pos); fflush(stderr);
			fprintf(stderr, "    with_len: %d\n", with_len); fflush(stderr);
		}
		ret_ptrw = buffer.ptrw();
		memcpy(ret_ptrw + result_pos, with_ptr, with_len * sizeof(char32_t));

		search_from = result_pos + with_len;
		str_len -= key_len;
		str_len += with_len;
	}
}

void OmakeStringAppender::_shift_buffer_left(int start_index, int end_index, int shift_count, bool is_logging) {
	CowData<char32_t> &data = buffer._cowdata;
	//for (int i = 0; i < count; i++) {
	//	data.remove_at(pos);
	//}
	char32_t* p = data.ptrw();
	// FIXME: Update this to use memcpy
	for (int i = start_index; i < end_index; i++) {
		p[i - shift_count] = p[i];
	}
}

void OmakeStringAppender::_shift_buffer_right(int start_index, int end_index, int shift_count, bool is_logging) {
	CowData<char32_t> &data = buffer._cowdata;
	//for (int i = 0; i < count; i++) {
	//	data.insert(pos, 0);
	//}
	char32_t* p = data.ptrw();
	// FIXME: Update this to use memcpy
	for (int i = end_index; i >= start_index; i--) {
		p[i + shift_count] = p[i];
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
