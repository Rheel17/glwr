/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include <ctre.hpp>

#include <fstream>

#include "Refpage.h"

constexpr static auto glfw_header_head = R"(#ifndef OPENGL_GLWR_H_
#define OPENGL_GLWR_H_

#include <GL/glew.h>

#if defined(__GNUC__) || defined(__clang__)
#define GLWR_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define GLWR_INLINE __forceinline
#else
#define GLWR_INLINE inline
#endif

)";

constexpr static auto glfw_header_tail = R"(
#endif
)";

std::vector<std::string> getFunctionFiles(const std::filesystem::path& dir) {
	std::vector<std::string> functionFiles;

	for (const auto& p : std::filesystem::directory_iterator(dir)) {
		std::string filename = p.path().filename().string();
		if (ctre::match<"gl[A-Z]\\w*\\.xml">(filename)) {
			functionFiles.push_back(std::move(filename));
		}
	}

	// on some systems the directory_iterator doesn't sort by name, so do it
	// manually here.
	std::sort(functionFiles.begin(), functionFiles.end());

	return functionFiles;
}

void writeGlwrHeader(const std::filesystem::path& path, const std::vector<std::string>& declarationNames) {
	std::ofstream file(path.string());
	file << glfw_header_head;

	for (const auto& declarationName : declarationNames) {
		file << "#include \"func/" << declarationName << ".h\"" << std::endl;
	}

	file << glfw_header_tail;
}

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		return -1;
	}

	auto gl4 = std::filesystem::current_path() / "opengl-refpages" / "gl4";
	auto include = std::filesystem::path(argv[1]);

	std::vector<std::string> functions = getFunctionFiles(gl4);
	std::vector<std::string> declarationNames;

	for (const auto& function : functions) {
		std::string name(function.begin(), function.end() - 4);
		declarationNames.push_back(name);

		std::ifstream file(gl4 / function, std::ios::binary);
		Refpage refpage(gl4, file, name);

		std::filesystem::path functionHeaderPath = include / "func" / (name + ".h");
		std::ofstream functionHeader(functionHeaderPath.string());
		refpage.GenerateHeader(functionHeader);
	}

	writeGlwrHeader(include / "glwr.h", declarationNames);
	return 0;
}
