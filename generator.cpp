/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "util.h"
#include "gl1.h"
#include "mathconv.h"

#include <rapidxml/rapidxml_print.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <functional>

class Parameter {

public:
	std::string type;
	std::string name;

};

class Prototype {

public:
	bool in_gl1 = false;
	std::string type;
	std::string name;
	std::vector<Parameter> parameters;

};

class Function {
	const static inline std::regex _begin_tag_space = std::regex(R"(((?:.|^)[^\w\d\.,:;])\s<)");
	const static inline std::regex _end_tag_space = std::regex(R"(>\s+(?!\w|\d))");
	const static inline std::regex _para_begin = std::regex(R"(<para>\s*)");
	const static inline std::regex _para_end = std::regex(R"(\s*</para>)");;
	const static inline std::regex _entry_begin = std::regex(R"(<entry>\s*)");
	const static inline std::regex _entry_end = std::regex(R"(\s*</entry>)");
	const static inline std::regex _constant = std::regex(R"(<constant>(.*?)</constant>)");
	const static inline std::regex _function = std::regex(R"(<function>(.*?)</function>)");
	const static inline std::regex _parameter = std::regex(R"(<parameter>(.*?)</parameter>)");
	const static inline std::regex _emphasis = std::regex(R"(<emphasis .*?>(.*?)</emphasis>)");

public:
	explicit Function(const std::filesystem::path& path) {
		_name = path.filename().string();
		_name = _name.substr(0, _name.size() - 4);

		// open the file
		std::ifstream file(path.c_str(), std::ios::binary);
		std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		char* buf = &fileContents[0];

		// create the XML document
		rapidxml::xml_document<> doc;
		doc.parse<0>(buf);

		// parse the function
		Parse_(doc);
	}

	const std::string& GetName() const {
		return _name;
	}

	const std::string& GetPurpose() const {
		return _purpose;
	}

	const std::vector<Prototype>& GetPrototypes() const {
		return _prototypes;
	}

	const std::unordered_map<std::string, std::vector<std::string>>& GetParameters() const {
		return _parameters;
	}

	const std::vector<std::string>& GetDescription() const {
		return _description;
	}

private:
	void Parse_(const rapidxml::xml_document<>& doc) {
		auto refentry = doc.first_node("refentry");

		// read the purpose
		auto refnamediv = refentry->first_node("refnamediv");
		auto refpurpose = refnamediv->first_node("refpurpose");
		_purpose = rmln(refpurpose->value());

		// read the prototype(s)
		auto refsynopsisdiv = refentry->first_node("refsynopsisdiv");
		auto funcsynopsis = refsynopsisdiv->first_node("funcsynopsis");

		for (auto funcprototype = funcsynopsis->first_node("funcprototype");
			 funcprototype != nullptr;
			 funcprototype = funcprototype->next_sibling("funcprototype")) {

			_prototypes.push_back(ParsePrototype_(funcprototype));
		}

		// read the parameter(s)
		auto refsect1 = findChildNodeWithAttribute(refentry, "refsect1", "xml:id", "parameters");
		auto variablelist = refsect1 == nullptr ? nullptr : refsect1->first_node("variablelist");

		if (variablelist) {
			for (auto varlistentry = variablelist->first_node("varlistentry");
				 varlistentry != nullptr;
				 varlistentry = varlistentry->next_sibling("varlistentry")) {

				ParseParameter_(varlistentry);
			}
		}

		// read the description
		using NodeType = decltype(refsect1);

		const static std::unordered_set<std::string> invalidTags = { "title" };
		const static std::unordered_map<std::string, std::function<std::string(NodeType, const std::string&)>> validTags = {
				{ "para",           [this](auto n, auto name) { return ParseParagraph_(n); } },
				{ "informaltable",  [this](auto n, auto name) { return ParseInformalTable_(n); } },
				{ "table",          [this](auto n, auto name) { std::cout << "unimplemented tag: table at " << name << std::endl; return std::string(); } },
				{ "programlisting", [this](auto n, auto name) { std::cout << "unimplemented tag: programlisting at " << name << std::endl; return std::string(); } },
				{ "xi:include",     [this](auto n, auto name) { std::cout << "unimplemented tag: xi:include at " << name << std::endl; return std::string(); } },
				{ "itemizedlist",   [this](auto n, auto name) { std::cout << "unimplemented tag: itemizedlist at " << name << std::endl; return std::string(); } },
				{ "glosslist",      [this](auto n, auto name) { std::cout << "unimplemented tag: glosslist at " << name << std::endl; return std::string(); } },
				{ "variablelist",   [this](auto n, auto name) { std::cout << "unimplemented tag: variablelist at " << name << std::endl; return std::string(); } }
		};

		refsect1 = findChildNodeWithAttribute(refentry, "refsect1", "xml:id", "description");
		if (refsect1) {
			for (auto node = refsect1->first_node(); node != nullptr; node = node->next_sibling()) {
				if (invalidTags.find(node->name()) != invalidTags.end()) {
					continue;
				}

				auto iter = validTags.find(node->name());
				if (iter == validTags.end()) {
					std::cout << "Unknown description tag: " << node->name() << " at " << _name << std::endl;
					continue;
				}

				const auto& function = iter->second;
				_description.push_back(function(node, _name));
			}
		}
	}

	template<typename Node>
	static Prototype ParsePrototype_(Node funcprototype) {
		Prototype pt;

		auto funcdef = funcprototype->first_node("funcdef");
		pt.type = trimr(funcdef->value());

		auto function = funcdef->first_node("function");
		pt.name = function->value();
		pt.in_gl1 = gl1.find(pt.name) != gl1.end();

		for (auto paramdef = funcprototype->first_node("paramdef");
			 paramdef != nullptr;
			 paramdef = paramdef->next_sibling("paramdef")) {

			auto parameter = paramdef->first_node("parameter");
			if (parameter && strcmp(parameter->value(), "void") != 0) {
				Parameter& p = pt.parameters.emplace_back();
				p.type = trimr(paramdef->value());
				p.name = parameter->value();
			}
		}

		return pt;
	}

	template<typename Node>
	void ParseParameter_(Node varlistentry) {
		std::vector<std::string> paragraphs;

		auto listitem = varlistentry->first_node("listitem");
		for (auto para = listitem->first_node("para");
			 para != nullptr;
			 para = para->next_sibling("para")) {

			paragraphs.push_back(ParseParagraph_(para));
		}

		for (auto term = varlistentry->first_node("term");
		     term != nullptr;
		     term = term->next_sibling("term")) {

			for (auto parameter = term->first_node("parameter");
				 parameter != nullptr;
				 parameter = parameter->next_sibling("parameter")) {


				const char* param = parameter->value();
				_parameters[param] = paragraphs;
			}
		}
	}

	static void ReplaceTags_(std::string& contents) {
		contents = std::regex_replace(contents, _begin_tag_space, "$1<");
		contents = std::regex_replace(contents, _end_tag_space, ">");
		contents = std::regex_replace(contents, _para_begin, "");
		contents = std::regex_replace(contents, _para_end, "");
		contents = std::regex_replace(contents, _entry_begin, "");
		contents = std::regex_replace(contents, _entry_end, "");
		contents = std::regex_replace(contents, _constant, "<i><code>$1</code></i>");
		contents = std::regex_replace(contents, _function, "<b><code>$1</code></b>");
		contents = std::regex_replace(contents, _parameter, "<code>$1</code>");
		contents = std::regex_replace(contents, _emphasis, "$1");

		mathReplace(contents);
		contents = trim(contents);
	}

	template<typename Node>
	std::string ParseParagraph_(Node para) {
		std::string contents;
		rapidxml::print(std::back_inserter(contents), *para);

		contents = rmln(contents);
		ReplaceTags_(contents);

		return "<p>\n" + contents + "\n</p>\n";
	}

	template<typename Node>
	std::string ParseInformalTable_(Node informaltable) {
		std::stringstream ss;
		ss << "<table style=\"border:1px solid; border-spacing:0px;\">\n";

		auto tgroup = informaltable->first_node("tgroup");
		if (tgroup == nullptr) {
			std::cout << "informaltable without tgroup: " << _name << std::endl;
			return "";
		}

		// header
		auto thead = tgroup->first_node("thead");
		if (thead != nullptr) {
			for (auto row = thead->first_node("row"); row != nullptr; row = row->next_sibling("row")) {
				ss << "<tr>\n";

				for (auto entry = row->first_node("entry"); entry != nullptr; entry = entry->next_sibling("entry")) {
					ss << "<th style=\"border:1px solid; padding:5px; margin:0px;\"> ";

					std::string contents;
					rapidxml::print(std::back_inserter(contents), *entry);

					contents = rmln(contents);
					ReplaceTags_(contents);

					ss << contents;
					ss << " </th>\n";
				}

				ss << "</tr>\n";
			}
		}

		auto tbody = tgroup->first_node("tbody");
		if (tbody != nullptr) {
			for (auto row = tbody->first_node("row"); row != nullptr; row = row->next_sibling("row")) {
				ss << "<tr>\n";

				for (auto entry = row->first_node("entry"); entry != nullptr; entry = entry->next_sibling("entry")) {
					ss << "<td style=\"border:1px solid; padding:5px; margin:0px;\"> ";

					std::string contents;
					rapidxml::print(std::back_inserter(contents), *entry);

					contents = rmln(contents);
					ReplaceTags_(contents);

					ss << contents;
					ss << " </th>\n";
				}

				ss << "</tr>\n";
			}
		}

		ss << "</table>\n";
		return ss.str();
	}

	std::string _name;
	std::string _purpose;
	std::vector<Prototype> _prototypes;
	std::unordered_map<std::string, std::vector<std::string>> _parameters;
	std::vector<std::string> _description;

};

std::vector<Function> getFunctions(const std::filesystem::path& dir) {
	std::vector<Function> functions;

	for (const auto& path : std::filesystem::directory_iterator(dir)) {
		std::string file = path.path().filename().string();
		if (file.size() > 2 && file[0] == 'g' && file[1] == 'l' && file[2] != '_') {
			functions.emplace_back(path.path());
		}
	}

	std::sort(functions.begin(), functions.end(), [](const Function& a, const Function& b) {
		return a.GetName() < b.GetName();
	});

	return functions;
}

struct vector_hash {
	size_t operator()(const std::vector<std::string>& vec) const {
		if (vec.empty()) {
			return 0;
		}

		return std::hash<std::string>()(vec[0]);
	}
};

void createComment(std::vector<std::string>& lines, const Function& function, const Prototype& prototype) {
	lines.emplace_back("///");
	lines.emplace_back("/// \\brief");

	auto purpose = split(function.GetPurpose(), 76);
	for (const std::string& line : purpose) {
		lines.push_back("/// " + line);
	}

	const auto& description = function.GetDescription();
	if (!description.empty()) {
		lines.emplace_back("///");
		lines.emplace_back("/// \\details");

		for (const auto& part : description) {
			auto partLines = split(part, 76);
			for (const auto& line : partLines) {
				lines.push_back("/// " + line);
			}
		}
	}

	const auto& functionParameters = function.GetParameters();
	std::unordered_map<std::vector<std::string>, std::vector<std::string>, vector_hash> combinedParagraphs;
	std::unordered_set<std::string> seenParameters;

	for (const auto& parameter : prototype.parameters) {
		auto iter = functionParameters.find(parameter.name);
		if (iter != functionParameters.end()) {
			combinedParagraphs[iter->second].push_back(parameter.name);
		} else {
			std::cout << "Could not find parameter description for " << prototype.name << "@" << parameter.name << std::endl;
		}
	}

	for (const auto& parameter : prototype.parameters) {
		if (seenParameters.find(parameter.name) != seenParameters.end()) {
			continue;
		}

		auto iter = functionParameters.find(parameter.name);
		if (iter == functionParameters.end()) {
			continue;
		}

		lines.emplace_back("///");

		std::stringstream ss;
		ss << "\\param";
		bool first = true;

		for (const auto& pname : combinedParagraphs[iter->second]) {
			if (pname.size() > 68) {
				std::cout << "parameter name probably too long: " << prototype.name << "@" << parameter.name;
			}

			if (!first) {
				ss << ',';
			}

			ss << ' ' << pname;
			first = false;
			seenParameters.insert(pname);
		}

		auto pnamePar = split(ss.str(), 76);
		for (const auto& line : pnamePar) {
			lines.push_back("/// " + line);
		}

		for (const auto& paragraph : iter->second) {
			auto par = split(paragraph, 76);
			for (const std::string& line : par) {
				lines.push_back("/// " + line);
			}
		}
	}

	lines.emplace_back("///");
}

std::vector<std::string> createFunctionHeader(const Function& function) {
	std::vector<std::string> lines;

	lines.emplace_back("#ifndef OPENGL_GLWR_H_");
	lines.emplace_back("#error \"Do not include glwr function headers directly, include GL/glwr.h");
	lines.emplace_back("#endif");
	lines.emplace_back();

	for (const auto& prototype : function.GetPrototypes()) {
		if (!prototype.in_gl1) {
			lines.push_back("#undef " + prototype.name);
		}
	}

	for (const auto& prototype : function.GetPrototypes()) {
		if (prototype.in_gl1) {
			lines.emplace_back();
			createComment(lines, function, prototype);

			std::stringstream ss;
			ss << prototype.type << " " << prototype.name << "(";

			bool first = true;
			for (const auto& parameter : prototype.parameters) {
				if (!first) {
					ss << ", ";
				}

				ss << parameter.type << " " << parameter.name;
				first = false;
			}

			ss << ");";
			lines.push_back(ss.str());
		}
	}

	for (const auto& prototype : function.GetPrototypes()) {
		if (gl1.find(prototype.name) == gl1.end()) {
			lines.emplace_back();
			createComment(lines, function, prototype);

			std::stringstream ss;
			ss << "OPENGL_INLINE " << prototype.type << " " << prototype.name << "(";

			bool first = true;
			for (const auto& parameter : prototype.parameters) {
				if (!first) {
					ss << ", ";
				}

				ss << parameter.type << " " << parameter.name;
				first = false;
			}

			ss << ") {";
			lines.push_back(ss.str());

			ss = std::stringstream();
			ss << "\tGLEW_GET_FUN(__glew" << prototype.name.substr(2) << ")(";

			first = true;
			for (const auto& parameter : prototype.parameters) {
				if (!first) {
					ss << ", ";
				}

				ss << parameter.name;
				first = false;
			}

			ss << ");";

			lines.push_back(ss.str());
			lines.emplace_back("}");
		}
	}

	return lines;
}

std::vector<std::string> createHeader(const std::vector<Function>& functions) {
	std::vector<std::string> lines;

	lines.emplace_back("#ifndef OPENGL_GLWR_H_");
	lines.emplace_back("#define OPENGL_GLWR_H_");
	lines.emplace_back();
	lines.emplace_back("#include <GL/glew.h>");
	lines.emplace_back();
	lines.emplace_back("#if defined(__GNUC__) || defined(__clang__)");
	lines.emplace_back("#define OPENGL_INLINE __attribute__((always_inline)) inline");
	lines.emplace_back("#elif defined(_MSC_VER)");
	lines.emplace_back("#define OPENGL_INLINE __forceinline");
	lines.emplace_back("#else");
	lines.emplace_back("#define OPENGL_INLINE inline");
	lines.emplace_back("#endif");
	lines.emplace_back();

	for (const auto& function : functions) {
		lines.push_back("#include \"func/" + function.GetName() + ".h\"");
	}

	lines.emplace_back();
	lines.emplace_back("#endif");

	return lines;
}

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		return -1;
	}

	auto functions = getFunctions(std::filesystem::current_path() / "opengl-refpages" / "gl4");
	auto header = createHeader(functions);

	std::string output = std::string(argv[1]);
	std::ofstream file(output + "/glwr.h");

	for (const auto& line : header) {
		file << line << '\n';
	}

	for (const auto& function : functions) {
		std::ofstream functionFile(output + "/func/" + function.GetName() + ".h");
		auto functionHeader = createFunctionHeader(function);

		for (const auto& line : functionHeader) {
			functionFile << line << '\n';
		}
	}

	return 0;
}
