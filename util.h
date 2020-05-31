/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_UTIL_H
#define GLWR_UTIL_H

#include <string>
#include <sstream>

std::string trimr(const std::string& str) {
	size_t length = str.size();
	while (length > 0 && isspace(str[length - 1])) {
		length--;
	}
	return std::string(str.c_str(), length);
}

std::string rmln(const std::string& str) {
	bool wasLastSpace = false;
	std::stringstream ss;

	for (char c : str) {
		if (isspace(c)) {
			if (wasLastSpace) {
				continue;
			}

			ss << ' ';
			wasLastSpace = true;
			continue;
		}

		ss << c;
		wasLastSpace = false;
	}

	return ss.str();
}

#endif