/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef GLWR_XMLHELPER_H
#define GLWR_XMLHELPER_H

#include <rapidxml/rapidxml_print.hpp>

#include <string_view>

using Document = rapidxml::xml_document<>;
using Node = rapidxml::xml_node<>*;

class NodeIterator {

public:
	class IteratorImpl {

	public:
		explicit IteratorImpl(Node n) :
				_n(n) {}

		inline IteratorImpl operator++() {
			// prefix
			_n = _n->next_sibling();
			return *this;
		}

		inline bool operator!=(const IteratorImpl& iter) {
			return _n != iter._n;
		}

		inline Node operator*() {
			return _n;
		}

		inline Node operator->() {
			return _n;
		}

	private:
		Node _n;

	};

	/* implicit */ NodeIterator(Node n) :
			_n(n) {}

	inline IteratorImpl begin() {
		return IteratorImpl(_n->first_node());
	}

	inline IteratorImpl end() {
		return IteratorImpl(nullptr);
	}

private:
	Node _n;

};

class NodeNameIterator {

public:
	class NodeName {

	public:
		explicit NodeName(Node node) :
				node(node) {

			if (node) {
				name = node->name();
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
			_n = NodeName(_n.node->next_sibling());
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
		return IteratorImpl(_n->first_node());
	}

	inline IteratorImpl end() {
		return IteratorImpl(nullptr);
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

#endif