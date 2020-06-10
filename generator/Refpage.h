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
#include "Options.h"

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
		impl_listitem listitem;
	};

	struct impl_refsect_parameters {
		std::optional<std::string> impl_for_function;
		std::vector<impl_varlistentry> varlistentries;
	};

	struct impl_refsect_parameters_2 : public impl_refsect_parameters {};

	struct impl_refsect_description {
		std::optional<std::string> impl_for_function;
		impl_abstract_text contents;
	};

	struct impl_refsect_description_2 : public impl_refsect_description {};

	struct impl_refsect_examples {
		impl_abstract_text contents;
	};

	struct impl_refsect_notes {
		impl_abstract_text contents;
	};

	struct impl_refsect_errors {
		impl_abstract_text contents;
	};

	struct impl_refsect_associatedgets {
		impl_abstract_text contents;
	};

	struct impl_refsect_versions {
		std::unordered_map<std::string, std::string> versions;
	};

	struct impl_refsect_seealso {
		impl_abstract_text contents;
	};

	struct impl_refsect_copyright {
		impl_abstract_text contents;
	};

	Refpage(std::filesystem::path dir, std::istream& input, std::string name);

	void GenerateHeader(std::ostream& output) const;

private:
	void Set_(const char* node, std::string& str, std::string_view value);
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
	bool ParseParamdef_(Node paramdef, impl_paramdef& value);

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
	std::string ParseValueNode_(Node node, const std::string_view& name, const std::string& begin, const std::string& end);
	std::string ParseInclude_(Node include);
	std::string ParsePara_(Node para);
	std::string ParseText_(Node node);
	std::string ParseEmphasis_(Node emphasis);
	std::string ParseTrademark_(Node trademark);
	std::string ParseCiterefentry_(Node citerefentry);
	std::string ParseLink_(Node link);
	std::string ParseInformaltable_(Node informaltable);
	std::string ParseTable_(Node table);
	void ParseTableGroup_(Node tgroup, std::stringstream& ss);
	void ParseTableRows_(Node node, const std::string_view& name, std::stringstream& ss);
	void ParseInformaltableRow_(Node row, bool head, std::stringstream& ss);
	std::string ParseProgramlisting_(Node programlisting);
	std::string ParseItemizedlist_(Node itemizedlist);
	std::string ParseVariablelistGlosslist_(Node variablelist);
	std::string ParseVarlistentryGlossentry_(Node varlistentry);
	std::string ParseInlineequation_(Node inlineequation);
	std::string ParseInformalequation_(Node inlineequation);

	std::string ParseAbstractMathNode_(Node node, const std::string_view& name);
	std::string ParseMathValue_(Node parent);
	std::string ParseMmlmi_(Node mmlmi);
	std::string ParseMmlmn_(Node mmlmn);
	std::string ParseMmlmo_(Node mmlmo);
	std::string ParseMmlmtext_(Node mmlmtext);
	std::string ParseMmlmfenced_(Node mmlmfenced);
	std::string ParseMmlmrow_(Node mmlmrow);
	std::string ParseMmlmsup_(Node mmlmsup);
	std::string ParseMmlmsub_(Node mmlmsub);
	std::string ParseMmlmfrac_(Node mmlmfrac);
	std::string ParseMmlmtable_(Node mmlmtable);
	std::string ParseMmlmtr_(Node mmlmtr);
	std::string ParseMmlmtd_(Node mmlmtd);
	static std::string ParseMmlmspace_(Node mmlmspace);

	std::string ParseLaTeX_(std::string_view type, std::string_view input);
	std::string ParseInnerLaTeX_(std::string_view input);

	void ParseParameters_(Node refsect1, impl_refsect_parameters& parameters);
	void ParseVariablelist_(Node variablelist, impl_refsect_parameters& parameters);
	void ParseVarlistentry_(Node varlistentry, impl_varlistentry& value);
	void ParseTerm_(Node term, impl_varlistentry& varlistentry);
	void ParseDescription_(Node refsect1, impl_refsect_description& description);

	void GenerateHeader_(std::ostream& output, const impl_funcprototype& prototype) const;
	void GenerateComments_(std::ostream& output, const impl_funcprototype& prototype) const;
	void GenerateText_(std::ostream& output, const impl_abstract_text& text) const;
	void GenerateText_(std::ostream& output, std::string_view text) const;

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

private:
	static bool PrototypeHasParameter_(const impl_funcprototype& prototype, std::string_view param);

};

#endif
