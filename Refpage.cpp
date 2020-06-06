/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#include "Refpage.h"

Refpage::Refpage(std::istream& input, std::string name) :
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

void Refpage::Set_(const char* name, std::string& str, const char* value) {
	if (str.empty()) {
		str = value;
		return;
	}

	std::cout << "@" << _name << " Duplicate value: " << name << std::endl;
}

void Refpage::Parse_(const Document& doc) {
	auto refentry = doc.first_node("refentry");
	ParseRefentry_(refentry);
	std::cout << "";
}

void Refpage::ParseRefentry_(Node refentry) {
	for (Node n : NodeIterator(refentry)) {
		ParseNode_(n);
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
	for (Node n : NodeIterator(info)) {
		std::string name = n->name();
		if (name == "copyright") {
			impl_copyright& value = _copyright.emplace_back();
			ParseCopyright_(n, value);
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
			Set_("refnamediv.refdescriptor", _refnamediv.refdescriptor, node->value());
		} else if (name == "refname") {
			_refnamediv.refnames.emplace_back(node->value());
		} else if (name == "refpurpose") {
			Set_("refnamediv.refpurpose", _refnamediv.refpurpose, node->value());
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

void Refpage::ParseFuncsynopsis_(Node funcsynopsis, Refpage::impl_refsynopsisdiv& value) {
	for (const auto& [node, name] : NodeNameIterator(funcsynopsis)) {
		if (name == "funcprototype") {
			auto& value2 = value.funcprototypes.emplace_back();
			ParseFuncprototype_(node, value2);
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis." << name << std::endl;
		}
	}
}

void Refpage::ParseFuncprototype_(Node funcprototype, Refpage::impl_funcprototype& value) {
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

void Refpage::ParseFuncdef_(Node funcdef, Refpage::impl_funcdef& value) {
	for (const auto& [node, name] : NodeNameIterator(funcdef)) {
		if (name == "") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef(value)", value.type, funcdef->value());
		} else if (name == "function") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef.function", value.function, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype.function." << name << std::endl;
		}
	}
}

void Refpage::ParseParamdef_(Node paramdef, Refpage::impl_paramdef& value) {
	for (const auto& [node, name] : NodeNameIterator(paramdef)) {
		if (name == "") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.paramdef(value)", value.type, paramdef->value());
		} else if (name == "parameter") {
			Set_("refsynopsisdiv.funcsynopsis.funcprototype.funcdef.parameter", value.parameter, node->value());
		} else {
			std::cout << "@" << _name << " Unknown node: refsynopsisdiv.funcsynopsis.funcprototype.paramdef." << name << std::endl;
		}
	}
}

void Refpage::ParseRefsect1Parameters_(Node refsect1) {

}

void Refpage::ParseRefsect1Parameters2_(Node refsect1) {

}

void Refpage::ParseRefsect1Description_(Node refsect1) {

}

void Refpage::ParseRefsect1Description2_(Node refsect1) {

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
