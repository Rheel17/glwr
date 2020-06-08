/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_REFPAGE_H
#define GLWR_REFPAGE_H

#include <iostream>
#include <vector>
#include <optional>
#include <filesystem>

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
		std::optional<std::string> refdescriptor;
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

	struct impl_abstract_text {
		std::vector<std::string> elements;
	};

	struct impl_listitem {
		impl_abstract_text contents;
	};

	struct impl_varlistentry {
		std::vector<std::string> terms;
		std::vector<impl_listitem> listitems;
	};

	struct impl_refsect_parameters {
		std::optional<std::string> impl_for_function;
		std::vector<impl_varlistentry> varlistentries;
	};

	struct impl_refsect_parameters_2 : public impl_refsect_parameters {};

	struct impl_refsect_description {
		std::optional<std::string> impl_for_function;
	};

	struct impl_refsect_description_2 : public impl_refsect_description {};

	struct impl_refsect_examples {

	};

	struct impl_refsect_notes {

	};

	struct impl_refsect_errors {

	};

	struct impl_refsect_associatedgets {

	};

	struct impl_refsect_versions {

	};

	struct impl_refsect_seealso {

	};

	struct impl_refsect_copyright {

	};

	Refpage(std::filesystem::path dir, std::istream& input, std::string name);

	void GenerateHeader(std::ostream& output);

private:
	void Set_(const char* node, std::string& str, const char* value);
	Node GetOnlyChild_(Node node, const std::string_view& name, std::string_view child);

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

	void ParseAbstractText_(Node parent, impl_abstract_text& text);
	std::string ParseAbstractTextNode_(Node node, const std::string_view& name);
	std::string ParseValueNode_(Node node, const std::string_view& name, const char* begin, const char* end);
	std::string ParseInclude_(Node include);
	std::string ParsePara_(Node para);
	std::string ParseEmphasis_(Node emphasis);
	std::string ParseCiterefentry_(Node citerefentry);
	std::string ParseInformaltable_(Node informaltable);
	void ParseInformaltableRows_(Node node, const std::string_view& name, std::stringstream& ss);
	void ParseInformaltableRow_(Node row, bool head, std::stringstream& ss);
	std::string ParseInlineequation_(Node inlineequation);
	std::string ParseAbstractMathNode_(Node node, const std::string_view& name);
	std::string ParseMathValue_(Node parent);
	std::string ParseMmlmi_(Node mmlmi);
	std::string ParseMmlmn_(Node mmlmn);
	std::string ParseMmlmo_(Node mmlmo);
	std::string ParseMmlmfenced_(Node mmlmfenced);
	std::string ParseMmlmrow_(Node mmlmfenced);
	std::string ParseMmlmsup_(Node mmlmsup);

	void ParseVariablelist_(Node variablelist, impl_refsect_parameters& parameters);
	void ParseVarlistentry_(Node varlistentry, impl_varlistentry& value);
	void ParseTerm_(Node term, impl_varlistentry& varlistentry);

	std::filesystem::path _dir;
	std::string _name;

	std::vector<impl_copyright> _copyrights;
	impl_refmeta _refmeta;
	impl_refnamediv _refnamediv;
	impl_refsynopsisdiv _refsynopsisdiv;

	std::optional<impl_refsect_parameters> _refsect_parameters;
	std::optional<impl_refsect_parameters_2> _refsect_parameters_2;
	std::optional<impl_refsect_description> _refsect_description;
	std::optional<impl_refsect_description_2> _refsect_description_2;
	std::optional<impl_refsect_examples> _refsect_examples;
	std::optional<impl_refsect_notes> _refsect_notes;
	std::optional<impl_refsect_errors> _refsect_errors;
	std::optional<impl_refsect_associatedgets> _refsect_associatedgets;
	std::optional<impl_refsect_versions> _refsect_versions;
	std::optional<impl_refsect_seealso> _refsect_seealso;
	std::optional<impl_refsect_copyright> _refsect_copyright;

};

#endif
