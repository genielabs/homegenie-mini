/*
	LinkedList.h - V1.1 - Generic LinkedList implementation
	Works better with FIFO, because LIFO will need to
	search the entire List to find the last one;

	For instructions, go to https://github.com/ivanseidel/LinkedList

	Created by Ivan Seidel Gomes, March, 2013.
	Released into the public domain.
*/

#pragma once

#include <stddef.h>
#include <iterator>

template<class T>
struct ListNode
{
	T data;
	ListNode<T> *next;

	explicit ListNode<T>(ListNode<T>* _next = nullptr) : next(_next) {}
	explicit ListNode<T>(const T& _v, ListNode<T>* _next = nullptr) : data(_v), next(_next) {}
};

template <typename T>
class LinkedList{

protected:
	unsigned _size = 0;
	ListNode<T> *root = nullptr;
	ListNode<T>	*last = nullptr;

	// Helps "get" method, by saving last position
	mutable ListNode<T> *lastNodeGot = nullptr;
	mutable unsigned lastIndexGot=0;		// cached node index

	ListNode<T>* getNode(unsigned index) const;

	ListNode<T>* findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &));

	/**
	 * @brief delete last node in a list
	 * @return true if success
	 */
	void _pop();

	/**
	 * @brief delete first node in a list
	 * @return true if success
	 */
	void _shift();

public:
	LinkedList(){};
	LinkedList(unsigned sizeIndex, T _t); //initiate list size and default value
	LinkedList(const LinkedList<T> &rhs) : LinkedList(){ *this = rhs; };	// clone constructor
	virtual ~LinkedList();

	/*
		Returns current size of LinkedList
	*/
	virtual unsigned size() const;
	/*
		Adds a T object in the specified index;
		Unlink and link the LinkedList correcly;
		Increment _size
	*/
	virtual bool add(unsigned index, const T&);
	/*
		Adds a T object in the end of the LinkedList;
		Increment _size;
	*/
	virtual bool add(const T&);
	/*
		Adds a T object in the start of the LinkedList;
		Increment _size;
	*/
	virtual bool unshift(const T&);
	/*
		Set the object at index, with T;
	*/
	virtual bool set(unsigned index, const T&);

	/*
		Remove node at index;
		If index is not reachable, returns false;
		else, deletes node
		Returns a copy of T object from removed node
	*/
	virtual T remove(unsigned index);

	/*
		Unlink and delete node at index;
		NOTE: this will not destroy dyn alloced objects
		like pointer to obj created with 'new'
		use 'delete(remove())' instead or use smart pointers
	*/
	void unlink(unsigned index);

	/*
		Remove last object;
		Returns a copy of T object from removed node
	*/
	virtual T pop();

	/**
	 * @brief 		Remove first object
	 *	Returns a copy of T object from removed node
	 */
	virtual T shift();

	/*
		Get the index'th element on the list;
		Return Element if accessible,
		else, return false;
	*/
	virtual T get(unsigned index) const;

	/*
		Get first element of the list;
		Return Element if accessible,
		else, return false;
	*/
	virtual T head() const;

	/*
		Get last element of the list;
		Return Element if accessible,
		else, return false;
	*/
	virtual T tail() const;

	/*
		Return true if element with specified index exist
		note: this works faster than (uncached) get(index)
	*/
	virtual bool exist(unsigned index) const;

	/**
	 * @brief clear linked list
	 * effectively unlinks all nodes from a chain
	 * NOTE: this will destruct linked nodes not considering stored objects
	 * i.e. it's OK to clear nodes containing trivial objects like int, float, etc...
	 * if node contains pointers to dynamically allocated objects created
	 * with 'new' operator it will result in a mem leak!
	 */
	virtual void clear();

	/*
		Sort the list, given a comparison function
	*/
	virtual void sort(int (*cmp)(T &, T &));


	inline T& operator[](unsigned i) { return getNode(i)->data; }
	inline const T& operator[](const unsigned i) const { return getNode(i)->data; }

	// deep-copy via assign operator
	virtual LinkedList<T> & operator =(const LinkedList<T> &rhs);

	/*
		ConstIterator class
		provides immutable forward iterator for the list
	*/
	struct ConstIterator {
	    using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = const T*;
		using reference         = const T&;

		ConstIterator(ListNode<T> *ptr = nullptr) : m_ptr(ptr) { if (m_ptr != nullptr) m_next_ptr = ptr->next; }

		const reference operator*() const { return m_ptr->data; }
		const pointer operator->() const { return &m_ptr->data; }

		// Prefix increment
		ConstIterator& operator++() { m_ptr = m_next_ptr; m_next_ptr = m_next_ptr ? m_next_ptr->next : nullptr; return *this; }

		// Postfix increment
		ConstIterator operator++(int) { ConstIterator tmp = *this; m_ptr = m_next_ptr; m_next_ptr = m_next_ptr ? m_next_ptr->next : nullptr; return tmp; }

		bool operator== (const ConstIterator& a) const { return m_ptr == a.m_ptr; };
		bool operator!= (const ConstIterator& a) const { return m_ptr != a.m_ptr; };
		//friend bool operator== (const ConstIterator& a, const ConstIterator& b) { return a.m_ptr == b.m_ptr; };
		//friend bool operator!= (const ConstIterator& a, const ConstIterator& b) { return a.m_ptr != b.m_ptr; };

		friend LinkedList<T>;

		protected:
			ListNode<T> *m_ptr;
			ListNode<T> *m_next_ptr = nullptr;
	};

	/*
		Iterator class
		inherits from ConstIterator, provides mutable forward iterator for the list
	*/
	struct Iterator : public ConstIterator {
	    using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = T*;
		using reference         = T&;

		Iterator(ListNode<T> *ptr = nullptr) : ConstIterator(ptr) {}
		Iterator(){}

		reference operator*() { return this->m_ptr->data; }
		pointer operator->() { return &this->m_ptr->data; }
	};

	// iterator methods
	ConstIterator cbegin() const { return ConstIterator(root); }
	ConstIterator cend() const { return ConstIterator(nullptr); }   // same as last->next for non-empty list
	Iterator begin() { return Iterator(root); }
	Iterator end() { return Iterator(nullptr); }                    // same as last->next for non-empty list

};

// D-tor
template<typename T>
LinkedList<T>::~LinkedList(){ clear(); }

/*
	Actualy "logic" coding
*/
template<typename T>
ListNode<T>* LinkedList<T>::getNode(unsigned index) const {
	if (!_size || index >=_size)
		return nullptr;

	if (!index)
		return root;

	if (index == _size-1)
		return last;

	unsigned _pos = 0;
	ListNode<T>* current = root;

	// Check if the node trying to get is
	// ahead or equal to the last one got
	if(lastIndexGot <= index){
		_pos = lastIndexGot;
		current = lastNodeGot;
	}

	while(_pos < index && current){
		current = current->next;
		_pos++;
	}

	// Check if the object index got is the same as the required
	if(_pos == index){
		lastIndexGot = index;
		lastNodeGot = current;
		return current;
	}

	return nullptr;
}

template<typename T>
unsigned LinkedList<T>::size() const {
	return _size;
}

template<typename T>
LinkedList<T>::LinkedList(unsigned sizeIndex, T _t){
	for (unsigned i = 0; i < sizeIndex; i++){
		add(_t);
	}
}

template<typename T>
bool LinkedList<T>::add(unsigned index, const T& _t){
	if(index >= _size)
		return add(_t);

	if(!index)
		return unshift(_t);

	lastIndexGot = index;
	ListNode<T> *_prev = getNode(--index);

	_prev->next = new ListNode<T>(_t, _prev->next);
	lastNodeGot = _prev->next;

	_size++;

	return true;
}

template<typename T>
bool LinkedList<T>::add(const T& _t){
	
	if(root){
		// Already have elements inserted
		last->next = new ListNode<T>(_t);
		last = last->next;
	} else {
		// First element being inserted
		root = new ListNode<T>(_t);
		last = root;
	}

	lastIndexGot = _size;
	lastNodeGot = last;
	++_size;

	return true;
}

template<typename T>
bool LinkedList<T>::unshift(const T& _t){

	if(!_size)
		return add(_t);

	root = new ListNode<T>(_t, root);
	
	_size++;

	lastIndexGot = 0;
	lastNodeGot = root;
	
	return true;
}

template<typename T>
bool LinkedList<T>::set(unsigned index, const T& _t){
	// Check if index position is in bounds
	if(index >= _size)
		return false;

	getNode(index)->data = _t;
	return true;
}

template<typename T>
void LinkedList<T>::_pop(){
	if(!_size)
		return;

	if (_size == 1){
		// Only one element left on the list
		delete(root);
		root = last = lastNodeGot = nullptr;
		lastIndexGot = _size = 0;
		return;
	}

	ListNode<T> *tmp = getNode(_size - 2);
	delete(tmp->next);
	tmp->next = nullptr;
	last = lastNodeGot = tmp;
	lastIndexGot = --_size;
	--lastIndexGot;
}

template<typename T>
T LinkedList<T>::pop(){
	if(!_size)
		return T();

	if(_size > 1){
		ListNode<T> *tmp = getNode(_size - 2);
		T ret(tmp->next->data);
		_pop();
		return ret;
	}

	T ret(root->data);
	_pop();
	return ret;
}

template<typename T>
void LinkedList<T>::_shift(){
	if(!_size)
		return;

	if (_size == 1){
		// Only one element left on the list
		delete(root);
		root = last = lastNodeGot = nullptr;
		lastIndexGot = _size = 0;
		return;
	}

	// drop first node
	ListNode<T> *_next = root->next;
	delete(root);
	root = lastNodeGot = _next;
	lastIndexGot = 0;
	--_size;	
}

template<typename T>
T LinkedList<T>::shift(){
	if(!_size)
		return T();

	T data(root->data);
	_shift();
	return data;
}

template<typename T>
T LinkedList<T>::remove(unsigned index){
	if (index >= _size)
		return T();

	if(!index)
		return shift();
	
	if (index == _size-1)
		return pop();

	T ret(getNode(index)->data);
	unlink(index);
	return ret;
}

template<typename T>
void LinkedList<T>::unlink(unsigned index){
	if (!_size || index >= _size)
		return;

	if(!index)
		return _shift();
	
	if (index == _size-1)
		return _pop();

	ListNode<T> *prev = getNode(--index);
	ListNode<T> *toDelete = prev->next;
	prev->next = prev->next->next;
	delete(toDelete);
	_size--;
	lastIndexGot = index;
	lastNodeGot = prev;
}

template<typename T>
T LinkedList<T>::get(unsigned index) const {
	ListNode<T> *tmp = getNode(index);

	return (tmp ? tmp->data : T());
}

template<typename T>
void LinkedList<T>::clear(){
	while(_size)
		_shift();
}

template<typename T>
void LinkedList<T>::sort(int (*cmp)(T &, T &)){
	if(_size < 2) return; // trivial case;

	for(;;) {	

		ListNode<T> **joinPoint = &root;

		while(*joinPoint) {
			ListNode<T> *a = *joinPoint;
			ListNode<T> *a_end = findEndOfSortedString(a, cmp);
	
			if(!a_end->next	) {
				if(joinPoint == &root) {
					last = a_end;
					lastIndexGot = 0;
					return;
				}
				else {
					break;
				}
			}

			ListNode<T> *b = a_end->next;
			ListNode<T> *b_end = findEndOfSortedString(b, cmp);

			ListNode<T> *tail = b_end->next;

			a_end->next = NULL;
			b_end->next = NULL;

			while(a && b) {
				if(cmp(a->data, b->data) <= 0) {
					*joinPoint = a;
					joinPoint = &a->next;
					a = a->next;	
				}
				else {
					*joinPoint = b;
					joinPoint = &b->next;
					b = b->next;	
				}
			}

			if(a) {
				*joinPoint = a;
				while(a->next) a = a->next;
				a->next = tail;
				joinPoint = &a->next;
			}
			else {
				*joinPoint = b;
				while(b->next) b = b->next;
				b->next = tail;
				joinPoint = &b->next;
			}
		}
	}
}

template<typename T>
ListNode<T>* LinkedList<T>::findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &)) {
	while(p->next && cmp(p->data, p->next->data) <= 0) {
		p = p->next;
	}
	
	return p;
}

template<typename T>
T LinkedList<T>::head() const {
	return _size ? root->data : T();
}

template<typename T>
T LinkedList<T>::tail() const {
	return _size ? last->data : T();
}

template<typename T>
bool LinkedList<T>::exist(unsigned index) const {
	return (index < _size);
}

template<typename T>
LinkedList<T>& LinkedList<T>::operator =(const LinkedList<T>& rhs) {
    clear();
	LinkedList::ConstIterator i(rhs.root);
	while (i != rhs.cend()){
		add(*i);
		++i;
	}

	//for (const auto& i :  std::as_const(rhs))		// std::as_const req c++17
	//	add(*i);

    return *this;
}
