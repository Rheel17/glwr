/*
 * Copyright (c) 2022 Levi van Rheenen
 */
#ifndef GLWR_OPTIONS_H
#define GLWR_OPTIONS_H

#include <cstdlib>

#define INCLUDE_LINK            0b10000000000
#define INCLUDE_BRIEF           0b01000000000
#define INCLUDE_VERSION         0b00100000000
#define INCLUDE_DESCRIPTION     0b00010000000
#define INCLUDE_EXAMPLES        0b00001000000
#define INCLUDE_NOTES           0b00000100000
#define INCLUDE_PARAMETERS      0b00000010000
#define INCLUDE_ERRORS          0b00000001000
#define INCLUDE_ASSOCIATED_GETS 0b00000000100
#define INCLUDE_SEE_ALSO        0b00000000010
#define INCLUDE_COPYRIGHT       0b00000000001

inline struct includes {
	bool link;
	bool brief;
	bool version;
	bool description;
	bool examples;
	bool notes;
	bool parameters;
	bool errors;
	bool associated_gets;
	bool see_also;
	bool copyright;

	inline void Parse(const char* argv) {
		auto value = strtoul(argv, nullptr, 2);

		link            = (value & INCLUDE_LINK) != 0;
		brief           = (value & INCLUDE_BRIEF) != 0;
		version         = (value & INCLUDE_VERSION) != 0;
		description     = (value & INCLUDE_DESCRIPTION) != 0;
		examples        = (value & INCLUDE_EXAMPLES) != 0;
		notes           = (value & INCLUDE_NOTES) != 0;
		parameters      = (value & INCLUDE_PARAMETERS) != 0;
		errors          = (value & INCLUDE_ERRORS) != 0;
		associated_gets = (value & INCLUDE_ASSOCIATED_GETS) != 0;
		see_also        = (value & INCLUDE_SEE_ALSO) != 0;
		copyright       = (value & INCLUDE_COPYRIGHT) != 0;
	}
} include;

inline bool verbose;

#endif