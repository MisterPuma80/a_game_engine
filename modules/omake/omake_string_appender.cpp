
#include "omake_string_appender.h"

OmakeStringAppender::OmakeStringAppender() {
	this->clear();
}

void OmakeStringAppender::operator+=(const String &p_str) {
	size_t len = p_str.length();
	if (len == 0) {
		return;
	}

	while ((size_t)buffer.size() <= str_len + len) {
		buffer.resize(buffer.size() + str_len + len + 1024);
		ret_ptrw = buffer.ptrw();
	}

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

	while ((size_t)buffer.size() <= str_len + len) {
		buffer.resize(buffer.size() + str_len + len + 1024);
		ret_ptrw = buffer.ptrw();
	}

	char32_t *dest = ret_ptrw + str_len;
	for (size_t j = 0; j < len; ++j) {
		dest[j] = (char32_t)(unsigned char)p_cstr[j];
	}
	str_len += len;
}

String OmakeStringAppender::get_string() {
	buffer.resize(str_len + 1);
	ret_ptrw = buffer.ptrw();
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
