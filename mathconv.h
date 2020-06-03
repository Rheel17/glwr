/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_MATHCONV_H
#define GLWR_MATHCONV_H

#include <string>
#include <regex>

const static inline std::regex mmlmath = std::regex(R"(<(inlineequation|informalequation)>(?:[^<]*?<(?!\1))*?[^<]*?<\/\1>)");

std::string doMathReplace(const std::string& eq) {
	return "[math]";
}

void mathReplace(std::string& xml) {
	std::smatch match;
	while (std::regex_search(xml, match, mmlmath)) {
		std::string eq = match.str();
		xml = xml.substr(0, match.position()) + doMathReplace(eq) + xml.substr(match.position() + match.length());
	}
}

#endif