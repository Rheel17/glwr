/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "Refpage.h"
#include "gl1.h"

#include <fstream>

#include <ctre.hpp>

constexpr static auto glwrFunctionHeaderHead = R"(#ifndef OPENGL_GLWR_H_
#error "Do not include glwr function headers directly, include GL/glwr.h
#endif
)";

static std::string& trimr(std::string& str) {
	auto [result, spaces] = ctre::match<".*?( +)">(str);
	str.erase(spaces.begin(), spaces.end());
	return str;
}

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

void Refpage::GenerateHeader(std::ostream& output) const {
	output << glwrFunctionHeaderHead;

	// #undef any non-gl1 prototype
	bool first = true;
	for (const auto& prototype : _refsynopsisdiv.funcprototypes) {
		if (gl1.find(prototype.funcdef.function) == gl1.end()) {
			if (first) {
				output << std::endl;
			}

			output << "#undef " << prototype.funcdef.function << std::endl;
			first = false;
		}
	}

	// declare all function prototypes
	for (const auto& prototype : _refsynopsisdiv.funcprototypes) {
		GenerateHeader_(output, prototype);
	}
}

void Refpage::Set_(const char* name, std::string& str, std::string_view value) {
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
}

void Refpage::ParseRefentry_(Node refentry) {
	for (const auto& [node, name] : NodeNameIterator(refentry)) {
		ParseNode_(node);
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
	for (const auto& [node, name] : NodeNameIterator(info)) {
		if (name == "copyright") {
			impl_copyright& value = _copyrights.emplace_back();
			ParseCopyright_(node, value);
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
			std::string purpose = ParseText_(node);
			Set_("refnamediv.refpurpose", _refnamediv.refpurpose, purpose);
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
			std::string type = node->value();
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef(value)", value.type, trimr(type));
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
			std::string type = node->value();
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.paramdef(value)", value.type, trimr(type));
		} else if (name == "parameter") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef.parameter", value.parameter, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype.paramdef." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1Parameters_(Node refsect1) {
	auto& parameters = _refsect_parameters.emplace();
	ParseParameters_(refsect1, parameters);
}

void Refpage::ParseRefsect1Parameters2_(Node refsect1) {
	auto& parameters2 = _refsect_parameters_2.emplace();
	ParseParameters_(refsect1, parameters2);
}

void Refpage::ParseRefsect1Description_(Node refsect1) {
	auto& description = _refsect_description.emplace();
	ParseDescription_(refsect1, description);
}

void Refpage::ParseRefsect1Description2_(Node refsect1) {
	auto& description2 = _refsect_description_2.emplace();
	ParseDescription_(refsect1, description2);
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
		std::string value = ParseAbstractTextNode_(node, name);

		if (!value.empty()) {
			text.elements.push_back(std::move(value));
		}
	}
}

std::string Refpage::ParseAbstractTextNode_(Node node, const std::string_view& name) {
	if (name == "title") {
		return ""; /* ignored */
	} else if (name == "para") {
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
	} else if (name == "superscript") {
		return ParseValueNode_(node, name, "<sup>", "</sup>");
	} else if (name == "emphasis") {
		return ParseEmphasis_(node);
	} else if (name == "citerefentry") {
		return ParseCiterefentry_(node);
	} else if (name == "footnote") {
		return ""; /* ignored for now */
	} else if (name == "informaltable") {
		return ParseInformaltable_(node);
	} else if (name == "table") {
		return ParseTable_(node);
	} else if (name == "programlisting") {
		return ParseProgramlisting_(node);
	} else if (name == "itemizedlist") {
		return ParseItemizedlist_(node);
	} else if (name == "variablelist") {
		return ParseVariablelistGlosslist_(node);
	} else if (name == "glosslist") {
		return ParseVariablelistGlosslist_(node);
	} else if (name == "inlineequation") {
		return ParseInlineequation_(node);
	} else if (name == "informalequation") {
		return ParseInformalequation_(node);
	} else {
		std::cout << "@" << _name << " Unknown text node: " << name << std::endl;
		return "";
	}
}

std::string Refpage::ParseValueNode_(Node node, const std::string_view& name, const char* begin, const char* end) {
	return begin + ParseText_(node) + end;
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
	return ParseValueNode_(para, "para", "<p>", "</p>");
}

std::string Refpage::ParseText_(Node para) {
	constexpr ctll::fixed_string regex_whitespace = "\r?\n\\s*";
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
	return ParseTable_(informaltable);
}

std::string Refpage::ParseTable_(Node table) {
	std::stringstream ss;

	for (const auto& [node, name] : NodeNameIterator(table)) {
		if (name == "title") {
			ss << ParseValueNode_(node, name, "<title>", "</title>\n");
		} else if (name == "tgroup") {
			ParseTableGroup_(node, ss);
		} else {
			std::cout << "@" << _name << " Unknown node: (informal?)table." << name << std::endl;
		}
	}

	return ss.str();
}

void Refpage::ParseTableGroup_(Node tgroup, std::stringstream& ss) {
	for (const auto& [node, name] : NodeNameIterator(tgroup)) {
		if (name == "colspec") {
			continue; /* ignored */
		} else if (name == "thead" || name == "tbody") {
			ParseTableRows_(node, name, ss);
		} else {
			std::cout << "@" << _name << " Unknown node: (informal?)table.tgroup." << name << std::endl;
		}
	}
}

void Refpage::ParseTableRows_(Node node, const std::string_view& name, std::stringstream& ss) {
	bool head = name == "thead";

	for (const auto& [node, name] : NodeNameIterator(node)) {
		if (name == "row") {
			ss << "<tr>\n";
			ParseInformaltableRow_(node, head, ss);
			ss << "</tr>\n";
		} else {
			std::cout << "@" << _name << " Unknown node: (informal?)table.tgroup." << name << "." << name << std::endl;
		}
	}
}

void Refpage::ParseInformaltableRow_(Node row, bool head, std::stringstream& ss) {
	for (const auto& [node, name] : NodeNameIterator(row)) {
		if (name == "entry") {
			ss << "<";
			ss << (head ? "th" : "td");
			ss << " style=\"border:1px solid; padding:5px; margin:0px;\">\n";
			ss << ParseText_(node);
			ss << "</";
			ss << (head ? "th" : "td");
			ss << ">\n";
		} else {
			std::cout << "@" << _name << " Unknown row node: " << name << std::endl;
		}
	}
}

std::string Refpage::ParseProgramlisting_(Node programlisting) {
	constexpr ctll::fixed_string regex_line = "([^\\r\\n]*)(\\r?\\n)?(.*)";

	std::stringstream contents;

	for (const auto& [node, name] : NodeNameIterator(programlisting)) {
		if (name == "") {
			contents << node->value();
		} else {
			contents << ParseAbstractTextNode_(node, name);
		}
	}

	std::stringstream ss;
	std::string valueString = contents.str();
	std::string_view value = valueString;

	while (!value.empty()) {
		const auto& [match, line, linefeed, rest] = ctre::match<regex_line>(value);
		std::string_view lineValue = line;
		ss << "<pre>" << lineValue << "</pre>" << std::endl;
		value = rest;
	}

	return ss.str();
}

std::string Refpage::ParseItemizedlist_(Node itemizedlist) {
	std::stringstream ss;
	ss << "<ul>\n";

	for (const auto& [node, name] : NodeNameIterator(itemizedlist)) {
		if (name == "listitem") {
			ss << ParseValueNode_(node, name, "<li>", "</li>\n");
		} else {
			std::cout << "@" << _name << " Unknown node: itemizedlist." << name << std::endl;
		}
	}

	ss << "</ul>\n";
	return ss.str();
}

std::string Refpage::ParseVariablelistGlosslist_(Node variablelist) {
	std::stringstream ss;
	ss << "<table>\n";

	for (const auto&[node, name] : NodeNameIterator(variablelist)) {
		if (name == "varlistentry" || name == "glossentry") {
			ss << ParseVarlistentryGlossentry_(node);
		} else {
			std::cout << "@" << _name << " Unknown node: " << variablelist->name() << "." << name << std::endl;
		}
	}

	ss << "</table>";
	return ss.str();
}

std::string Refpage::ParseVarlistentryGlossentry_(Node varlistentry) {
	std::vector<std::string> terms;
	impl_abstract_text text;

	for (const auto& [node, name] : NodeNameIterator(varlistentry)) {
		if (name == "term" || name == "glossterm") {
			terms.push_back(ParseValueNode_(node, name, "<i><code>", "</code></i>"));
		} else if (name == "listitem" || name == "glossdef") {
			ParseAbstractText_(node, text);
		} else {
			std::cout << "@" << _name << " Unknown node: " << varlistentry->name() << "." << name << std::endl;
		}
	}

	std::stringstream ss;
	ss << "<tr>\n";
	ss << "<th>\n";

	bool first = true;
	for (const auto& term : terms) {
		if (!first) {
			ss << ", ";
		}

		ss << term;
		first = false;
	}

	ss << "</th>\n";
	ss << "<td>&nbsp;&nbsp;</th>\n";
	ss << "<td>\n";

	for (const auto& t : text.elements) {
		ss << t << std::endl;
	}

	ss << "</td>\n";
	ss << "</tr>\n";
	return ss.str();
}

std::string Refpage::ParseInlineequation_(Node inlineequation) {
	if (Node mmlMath = GetOnlyChild_(inlineequation, "inlineequation", "mml:math"); mmlMath) {
		std::stringstream ss;

		for (const auto& [node, name] : NodeNameIterator(mmlMath)) {
			ss << ParseAbstractMathNode_(node, name);
		}

		std::string value = ss.str();

		// an inline table can only be displayed if the entire thing is in a
		// table. So if we have a table, enclose the equation.
		bool table = hasDescendant(inlineequation, "mml:mtable");
		if (table) {
			value = "<table><tr><td> " + value + " </td></tr></table>";
		}

		return value;
	}

	return "";
}

std::string Refpage::ParseInformalequation_(Node informalequation) {
	return ParseInlineequation_(informalequation);
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
	} else if (name == "mml:mtext") {
		return ParseMmlmtext_(node);
	} else if (name == "mml:mfenced") {
		return ParseMmlmfenced_(node);
	} else if (name == "mml:mrow") {
		return ParseMmlmrow_(node);
	} else if (name == "mml:msup") {
		return ParseMmlmsup_(node);
	} else if (name == "mml:msub") {
		return ParseMmlmsub_(node);
	} else if (name == "mml:mfrac") {
		return ParseMmlmfrac_(node);
	} else if (name == "mml:mtable") {
		return ParseMmlmtable_(node);
	} else if (name == "mml:mtr") {
		return ParseMmlmtr_(node);
	} else if (name == "mml:mtd") {
		return ParseMmlmtd_(node);
	} else if (name == "mml:mspace") {
		return ParseMmlmspace_(node);
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

std::string Refpage::ParseMmlmtext_(Node mmlmtext) {
	std::string value = ParseMathValue_(mmlmtext);
	std::string open;
	std::string close;

	if (auto attr = mmlmtext->first_attribute("mathvariant"); attr) {
		std::string mathvariant = attr->value();

		if (mathvariant == "italic") {
			open = "<i>";
			close = "</i>";
		} else {
			std::cout << "@" << _name << " Unknown mml:mtext mathvariant value: " << mathvariant << std::endl;
		}
	} else {
		open = "";
		close = "";
	}

	return open + value + close;
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

std::string Refpage::ParseMmlmrow_(Node mmlmrow) {
	return ParseMathValue_(mmlmrow);
}

std::string Refpage::ParseMmlmsup_(Node mmlmsup) {
	auto base = mmlmsup->first_node();
	auto superscript = base->next_sibling();
	return ParseAbstractMathNode_(base, base->name()) + "<sup>" + ParseAbstractMathNode_(superscript, superscript->name()) + "</sup>";
}

std::string Refpage::ParseMmlmsub_(Node mmlmsub) {
	auto base = mmlmsub->first_node();
	auto subscript = base->next_sibling();
	return ParseAbstractMathNode_(base, base->name()) + "<sub>" + ParseAbstractMathNode_(subscript, subscript->name()) + "</sub>";
}

std::string Refpage::ParseMmlmfrac_(Node mmlmsub) {
	auto numerator = mmlmsub->first_node();
	auto denominator = numerator->next_sibling();
	return " <sup>" + ParseAbstractMathNode_(numerator, numerator->name()) + "</sup>/<sub>" + ParseAbstractMathNode_(denominator, denominator->name()) + "</sub> ";
}

std::string Refpage::ParseMmlmtable_(Node mmlmtable) {
	return " </rd><td><table>\n" + ParseMathValue_(mmlmtable) + "</table></rd>\n<td> ";
}

std::string Refpage::ParseMmlmtr_(Node mmlmtr) {
	return "<tr>\n" + ParseMathValue_(mmlmtr) + "</tr>\n";
}

std::string Refpage::ParseMmlmtd_(Node mmlmtd) {
	return "<td>" + ParseMathValue_(mmlmtd) + "</td>\n";
}

std::string Refpage::ParseMmlmspace_(Node) {
	return "&nbsp;&nbsp;&nbsp;&nbsp;";
}

void Refpage::ParseParameters_(Node refsect1, impl_refsect_parameters& parameters) {
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
			ParseAbstractText_(node, value.listitem.contents);
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

void Refpage::ParseDescription_(Node refsect1, impl_refsect_description& description) {
	// parse the optional function name
	for (const auto& [node, name] : NodeNameIterator(refsect1)) {
		if (name == "title") {
			Node function = node->first_node("function", 8, true);
			if (function) {
				description.impl_for_function.emplace(function->value());
			}
		}
	}

	// parse the text
	ParseAbstractText_(refsect1, description.contents);
}

void Refpage::GenerateHeader_(std::ostream& output, const impl_funcprototype& prototype) const {
	output << std::endl;

	// Generate the comments for this prototype
	GenerateComments_(output, prototype);

	// Output the function prototype
	if (gl1.find(prototype.funcdef.function) == gl1.end()) {
		output << "GLWR_INLINE ";
	}

	output << prototype.funcdef.type << " " << prototype.funcdef.function << "(";

	bool first = true;
	for (const auto& parameter : prototype.paramdefs) {
		if (!first) {
			output << ", ";
		}

		output << parameter.type << " " << parameter.parameter;
		first = false;
	}

	output << ")";

	if (gl1.find(prototype.funcdef.function) == gl1.end()) {
		// later than OpenGL 1, so give a definition
		std::string_view nongl(prototype.funcdef.function.begin() + 2, prototype.funcdef.function.end());

		output << " {" << std::endl;
		output << "\tGLEW_GET_FUN(__glew" << nongl << ")(";

		first = true;
		for (const auto& parameter : prototype.paramdefs) {
			if (!first) {
				output << ", ";
			}

			output << parameter.parameter;
			first = false;
		}

		output << ");" << std::endl;
		output << "}" << std::endl;
	} else {
		// OpenGL 1 function, so only the declaration
		output << ";" << std::endl;
	}
}

void Refpage::GenerateComments_(std::ostream& output, const Refpage::impl_funcprototype& prototype) const {
	if (prototype.funcdef.function == "glBindBuffer") {
		std::cout << "";
	}

	output << "///" << std::endl;

	// \brief
	output << "/// \\brief" << std::endl;
	GenerateText_(output, _refnamediv.refpurpose);

	// parameters
	if (_refsect_parameters) {
		const impl_refsect_parameters& parameters =
				_refsect_parameters_2.has_value() && _refsect_parameters_2->impl_for_function.value() == prototype.funcdef.function
						? _refsect_parameters_2.value()
						: _refsect_parameters.value();

		for (const auto& varlistentry : parameters.varlistentries) {
			bool first = true;

			for (const auto& term : varlistentry.terms) {
				if (!PrototypeHasParameter(prototype, term)) {
					continue;
				}

				if (first) {
					output << "///" << std::endl;
					output << "/// \\param";
				} else {
					output << ',';
				}

				output << ' ' << term;
				first = false;
			}

			if (!first) {
				output << std::endl;
				GenerateText_(output, varlistentry.listitem.contents);
			}
		}
	}

	output << "///" << std::endl;
}

void Refpage::GenerateText_(std::ostream& output, const Refpage::impl_abstract_text& text) const {
	for (const auto& element : text.elements) {
		GenerateText_(output, element);
	}
}

void Refpage::GenerateText_(std::ostream& output, std::string_view text) const {
	constexpr ctll::fixed_string regex_token = "( *)([^\\w< ]*(?:[\\w'\\-]+|<(code|i|b)>[^ <]{0,64}</\\3>|<pre>.*?</pre>|<.*?>)[^\\w< \\n]*)(.*)";

	// ignore any spaces at the beginning
	while (!text.empty() && *text.begin() == ' ') {
		text = text.substr(1);
	}

	// main text loop
	unsigned lineWidth = 0;

	while (!text.empty()) {
		// new line -> output the new line
		if (*text.begin() == '\n') {
			if (lineWidth == 0) {
				output << "/// " << std::endl;
			} else {
				output << std::endl;
				lineWidth = 0;
			}

			text = text.substr(1);
			continue;
		}

		// get the first token, its following whitespace, and the rest
		const auto& [match, spacesMatch, tokenMatch, _, restMatch] = ctre::match<regex_token>(text);

		if (!match) {
			std::cout << "@" << _name << ": token generation failed. Left: " << text << std::endl;
			return;
		}

		// retrieve the groups
		std::string_view token = tokenMatch;
		std::string_view spaces = spacesMatch;

		if (!token.empty()) {
			// output the token
			bool space = !spaces.empty();

			if (lineWidth == 0) {
				output << "///";
				space = true;
			} else if (lineWidth + token.length() + space > 77) {
				output << std::endl;
				output << "///";
				lineWidth = 0;
				space = true;
			}

			if (space) {
				output << ' ';
			}

			output << token;
			lineWidth += token.length() + space;
		}

		text = restMatch;
	}

	// end the final line
	if (lineWidth > 0) {
		output << std::endl;
	}
}

bool Refpage::PrototypeHasParameter(const Refpage::impl_funcprototype& prototype, std::string_view param) {
	for (const auto& paramdef : prototype.paramdefs) {
		if (paramdef.parameter == param) {
			return true;
		}
	}

	return false;
}
