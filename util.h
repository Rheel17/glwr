/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef MGL_UTIL_H
#define MGL_UTIL_H

#include <string>

std::string trimr(const std::string& str) {
	size_t length = str.size();
	while (length > 0 && isspace(str[length - 1])) {
		length--;
	}
	return std::string(str.c_str(), length);
}

#endif