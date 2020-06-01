/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "util.h"
#include "gl1.h"

#include <rapidxml/rapidxml.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>

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

public:
	explicit Function(const std::filesystem::path& path) {
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
		auto variablelist = findChildNodeChildNode(refentry, "refsect1", "variablelist");

		if (variablelist) {
			for (auto varlistentry = variablelist->first_node("varlistentry");
				 varlistentry != nullptr;
				 varlistentry = varlistentry->next_sibling("varlistentry")) {

				ParseParameter_(varlistentry);
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

			paragraphs.push_back(rmln(para->value()));
		}


		auto term = varlistentry->first_node("term");
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

	std::string _name;
	std::string _purpose;
	std::vector<Prototype> _prototypes;
	std::unordered_map<std::string, std::vector<std::string>> _parameters;

};

std::vector<Function> getFunctions(const std::filesystem::path& dir) {
	std::vector<Function> functions;

	for (const auto& path : std::filesystem::directory_iterator(dir)) {
		std::string file = path.path().filename();
		if (file.size() > 2 && file[0] == 'g' && file[1] == 'l' && file[2] != '_') {
			functions.emplace_back(path.path());
		}
	}

	std::sort(functions.begin(), functions.end(), [](const Function& a, const Function& b) {
		return a.GetName() < b.GetName();
	});

	return functions;
}

void createComment(std::vector<std::string>& lines, const Function& function, const Prototype& prototype) {
	lines.emplace_back("///");
	lines.emplace_back("/// \\brief");

	auto purpose = split(function.GetPurpose(), 76);
	for (const std::string& line : purpose) {
		lines.push_back("/// " + line);
	}

	const auto& functionParameters = function.GetParameters();
	for (const auto& parameter : prototype.parameters) {
		lines.emplace_back("///");
		lines.push_back("/// \\param " + parameter.name);

		bool first = true;
		auto iter = functionParameters.find(parameter.name);
		if (iter != functionParameters.end()) {
			for (const auto& paragraph : functionParameters.find(parameter.name)->second) {
				if (!first) {
					lines.emplace_back("///");
				}

				auto par = split(paragraph, 76);
				for (const std::string& line : par) {
					lines.push_back("/// " + line);
				}

				first = false;
			}
		} else {
			std::cout << "Could not find parameter description for " << prototype.name << "@" << parameter.name << std::endl;
		}
	}

	lines.emplace_back("///");
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
		for (const auto& prototype : function.GetPrototypes()) {
			if (!prototype.in_gl1) {
				lines.push_back("#undef " + prototype.name);
			}
		}
	}

	for (const auto& function : functions) {
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
	}

	for (const auto& function : functions) {
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

	const char* output = argv[1];
	std::ofstream file(output);

	for (const auto& line : header) {
		file << line << '\n';
	}

	return 0;
}
