/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include <ctre.hpp>

#include <cstring>
#include <fstream>

#include "Refpage.h"

constexpr static auto glfwHeaderHead = R"(#ifndef OPENGL_GLWR_H_
#define OPENGL_GLWR_H_

#include <GL/glew.h>

#if defined(__GNUC__) || defined(__clang__)
#define GLWR_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define GLWR_INLINE __forceinline
#else
#define GLWR_INLINE inline
#endif

using DEBUGPROC = GLDEBUGPROC;

)";

constexpr static auto glfwHeaderTail = R"(
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
	file << glfwHeaderHead;

	for (const auto& declarationName : declarationNames) {
		file << "#include \"func/" << declarationName << ".h\"" << std::endl;
	}

	file << glfwHeaderTail;
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		return -1;
	}

	auto gl4 = std::filesystem::current_path() / "opengl-refpages" / "gl4";
	auto dir = std::filesystem::path(argv[1]);

	include.Parse(argv[2]);
	verbose = std::strcmp(argv[3], "ON") == 0;

	std::vector<std::string> functions = getFunctionFiles(gl4);
	std::vector<std::string> declarationNames;

	for (const auto& function : functions) {
		std::string name(function.begin(), function.end() - 4);
		declarationNames.push_back(name);

		if (verbose) {
			std::cout << "Generating " << name << ".h" << std::endl;
		}

		std::ifstream file(gl4 / function, std::ios::binary);
		Refpage refpage(gl4, file, name);

		std::filesystem::path functionHeaderPath = dir / "func" / (name + ".h");
		std::ofstream functionHeader(functionHeaderPath.string());
		refpage.GenerateHeader(functionHeader);
	}

	writeGlwrHeader(dir / "glwr.h", declarationNames);
	return 0;
}
