/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "Refpage.h"

#include <fstream>

#include <ctre.hpp>

Refpage::Refpage(std::filesystem::path dir, std::istream& input, std::string name) :
		_dir(std::move(dir)),
		_name(std::move(name)) {

	// read the input
	std::string fileContents(
			(std::istreambuf_iterator<char>(input)),
			std::istreambuf_iterator<char>());
	char* buf = &fileContents[0];

	// create the XML document
	rapidxml::xml_document<> doc;
	doc.parse<0>(buf);

	// parse
	Parse_(doc);
}

void Refpage::Set_(const char* name, std::string& str, const char* value) {
	if (str.empty()) {
		str = value;
		return;
	}

	std::cout << "@" << _name << " Duplicate value: " << name << std::endl;
}

Node Refpage::GetOnlyChild_(Node node, const std::string_view& name, std::string_view child) {
	Node childNode = node->first_node();

	if (childNode->next_sibling()) {
		std::cout << "@" << _name << " node with multiple child nodes: " << name << std::endl;
	} else if (std::string(childNode->name()) == child) {
		return childNode;
	} else {
		std::cout << "@" << _name << " node with invalid child node: " << name << std::endl;
	}

	return nullptr;
}

void Refpage::Parse_(const Document& doc) {
	auto refentry = doc.first_node("refentry");
	ParseRefentry_(refentry);
	std::cout << "";
}

void Refpage::ParseRefentry_(Node refentry) {
	for (Node n : NodeIterator(refentry)) {
		ParseNode_(n);
	}
}

void Refpage::ParseNode_(Node node) {
	std::string name = node->name();

	if (name == "info") {
		ParseInfo_(node);
	} else if (name == "refmeta") {
		ParseRefmeta_(node);
	} else if (name == "refnamediv") {
		ParseRefnamediv_(node);
	} else if (name == "refsynopsisdiv") {
		ParseRefsynopsisdiv_(node);
	} else if (name == "refsect1") {
		ParseRefsect1_(node);
	} else {
		std::cout << "@" << _name << " Unknown node: " << name << std::endl;
	}
}

void Refpage::ParseInfo_(Node info) {
	for (Node n : NodeIterator(info)) {
		std::string name = n->name();
		if (name == "copyright") {
			impl_copyright& value = _copyrights.emplace_back();
			ParseCopyright_(n, value);
		} else {
			std::cout << "@" << _name << " Unknown node: info." << name << std::endl;
		}
	}
}

void Refpage::ParseRefmeta_(Node refmeta) {
	for (const auto& [node, name] : NodeNameIterator(refmeta)) {
		if (name == "refentrytitle") {
			Set_("refmeta.refentrytitle", _refmeta.refentrytitle, node->value());
		} else if (name == "manvolnum") {
			Set_("refmeta.manvolnum", _refmeta.manvolnum, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refmeta." << name << std::endl;
		}
	}
}

void Refpage::ParseRefnamediv_(Node refnamediv) {
	for (const auto& [node, name] : NodeNameIterator(refnamediv)) {
		if (name == "refdescriptor") {
			auto& refdescriptor = _refnamediv.refdescriptor.emplace();
			Set_("refnamediv.refdescriptor", refdescriptor, node->value());
		} else if (name == "refname") {
			_refnamediv.refnames.emplace_back(node->value());
		} else if (name == "refpurpose") {
			Set_("refnamediv.refpurpose", _refnamediv.refpurpose, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refnamediv." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsynopsisdiv_(Node refsynopsisdiv) {
	for (const auto& [node, name] : NodeNameIterator(refsynopsisdiv)) {
		if (name == "title") {
			continue; /* ignored */
		} else if (name == "funcsynopsis") {
			ParseFuncsynopsis_(node, _refsynopsisdiv);
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1_(Node refsect1) {
	auto attr = refsect1->first_attribute("xml:id");
	if (attr) {
		std::string id = attr->value();

		if (id == "parameters") {
			ParseRefsect1Parameters_(refsect1);
		} else if (id == "parameters2") {
			ParseRefsect1Parameters2_(refsect1);
		} else if (id == "description") {
			ParseRefsect1Description_(refsect1);
		} else if (id == "description2") {
			ParseRefsect1Description2_(refsect1);
		} else if (id == "examples") {
			ParseRefsect1Examples_(refsect1);
		} else if (id == "notes") {
			ParseRefsect1Notes_(refsect1);
		} else if (id == "errors") {
			ParseRefsect1Errors_(refsect1);
		} else if (id == "associatedgets") {
			ParseRefsect1Associatedgets_(refsect1);
		} else if (id == "versions") {
			ParseRefsect1Versions_(refsect1);
		} else if (id == "seealso") {
			ParseRefsect1Seealso_(refsect1);
		} else if (id == "Copyright") {
			ParseRefsect1Copyright_(refsect1);
		} else {
			std::cout << "@" << _name << " Unknown refsect1 xml:id: " << id << std::endl;
		}
	} else {
		std::cout << "@" << _name << " No attribute xml:id in refsect1" << std::endl;
	}
}

void Refpage::ParseCopyright_(Node copyright, impl_copyright& value) {
	for (const auto& [node, name] : NodeNameIterator(copyright)) {
		if (name == "year") {
			Set_("info.copyright.year", value.year, node->value());
		} else if (name == "holder") {
			Set_("info.copyright.holder", value.holder, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: info.copyright." << name << std::endl;
		}
	}
}

void Refpage::ParseFuncsynopsis_(Node funcsynopsis, impl_refsynopsisdiv& value) {
	for (const auto& [node, name] : NodeNameIterator(funcsynopsis)) {
		if (name == "funcprototype") {
			auto& value2 = value.funcprototypes.emplace_back();
			ParseFuncprototype_(node, value2);
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis." << name << std::endl;
		}
	}
}

void Refpage::ParseFuncprototype_(Node funcprototype, impl_funcprototype& value) {
	for (const auto& [node, name] : NodeNameIterator(funcprototype)) {
		if (name == "funcdef") {
			ParseFuncdef_(node, value.funcdef);
		} else if (name == "paramdef") {
			auto& value2 = value.paramdefs.emplace_back();
			ParseParamdef_(node, value2);
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype." << name << std::endl;
		}
	}
}

void Refpage::ParseFuncdef_(Node funcdef, impl_funcdef& value) {
	for (const auto& [node, name] : NodeNameIterator(funcdef)) {
		if (name == "") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef(value)", value.type, funcdef->value());
		} else if (name == "function") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef.function", value.function, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype.function." << name << std::endl;
		}
	}
}

void Refpage::ParseParamdef_(Node paramdef, impl_paramdef& value) {
	for (const auto& [node, name] : NodeNameIterator(paramdef)) {
		if (name == "") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.paramdef(value)", value.type, paramdef->value());
		} else if (name == "parameter") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef.parameter", value.parameter, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype.paramdef." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1Parameters_(Node refsect1) {
	auto& parameters = _refsect_parameters.emplace();

	for (const auto& [node, name] : NodeNameIterator(refsect1)) {
		if (name == "variablelist") {
			ParseVariablelist_(node, parameters);
		} else if (name == "title") {
			Node function = node->first_node("function", 8, true);
			if (function) {
				parameters.impl_for_function.emplace(function->value());
			}
		} else {
			std::cout << "@" << _name << " Unknown node: refsect1(parameters)." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1Parameters2_(Node refsect1) {
	auto& parameters2 = _refsect_parameters_2.emplace();

	for (const auto& [node, name] : NodeNameIterator(refsect1)) {
		if (name == "variablelist") {
			ParseVariablelist_(node, parameters2);
		} else if (name == "title") {
			Node function = node->first_node("function", 8, true);
			if (function) {
				parameters2.impl_for_function.emplace(function->value());
			}
		} else {
			std::cout << "@" << _name << " Unknown node: refsect1(parameters2)." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1Description_(Node refsect1) {

}

void Refpage::ParseRefsect1Description2_(Node refsect1) {

}

void Refpage::ParseRefsect1Examples_(Node refsect1) {

}

void Refpage::ParseRefsect1Notes_(Node refsect1) {

}

void Refpage::ParseRefsect1Errors_(Node refsect1) {

}

void Refpage::ParseRefsect1Associatedgets_(Node refsect1) {

}

void Refpage::ParseRefsect1Versions_(Node refsect1) {

}

void Refpage::ParseRefsect1Seealso_(Node refsect1) {

}

void Refpage::ParseRefsect1Copyright_(Node refsect1) {

}

void Refpage::ParseAbstractText_(Node parent, impl_abstract_text& text) {
	for (const auto& [node, name] : NodeNameIterator(parent)) {
		text.elements.push_back(ParseAbstractTextNode_(node, name));
	}
}

std::string Refpage::ParseAbstractTextNode_(Node node, const std::string_view& name) {
	if (name == "para") {
		return ParsePara_(node);
	} else if (name == "xi:include") {
		return ParseInclude_(node);
	} else if (name == "parameter") {
		return ParseValueNode_(node, name, "<code>", "</code>");
	} else if (name == "constant") {
		return ParseValueNode_(node, name, "<code>", "</code>");
	} else if (name == "function") {
		return ParseValueNode_(node, name, "<b><code>", "</code></b>");
	} else if (name == "code") {
		return ParseValueNode_(node, name, "<code>", "</code>");
	} else if (name == "emphasis") {
		return ParseEmphasis_(node);
	} else if (name == "citerefentry") {
		return ParseCiterefentry_(node);
	} else if (name == "footnote") {
		return ""; /* ignored for now */
	} else if (name == "informaltable") {
		return ParseInformaltable_(node);
	} else if (name == "inlineequation") {
		return ParseInlineequation_(node);
	} else {
		std::cout << "@" << _name << " Unknown text node: " << name << std::endl;
		return "";
	}
}

std::string Refpage::ParseValueNode_(Node node, const std::string_view& name, const char* begin, const char* end) {
	if (Node child = GetOnlyChild_(node, name, ""); child) {
		return begin + std::string(node->value()) + end;
	}

	return "";
}

std::string Refpage::ParseInclude_(Node include) {
	auto attr = include->first_attribute("href");
	if (!attr) {
		std::cout << "@" << _name << " xi:include without href" << std::endl;
		return "";
	}

	std::string filename = attr->value();
	std::filesystem::path path = _dir / filename;
	std::ifstream file(path.string());

	std::string fileContents(
			(std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
	char* buf = &fileContents[0];

	Document doc;
	doc.parse<0>(buf);

	Node node = doc.first_node();
	return ParseAbstractTextNode_(node, node->name());
}

std::string Refpage::ParsePara_(Node para) {
	constexpr ctll::fixed_string regex_whitespace = "\r\n\\s*";
	constexpr ctll::fixed_string regex_spaces = "^( *).*?( *)$";

	std::stringstream ss;

	for (const auto& [node, name] : NodeNameIterator(para)) {
		std::string value = node->value();

		if (name == "") {
			auto result = ctre::search<regex_whitespace>(value);
			bool hasWhitespace = result;

			while (hasWhitespace) {
				auto iter = value.erase(result.get<0>().begin(), result.get<0>().end());
				value.insert(iter, ' ');

				result = ctre::search<regex_whitespace>(value);
				hasWhitespace = result;
			}

			ss << value;
		} else {
			ss << ParseAbstractTextNode_(node, name);
		}
	}

	std::string result = ss.str();

	if (auto [found, begin, end] = ctre::match<regex_spaces>(result); found) {
		result.erase(end.begin(), end.end());
		result.erase(begin.begin(), begin.end());
	}

	return result;
}

std::string Refpage::ParseEmphasis_(Node emphasis) {
	std::string openTag;
	std::string closeTag;

	if (auto attr = emphasis->first_attribute("role"); attr) {
		std::string role = attr->value();

		if (role == "bold") {
			openTag = "<b>";
			closeTag = "</b>";
		} else {
			std::cout << "@" << _name << " Unknown emphasis role attribute: " << role << std::endl;
		}
	} else {
		openTag = "<i>";
		closeTag = "</i>";
	}

	return ParseValueNode_(emphasis, "emphasis", openTag.c_str(), closeTag.c_str());
}

std::string Refpage::ParseCiterefentry_(Node citerefentry) {
	if (Node child = GetOnlyChild_(citerefentry, "citerefentry", "refentrytitle"); child) {
		return ParseValueNode_(child, "refentrytitle", "<b>", "</b>");
	}

	return "";
}

std::string Refpage::ParseInformaltable_(Node informaltable) {
	if (Node tgroup = GetOnlyChild_(informaltable, "informaltable", "tgroup"); tgroup) {
		std::stringstream ss;
		ss << "<table style=\"border:1px solid; border-spacing:0px;\">\n";

		for (const auto& [node, name] : NodeNameIterator(tgroup)) {
			if (name == "colspec") {
				continue; /* ignored */
			} else if (name == "thead" || name == "tbody") {
				ParseInformaltableRows_(node, name, ss);
			} else {
				std::cout << "@" << _name << " Unknown node: informaltable.tgroup." << name << std::endl;
			}
		}

		ss << "<table>\n";
		return ss.str();
	}

	return "";
}

void Refpage::ParseInformaltableRows_(Node pNode, const std::string_view& pName, std::stringstream& ss) {
	bool head = pName == "thead";

	for (const auto& [node, name] : NodeNameIterator(pNode)) {
		if (name == "row") {
			ss << "<tr>\n";
			ParseInformaltableRow_(node, head, ss);
			ss << "</tr>\n";
		} else {
			std::cout << "@" << _name << " Unknown node: informaltable.tgroup." << pName << "." << name << std::endl;
		}
	}
}

void Refpage::ParseInformaltableRow_(Node row, bool head, std::stringstream& ss) {
	for (const auto& [node, name] : NodeNameIterator(row)) {
		if (name == "entry") {
			ss << "<";
			ss << (head ? "th" : "td");
			ss << " style=\"border:1px solid; padding:5px; margin:0px;\">\n";
			ss << ParsePara_(node);
			ss << "</";
			ss << (head ? "th" : "td");
			ss << ">\n";
		} else {
			std::cout << "@" << _name << " Unknown row node: " << name << std::endl;
		}
	}
}

std::string Refpage::ParseInlineequation_(Node inlineequation) {
	if (Node mmlMath = GetOnlyChild_(inlineequation, "inlineequation", "mml:math"); mmlMath) {
		std::stringstream ss;

		for (const auto& [node, name] : NodeNameIterator(mmlMath)) {
			ss << ParseAbstractMathNode_(node, name);
		}

		return ss.str();
	}

	return "";
}

std::string Refpage::ParseAbstractMathNode_(Node node, const std::string_view& name) {
	if (name == "") {
		return node->value();
	} else if (name == "mml:mi") {
		return ParseMmlmi_(node);
	} else if (name == "mml:mn") {
		return ParseMmlmn_(node);
	} else if (name == "mml:mo") {
		return ParseMmlmo_(node);
	} else if (name == "mml:mfenced") {
		return ParseMmlmfenced_(node);
	} else if (name == "mml:mrow") {
		return ParseMmlmrow_(node);
	} else if (name == "mml:msup") {
		return ParseMmlmsup_(node);
	} else {
		std::cout << "@" << _name << " Unknown math node: " << name << std::endl;
		return "";
	}
}

std::string Refpage::ParseMathValue_(Node parent) {
	std::stringstream ss;

	for (const auto& [node, name] : NodeNameIterator(parent)) {
		ss << ParseAbstractMathNode_(node, name);
	}

	return ss.str();
}

std::string Refpage::ParseMmlmi_(Node mmlmi) {
	std::string value = ParseMathValue_(mmlmi);
	std::string open;
	std::string close;

	if (auto attr = mmlmi->first_attribute("mathvariant"); attr) {
		std::string mathvariant = attr->value();

		if (mathvariant == "italic") {
			open = "<i>";
			close = "</i>";
		} else {
			std::cout << "@" << _name << " Unknown mml:mi mathvariant value: " << mathvariant << std::endl;
		}
	} else if (value.size() == 1) {
		open = "<i>";
		close = "</i>";
	} else {
		open = "";
		close = "";
	}

	return open + value + close;
}

std::string Refpage::ParseMmlmn_(Node mmlmn) {
	return ParseMathValue_(mmlmn);
}

std::string Refpage::ParseMmlmo_(Node mmlmo) {
	return ParseMathValue_(mmlmo);
}

std::string Refpage::ParseMmlmfenced_(Node mmlmfenced) {
	std::string value = ParseMathValue_(mmlmfenced);
	std::string open = "(";
	std::string close = ")";

	if (auto openAttr = mmlmfenced->first_attribute("open"); openAttr) {
		open = openAttr->value();
	}

	if (auto closeAttr = mmlmfenced->first_attribute("close"); closeAttr) {
		close = closeAttr->value();
	}

	return open + value + close;
}

std::string Refpage::ParseMmlmrow_(Node mmlmfenced) {
	return ParseMathValue_(mmlmfenced);
}

std::string Refpage::ParseMmlmsup_(Node mmlmsup) {
	auto base = mmlmsup->first_node();
	auto superscript = base->next_sibling();
	return ParseAbstractMathNode_(base, base->name()) + "<sup>" + ParseAbstractMathNode_(superscript, superscript->name()) + "</sup>";
}

void Refpage::ParseVariablelist_(Node variablelist, impl_refsect_parameters& parameters) {
	for (const auto& [node, name] : NodeNameIterator(variablelist)) {
		if (name == "varlistentry") {
			auto& varlistentry = parameters.varlistentries.emplace_back();
			ParseVarlistentry_(node, varlistentry);
		} else {
			std::cout << "@" << _name << " Unknown node: refsect1(parameters).variablelist." << name << std::endl;
		}
	}
}

void Refpage::ParseVarlistentry_(Node varlistentry, impl_varlistentry& value) {
	for (const auto& [node, name] : NodeNameIterator(varlistentry)) {
		if (name == "term") {
			ParseTerm_(node, value);
		} else if (name == "listitem") {
			auto& value2 = value.listitems.emplace_back();
			ParseAbstractText_(node, value2.contents);
		} else {
			std::cout << "@" << _name << " Unknown node: refsect1(parameters).variablelist.varlistentry." << name << std::endl;
		}
	}
}

void Refpage::ParseTerm_(Node term, impl_varlistentry& varlistentry) {
	for (const auto& [node, name] : NodeNameIterator(term)) {
		if (name == "") {
			continue; /* ignored */
		} else if (name == "parameter") {
			varlistentry.terms.emplace_back(node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsect1(parameters).variablelist.varlistentry.term." << name << std::endl;
		}
	}
}
