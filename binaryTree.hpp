#ifndef BINARYTREE_HPP
#define BINARYTREE_HPP

#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "PointerIntPair.h"

using namespace std::string_literals;

template <typename T>
class BinaryTree {
public:
	class Node;
	using NodePtr = llvm::PointerIntPair<Node*, 1, bool>;

	BinaryTree() = default;
	~BinaryTree();
	BinaryTree(BinaryTree&&) noexcept = default;
	BinaryTree& operator=(BinaryTree&&) noexcept = default;
	// BinaryTree(const BinaryTree&);
	// BinaryTree& operator=(const BinaryTree&);

	void swap(BinaryTree& other);

	void insert(const T&);
	void insert(T&&);
	const T* search(const T&) const noexcept;
	const T* searchRecursive(const T&) const noexcept;
	void prettyPrint() const noexcept;
	void prettyPrint(const Node*, const std::string&, const std::string&) const
		noexcept;

	void inorder(const std::function<void(const T&)>&) const noexcept;
	void preorder(const std::function<void(const T&)>&) const noexcept;
	void postorder(const std::function<void(const T&)>&) const noexcept;

	const T& minimum() const;
	const T& maximum() const;
	unsigned size() const noexcept;
	unsigned depth() const noexcept;

private:
	void insertLeft(NodePtr&, Node*, Node*, unsigned = 1) noexcept;
	void insertRight(NodePtr&, Node*, Node*, unsigned = 1) noexcept;
	static const T* searchRecursive(const Node*, const T&) noexcept;
	void inorder(const Node*, const std::function<void(const T&)>&) const
		noexcept;
	void preorder(const Node*, const std::function<void(const T&)>&) const
		noexcept;
	void postorder(const Node*, const std::function<void(const T&)>&) const
		noexcept;
	static void deleter(Node*);

	NodePtr m_root{nullptr};
	unsigned m_size{0};
	unsigned m_depth{0};
};

////////////////////////////////////////

template <typename T>
class BinaryTree<T>::Node {
	friend BinaryTree<T>;

	enum : bool { THREAD = 0, NO_THREAD };

public:
	Node(const T value) : m_value{value}
	{
	}
	~Node() = default;
	Node(const Node&) = default;
	Node& operator=(const Node&) = default;
	Node(Node&&) = default;
	Node& operator=(Node&&) = default;

	const T& value() const noexcept;
	Node* leftChild() const noexcept;
	Node* rightChild() const noexcept;
	Node* previous() const noexcept;
	Node* next() const noexcept;

private:
	NodePtr& leftPointer() noexcept;
	NodePtr& rightPointer() noexcept;

	const T m_value{};
	NodePtr m_leftChild{nullptr};
	NodePtr m_rightChild{nullptr};
};

////////////////////////////////////////
// Node implementation

template <typename T>
inline const T& BinaryTree<T>::Node::value() const noexcept
{
	return m_value;
}

template <typename T>
inline typename BinaryTree<T>::Node* BinaryTree<T>::Node::leftChild() const
	noexcept
{
	if (m_leftChild.getInt() == THREAD)
		return nullptr;
	return m_leftChild.getPointer();
}

template <typename T>
inline typename BinaryTree<T>::Node* BinaryTree<T>::Node::rightChild() const
	noexcept
{
	if (m_rightChild.getInt() == THREAD)
		return nullptr;
	return m_rightChild.getPointer();
}

template <typename T>
inline typename BinaryTree<T>::Node* BinaryTree<T>::Node::previous() const
	noexcept
{
	if (m_leftChild.getInt() == Node::THREAD)
		return m_leftChild.getPointer();

	auto current{m_leftChild.getPointer()};
	while (current->rightChild()) {
		current = current->rightChild();
	}
	return current;
}

template <typename T>
inline typename BinaryTree<T>::Node* BinaryTree<T>::Node::next() const noexcept
{
	if (m_rightChild.getInt() == Node::THREAD)
		return m_rightChild.getPointer();

	auto current{m_rightChild.getPointer()};
	while (current->leftChild()) {
		current = current->leftChild();
	}
	return current;
}

template <typename T>
inline typename BinaryTree<T>::NodePtr&
BinaryTree<T>::Node::leftPointer() noexcept
{
	return m_leftChild;
}

template <typename T>
inline typename BinaryTree<T>::NodePtr&
BinaryTree<T>::Node::rightPointer() noexcept
{
	return m_rightChild;
}

////////////////////////////////////////
// BinaryTree implementation

template <typename T>
BinaryTree<T>::~BinaryTree()
{
	deleter(m_root.getPointer());
}

template <typename T>
void BinaryTree<T>::deleter(Node* const node)
{
	if (!node)
		return;
	deleter(node->leftChild());
	deleter(node->rightChild());
	delete node;
}

template <typename T>
void BinaryTree<T>::swap(BinaryTree& other)
{
	using std::swap; // not needed now, just in case
	swap(m_root, other.m_root);
	swap(m_size, other.m_size);
	swap(m_depth, other.m_depth);
}

template <typename T>
void BinaryTree<T>::insert(const T& x)
{
	auto tmp{new Node(x)};
	insertLeft(m_root, tmp, nullptr);
}

template <typename T>
void BinaryTree<T>::insert(T&& x)
{
	auto tmp{new Node(std::forward<T>(x))};
	insertLeft(m_root, tmp, nullptr);
}

template <typename T>
const T* BinaryTree<T>::search(const T& x) const noexcept
{
	auto tmp{m_root.getPointer()};
	while (tmp != nullptr) {
		if (tmp->value() == x)
			return &tmp->value();
		if (tmp->value() > x)
			tmp = tmp->leftChild();
		else
			tmp = tmp->rightChild();
	}

	return nullptr;
}

template <typename T>
inline const T* BinaryTree<T>::searchRecursive(const T& x) const noexcept
{
	return searchRecursive(m_root.getPointer(), x);
}

template <typename T>
const T*
BinaryTree<T>::searchRecursive(const Node* const node, const T& x) noexcept
{
	if (!node)
		return nullptr;
	if (node->value() == x)
		return &node->value();
	if (node->value() > x)
		return searchRecursive(node->leftChild(), x);
	return searchRecursive(node->rightChild(), x);
}

template <typename T>
void BinaryTree<T>::prettyPrint() const noexcept
{
	std::string tmp{""};
	prettyPrint(m_root.getPointer(), ""s, tmp);
}

template <typename T>
void BinaryTree<T>::prettyPrint(
	const Node* const node,
	const std::string& sp,
	const std::string& sn) const noexcept
{
#if 1
	static const std::string cc{"    "};
	static const std::string cr{"┌───"};
	static const std::string cp{"│   "};
	static const std::string cl{"└───"};
#else
	static const std::string cc{"   "};
	static const std::string cr{",--"};
	static const std::string cp{"|  "};
	static const std::string cl{"`--"};
#endif

	if (!node)
		return;

	std::string s;

	s = sp;
	if (&sn == &cr) {
		s = s.substr(0, s.length() - cp.length());
		s += cc;
	}
	prettyPrint(node->rightChild(), s + cp, cr);

	s = sp.substr(0, sp.length() - cp.length());
	std::cout << "\033[2;34m" << s << sn << "\033[22;39m" << node->value()
			  << std::endl;

	s = sp;
	if (&sn == &cl) {
		s = s.substr(0, s.length() - cp.length());
		s += cc;
	}
	prettyPrint(node->leftChild(), s + cp, cl);
}

template <typename T>
inline void BinaryTree<T>::inorder(const std::function<void(const T&)>& f) const
	noexcept
{
	inorder(m_root.getPointer(), f);
}

template <typename T>
void BinaryTree<T>::inorder(
	const Node* const node,
	const std::function<void(const T&)>& f) const noexcept
{
	if (!node)
		return;
	inorder(node->leftChild(), f);
	f(node->value());
	inorder(node->rightChild(), f);
}

template <typename T>
inline void
BinaryTree<T>::preorder(const std::function<void(const T&)>& f) const noexcept
{
	preorder(m_root, f);
}

template <typename T>
void BinaryTree<T>::preorder(
	const Node* const node,
	const std::function<void(const T&)>& f) const noexcept
{
	if (!node)
		return;
	f(node->value());
	preorder(node->leftChild(), f);
	preorder(node->rightChild(), f);
}

template <typename T>
inline void
BinaryTree<T>::postorder(const std::function<void(const T&)>& f) const noexcept
{
	postorder(m_root, f);
}

template <typename T>
void BinaryTree<T>::postorder(
	const Node* const node,
	const std::function<void(const T&)>& f) const noexcept
{
	if (!node)
		return;
	postorder(node->leftChild(), f);
	postorder(node->rightChild(), f);
	f(node->value());
}

template <typename T>
const T& BinaryTree<T>::minimum() const
{
	if (!m_root.getPointer())
		throw std::out_of_range("Drzewo puste");

	auto tmp{m_root.getPointer()};

	while (tmp->leftChild())
		tmp = tmp->leftChild();

	return tmp->value();
}

template <typename T>
const T& BinaryTree<T>::maximum() const
{
	if (!m_root.getPointer())
		throw std::out_of_range("Drzewo puste");

	auto tmp{m_root.getPointer()};

	while (tmp->rightChild())
		tmp = tmp->rightChild();

	return tmp->value();
}

template <typename T>
inline unsigned BinaryTree<T>::size() const noexcept
{
	return m_size;
}

template <typename T>
inline unsigned BinaryTree<T>::depth() const noexcept
{
	return m_depth;
}

template <typename T>
void BinaryTree<T>::insertLeft(
	NodePtr& parent,
	Node* const newNode,
	Node* const previous,
	unsigned level) noexcept
{
	// if (parent.getPointer() == nullptr) {
	if (parent.getInt() == Node::THREAD) {
		++m_size;
		if (level > m_depth)
			m_depth = level;
		newNode->m_leftChild = parent;
		newNode->m_rightChild.setPointerAndInt(previous, Node::THREAD);
		parent.setPointerAndInt(newNode, Node::NO_THREAD);
		return;
	}
	if (parent.getPointer()->value() > newNode->value()) {
		insertLeft(
			parent.getPointer()->leftPointer(),
			newNode,
			parent.getPointer(),
			level + 1);
		return;
	}
	insertRight(
		parent.getPointer()->rightPointer(),
		newNode,
		parent.getPointer(),
		level + 1);
}

template <typename T>
void BinaryTree<T>::insertRight(
	NodePtr& parent,
	Node* const newNode,
	Node* const next,
	unsigned level) noexcept
{
	// if (parent.getPointer() == nullptr) {
	if (parent.getInt() == Node::THREAD) {
		++m_size;
		if (level > m_depth)
			m_depth = level;
		newNode->m_rightChild = parent;
		newNode->m_leftChild.setPointerAndInt(next, Node::THREAD);
		parent.setPointerAndInt(newNode, Node::NO_THREAD);
		return;
	}
	if (parent.getPointer()->value() > newNode->value()) {
		insertLeft(
			parent.getPointer()->leftPointer(),
			newNode,
			parent.getPointer(),
			level + 1);
		return;
	}
	insertRight(
		parent.getPointer()->rightPointer(),
		newNode,
		parent.getPointer(),
		level + 1);
}

////////////////////////////////////////

#endif /* BINARYTREE_HPP */
