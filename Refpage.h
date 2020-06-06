/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_REFPAGE_H
#define GLWR_REFPAGE_H

#include <iostream>
#include <vector>

#include "XmlHelper.h"

class Refpage {

public:
	struct impl_copyright {
		std::string year;
		std::string holder;
	};

	struct impl_refmeta {
		std::string refentrytitle;
		std::string manvolnum;
	};

	struct impl_refnamediv {
		std::string refdescriptor;
		std::vector<std::string> refnames;
		std::string refpurpose;
	};

	struct impl_paramdef {
		std::string type;
		std::string parameter;
	};

	struct impl_funcdef {
		std::string type;
		std::string function;
	};

	struct impl_funcprototype {
		impl_funcdef funcdef;
		std::vector<impl_paramdef> paramdefs;
	};

	struct impl_refsynopsisdiv {
		std::vector<impl_funcprototype> funcprototypes;
	};

	struct impl_refsect1_parameters {

	};

	explicit Refpage(std::istream& input, std::string name);

private:
	void Set_(const char* node, std::string& str, const char* value);

	void Parse_(const Document& doc);
	void ParseRefentry_(Node refentry);
	void ParseNode_(Node node);

	void ParseInfo_(Node info);
	void ParseRefmeta_(Node refmeta);
	void ParseRefnamediv_(Node refnamediv);
	void ParseRefsynopsisdiv_(Node refsynopsisdiv);
	void ParseRefsect1_(Node refsect1);

	void ParseCopyright_(Node copyright, impl_copyright& value);
	void ParseFuncsynopsis_(Node funcsynopsis, impl_refsynopsisdiv& value);
	void ParseFuncprototype_(Node funcprototype, impl_funcprototype& value);
	void ParseFuncdef_(Node funcdef, impl_funcdef& value);
	void ParseParamdef_(Node paramdef, impl_paramdef& value);

	void ParseRefsect1Parameters_(Node refsect1);
	void ParseRefsect1Parameters2_(Node refsect1);
	void ParseRefsect1Description_(Node refsect1);
	void ParseRefsect1Description2_(Node refsect1);
	void ParseRefsect1Examples_(Node refsect1);
	void ParseRefsect1Notes_(Node refsect1);
	void ParseRefsect1Errors_(Node refsect1);
	void ParseRefsect1Associatedgets_(Node refsect1);
	void ParseRefsect1Versions_(Node refsect1);
	void ParseRefsect1Seealso_(Node refsect1);
	void ParseRefsect1Copyright_(Node refsect1);

	std::string _name;

	std::vector<impl_copyright> _copyright;
	impl_refmeta _refmeta;
	impl_refnamediv _refnamediv;
	impl_refsynopsisdiv _refsynopsisdiv;

};

#endif
