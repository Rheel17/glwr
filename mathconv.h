/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_MATHCONV_H
#define GLWR_MATHCONV_H

#include <rapidxml/rapidxml_print.hpp>

#include <string>
#include <regex>
#include <iostream>

const static inline std::regex mathRegex = std::regex(R"(<(inlineequation|informalequation)>(?:[^<]*?<(?!\1))*?[^<]*?<\/\1>)");

using N = rapidxml::xml_node<> *;

std::string parseNode(N node);

std::string parseValue(char *value) {
	return value;
}

std::string parseIdentifier(N node) {
	auto attr = node->first_attribute("mathvariant");
	if (attr != nullptr && strcmp(attr->value(), "italic") == 0) {
		return "<i>" + parseValue(node->value()) + "</i>";
	}
	return parseValue(node->value());
}

std::string parseNumber(N node) {
	return parseValue(node->value());
}

std::string parseOperator(N node) {
	return parseValue(node->value());
}

std::string parseFenced(N node) {
	std::stringstream ss;
	std::string open = node->first_attribute("open")->value();
	std::string close = node->first_attribute("close")->value();

	ss << open;
	bool first = true;

	for (N sub = node->first_node(); sub; sub = sub->next_sibling()) {
		if (!first) {
			ss << ", ";
		}

		ss << parseNode(sub);
		first = false;
	}

	ss << close;
	return ss.str();
}

std::string parseSub(N node) {
	auto base = node->first_node();
	auto subscript = base->next_sibling();

	return parseNode(base) + "<sub>" + parseNode(subscript) + "</sub>";
}

std::string parseSup(N node) {
	auto base = node->first_node();
	auto superscript = base->next_sibling();

	return parseNode(base) + "<sup>" + parseNode(superscript) + "</sup>";
}

std::string parseRow(N node) {
	std::stringstream ss;

	for (N sub = node->first_node(); sub; sub = sub->next_sibling()) {
		ss << parseNode(sub);
	}

	return ss.str();
}

std::string parseFrac(N node) {
	auto numerator = node->first_node();
	auto denumerator = numerator->next_sibling();

	return " <sup>" + parseNode(numerator) + "</sup>/<sub>" + parseNode(denumerator) + "</sub> ";
}

std::string parseTable(N node) {
	std::stringstream ss;
	ss << "<table>\n";

	for (N sub = node->first_node(); sub; sub = sub->next_sibling()) {
		ss << parseNode(sub);
	}

	ss << "\n</table>";
	return ss.str();
}

std::string parseTableRow(N node) {
	std::stringstream ss;
	ss << "<tr>\n";

	for (N sub = node->first_node(); sub; sub = sub->next_sibling()) {
		ss << parseNode(sub);
	}

	ss << "\n</tr>";
	return ss.str();
}

std::string parseTableCell(N node) {
	std::stringstream ss;
	ss << "<td>";

	for (N sub = node->first_node(); sub; sub = sub->next_sibling()) {
		ss << parseNode(sub);
	}

	ss << "</td>";
	return ss.str();
}

std::string parseNode(N node) {
	if (strcmp(node->name(), "mml:mi") == 0) {
		return parseIdentifier(node);
	} else 	if (strcmp(node->name(), "mml:mtext") == 0) {
		return parseIdentifier(node);
	} else if (strcmp(node->name(), "mml:mn") == 0) {
		return parseNumber(node);
	} else if (strcmp(node->name(), "mml:mo") == 0) {
		return parseOperator(node);
	} else if (strcmp(node->name(), "mml:mfenced") == 0) {
		return parseFenced(node);
	} else if (strcmp(node->name(), "mml:msub") == 0) {
		return parseSub(node);
	} else if (strcmp(node->name(), "mml:msup") == 0) {
		return parseSup(node);
	} else if (strcmp(node->name(), "mml:mrow") == 0) {
		return parseRow(node);
	} else if (strcmp(node->name(), "mml:mfrac") == 0) {
		return parseFrac(node);
	} else if (strcmp(node->name(), "mml:mtable") == 0) {
		return parseTable(node);
	} else if (strcmp(node->name(), "mml:mtr") == 0) {
		return parseTableRow(node);
	} else if (strcmp(node->name(), "mml:mtd") == 0) {
		return parseTableCell(node);
	} else if (strcmp(node->name(), "mml:mspace") == 0) {
		return " ";
	} else {
		std::cout << "Unknown math tag: " << node->name() << std::endl;
	}

	return "[math]";
}

std::string doMathReplace(std::string& eq) {
	rapidxml::xml_document<> doc;
	doc.parse<0>(&eq[0]);

	N equation = doc.first_node();
	N mmlmath = equation->first_node();
	N node = mmlmath->first_node();

	return " " + parseNode(node) + " ";
}

void mathReplace(std::string& xml) {
	std::smatch match;
	while (std::regex_search(xml, match, mathRegex)) {
		std::string eq = match.str();
		xml = xml.substr(0, match.position()) + doMathReplace(eq) + xml.substr(match.position() + match.length());
	}
}

#endif