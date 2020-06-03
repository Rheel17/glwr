/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_UTIL_H
#define GLWR_UTIL_H

#include <cstring>
#include <string>
#include <sstream>
#include <vector>

std::string triml(const std::string& str) {
	size_t i = 0;
	while (i < str.length() && isspace(str[i])) {
		i++;
	}
	return str.substr(i);
}

std::string trimr(const std::string& str) {
	size_t length = str.size();
	while (length > 0 && isspace(str[length - 1])) {
		length--;
	}
	return std::string(str.c_str(), length);
}

std::string trim(const std::string& str) {
	return triml(trimr(str));
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

std::vector<std::string> splitLine(const std::string& str, unsigned int width) {
	if (str.size() <= width) {
		return std::vector<std::string>{ str };
	}

	std::vector<std::string> lines;
	std::istringstream ss(str);

	unsigned int currentWidth = 0;
	std::stringstream currentLine;
	std::string word;

	const auto finishLine = [&]() {
		lines.push_back(currentLine.str());
		currentLine = std::stringstream();
		currentWidth = 0;
	};

	while (std::getline(ss, word, ' ')) {
		if (currentWidth == 0) {
			currentLine << word;
			currentWidth += word.size();
			continue;
		}

		if (currentWidth + word.size() + 1 > width) {
			finishLine();
		} else {
			currentLine << ' ';
		}

		currentLine << word;
		currentWidth += word.size() + 1;
	}

	if (currentWidth > 0) {
		finishLine();
	}

	return lines;
}

std::vector<std::string> split(const std::string& str, unsigned int width) {
	std::vector<std::string> lines;
	std::istringstream ss(str);
	std::string line;

	while (std::getline(ss, line, '\n')) {
		auto lineLines = splitLine(line, width);
		lines.insert(lines.end(), lineLines.begin(), lineLines.end());
	}

	return lines;
}

template<typename Node>
Node findChildNodeWithAttribute(Node node, const char* child, const char* attribute, const char* value) {
	for (auto n = node->first_node(child); n != nullptr; n = n->next_sibling(child)) {
		auto attr = n->first_attribute(attribute);
		if (attr && strcmp(attr->value(), value) == 0) {
			return n;
		}
	}

	return nullptr;
}

#endif