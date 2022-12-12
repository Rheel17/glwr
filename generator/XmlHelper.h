/*
 * Copyright (c) 2022 Levi van Rheenen
 */
#ifndef GLWR_XMLHELPER_H
#define GLWR_XMLHELPER_H

#include <pugixml.hpp>

#include <string_view>

using Document = pugi::xml_document;
using Node = pugi::xml_node;
using Attribute = pugi::xml_attribute;

class NodeNameIterator {

public:
	class NodeName {

	public:
		explicit NodeName(Node node) :
				node(node) {

			if (node) {
				name = node.name();
			}
		}

		template<std::size_t I>
		const auto& get() const {
			if constexpr(I == 0) {
				return node;
			} else if constexpr (I == 1) {
				return name;
			}
		}

		Node node;
		std::string_view name;

	};

	class IteratorImpl {

	public:
		explicit IteratorImpl(Node n) :
				_n(n) {}

		inline IteratorImpl operator++() {
			// prefix
			_n = NodeName(_n.node.next_sibling());
			return *this;
		}

		inline bool operator!=(const IteratorImpl& iter) const {
			return _n.node != iter._n.node;
		}

		inline NodeName& operator*() {
			return _n;
		}

		inline NodeName* operator->() {
			return &_n;
		}

	private:
		NodeName _n;

	};

	/* implicit */ NodeNameIterator(Node n) :
			_n(n) {}

	inline IteratorImpl begin() {
		return IteratorImpl(_n.first_child());
	}

	inline IteratorImpl end() {
		return IteratorImpl(Node());
	}

private:
	Node _n;

};

namespace std {

template<>
struct tuple_size<NodeNameIterator::NodeName> :
		std::integral_constant<size_t, 2> { };

template<>
struct tuple_element<0, NodeNameIterator::NodeName> {
	using type = Node;
};

template<>
struct tuple_element<1, NodeNameIterator::NodeName> {
	using type = std::string_view;
};

}

inline bool hasDescendant(Node node, std::string_view descendantName) {
	for (const auto& [nd, nm] : NodeNameIterator(node)) {
		if (nm == descendantName || hasDescendant(nd, descendantName)) {
			return true;
		}
	}

	return false;
}

inline Node firstChild(Node parent, std::string_view childName) {
	for (const auto& [child, name] : NodeNameIterator(parent)) {
		if (name == childName) {
			return child;
		}
	}

	return Node();
}

inline Attribute firstAttribute(Node node, std::string_view attributeName) {
	for (auto attribute : node.attributes()) {
		if (attribute.name() == attributeName) {
			return attribute;
		}
	}

	return Attribute();
}

#endif