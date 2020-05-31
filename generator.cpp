/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "download.h"
#include "util.h"
#include "gl1.h"

#include <sstream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

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
	// static constexpr auto _base_url = "https://raw.githubusercontent.com/KhronosGroup/OpenGL-Refpages/master/gl4/";
	static constexpr auto _base_url = "file:///D:/OpenGL-Refpages-master/gl4/";
	static constexpr auto _extension = ".xml";

public:
	explicit Function(const std::string& name) {
		Parse_(download(_base_url + name + _extension).AsXml());
	}

	const std::string& GetPurpose() const {
		return _purpose;
	}
	const std::vector<Prototype>& GetPrototypes() const {
		return _prototypes;
	}
	const std::unordered_map<std::string, std::string>& GetParameters() const {
		return _parameters;
	}

private:
	void Parse_(std::unique_ptr<XmlDocument> doc) {
		auto refentry = doc->first_node("refentry");

		// read the purpose
		auto refnamediv = refentry->first_node("refnamediv");
		auto refpurpose = refnamediv->first_node("refpurpose");
		_purpose = refpurpose->value();

		// read the prototype(s)
		auto refsynopsisdiv = refentry->first_node("refsynopsisdiv");
		auto funcsynopsis = refsynopsisdiv->first_node("funcsynopsis");

		for (auto funcprototype = funcsynopsis->first_node("funcprototype");
			 funcprototype != nullptr;
			 funcprototype = funcprototype->next_sibling("funcprototype")) {

			_prototypes.push_back(ParsePrototype_(funcprototype));
		}

		// read the parameter(s)
		auto variablelist = refentry->first_node("variablelist");

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
		auto term = varlistentry->first_node("term");
		std::vector<std::string> parameters;

		for (auto parameter = term->first_node("parameter");
			 parameter != nullptr;
			 parameter = parameter->next_sibling("parameter")) {

			parameters.push_back(parameter->value());
		}


	}

	std::string _purpose;
	std::vector<Prototype> _prototypes;
	std::unordered_map<std::string, std::string> _parameters;

};

template<typename Tree>
std::vector<Function> getFunctions(const Tree& dir) {
	std::vector<Function> functions;

	for (const auto& file : dir) {
		std::string path = file["path"];
		if (path.size() > 2 && path[0] == 'g' && path[1] == 'l' && path[2] != '_') {
			functions.emplace_back(path.substr(0, path.size() - 4));
		}
	}

	return functions;
}

template<>
std::vector<Function> getFunctions<fs::path>(const fs::path& dir) {
	std::vector<Function> functions;

	for (const auto& file : fs::directory_iterator(dir)) {
		std::string path = file.path().filename().string();
		if (path.size() > 2 && path[0] == 'g' && path[1] == 'l' && path[2] != '_') {
			functions.emplace_back(path.substr(0, path.size() - 4));
		}
	}

	return functions;
}

void createComment(std::vector<std::string>& lines, const Function& function, const Prototype& prototype) {

}

std::vector<std::string> createHeader(const std::vector<Function>& functions) {
	std::vector<std::string> lines;

	lines.emplace_back("#ifndef OPENGL_OPENGL_H_");
	lines.emplace_back("#define OPENGL_OPENGL_H_");
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

int main(int argc, char *argv[]) {
	curl_global_init(CURL_GLOBAL_ALL);

	// auto commits = download("https://api.github.com/repos/KhronosGroup/OpenGL-Refpages/commits").AsJson();
	// auto lastCommit = commits[0]["sha"].get<std::string>();
	//
	// auto treeCommit = download("https://api.github.com/repos/KhronosGroup/OpenGL-Refpages/git/trees/" + lastCommit).AsJson();
	// auto tree = treeCommit["tree"];
	// std::string dirPath;
	//
	// for (const auto& file : tree) {
	// 	if (file["path"].get<std::string>() == "gl4") {
	// 		dirPath = file["url"];
	// 		break;
	// 	}
	// }
	//
	// auto dir = download(dirPath).AsJson();
	// auto dirTree = dir["tree"];
	auto functions = getFunctions(/*dirTree*/ fs::path("D:\\OpenGL-Refpages-master\\gl4"));
	auto header = createHeader(functions);

	const char *output = "opengl.h";
	if (argc > 1) {
		output = argv[1];
	}

	std::ofstream file(output);
	for (const auto& line : header) {
		file << line << '\n';
	}

	curl_global_cleanup();
	return 0;
}
