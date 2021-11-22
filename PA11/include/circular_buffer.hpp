#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <array>
#include <exception>
#include <type_traits>

// Adaptor which extends an array-like class to act as a circular buffer
// NOTE: In order for ranged iteration to work, there must be an unused cell between
// 	the start and end, thus if the backing array can store N elements, the circular buffer can only store N - 1
template<typename T, typename Container = std::array<T, 10>>
class circular_buffer : public Container {
	// Template metaprogramming checking if a type has a function called capacity
	template<typename, typename _T>
	struct has_capacity { static_assert(std::integral_constant<_T, false>::value, "Second template parameter needs to be of function type."); };
	template<typename C, typename Ret, typename... Args>
	struct has_capacity<C, Ret(Args...)> {
	private:
	    template<typename _T> static constexpr auto check(_T*) -> typename std::is_same<decltype( std::declval<_T>().capacity( std::declval<Args>()... ) ), Ret>::type;
	    template<typename> static constexpr std::false_type check(...);
	    typedef decltype(check<C>(0)) type;
	public:
	    static constexpr bool value = type::value;
	};

	// Template metaprogramming checking if a type has a function called get callocator
	template<typename, typename _T>
	struct has_get_allocator { static_assert(std::integral_constant<_T, false>::value, "Second template parameter needs to be of function type."); };
	template<typename C, typename Ret, typename... Args>
	struct has_get_allocator<C, Ret(Args...)> {
	private:
	    template<typename _T> static constexpr auto check(_T*) -> typename std::is_same<decltype( std::declval<_T>().get_allocator( std::declval<Args>()... ) ), Ret>::type;
	    template<typename> static constexpr std::false_type check(...);
	    typedef decltype(check<C>(0)) type;
	public:
	    static constexpr bool value = type::value;
	};

public:
	// Types brought in from the backing container
	using container_type = Container;
	using value_type = typename Container::value_type;
	using size_type = typename Container::size_type;
	using difference_type = typename Container::difference_type;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	using pointer = typename Container::pointer;
	using const_pointer = typename Container::const_pointer;

public:
	// Class defining a special kind of bidirectional-iterator to support the circular buffer
	template<typename pointerType, typename referenceType>
	class circular_iterator : public std::iterator<std::bidirectional_iterator_tag, value_type, difference_type, pointerType, referenceType> {
		// Reference to the buffer which created this iterator
		circular_buffer& creator;
		// Index (in array space) the iterator is currently pointing to
		size_type i;

	public:
		explicit circular_iterator(circular_buffer& creator, const size_type i): creator(creator), i(i) {}
		// Increment support
		circular_iterator& operator++() { creator.increment(i); return *this; }
        circular_iterator operator++(int) { circular_iterator retval = *this; ++(*this); return retval; }
		// Decrement support
		circular_iterator& operator--() { creator.decrement(i); return *this; }
        circular_iterator operator--(int) { circular_iterator retval = *this; --(*this); return retval; }
		// In/equality support
        bool operator==(circular_iterator& other) const { return i == other.i && creator == other.creator; }
        bool operator!=(circular_iterator& other) const { return !(*this == other); }
		// Dereference support
        reference operator*() const { return creator.container_type::operator[](i); }
	};

public:
	// Iterator types
	using iterator = circular_iterator<pointer, reference>;
	using const_iterator = circular_iterator<const_pointer, const_reference>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

protected:
	// The number of elements of the array which are currently in use
	size_type _size = 0;
	// The index of the array which maps to index [0] in the buffer
 	size_type start = 0;
	// The index of the array which maps to the end of the buffer
	size_type _end() { return (start + _size) % _capacity(); }

public:
	circular_buffer() = default;
	circular_buffer(const Container& c, size_t start = 0) : Container(c), _size(c.size() - 1), start(start) {}
	circular_buffer(const Container&& c, size_t start = 0) : Container(std::move(c)), _size(c.size() - 1), start(start) {}
	circular_buffer(const circular_buffer& b) : Container(b), _size(b._size), start(b.start) {}
	circular_buffer(const circular_buffer&& b) : Container(std::move(b)), _size(b._size), start(b.start) {}
	using Container::Container;

	circular_buffer& operator=(const circular_buffer& other) {
		*((Container*) this) = other;
		_size = other._size;
		start = other.start;

		return *this;
	}

	circular_buffer& operator=(const circular_buffer&& other) {
		*((Container*) this) = std::move(other);
		_size = other._size;
		start = other.start;

		return *this;
	}

	circular_buffer& operator=(const Container& other) { *((Container*) this) = other; }
	circular_buffer& operator=(const Container&& other) { *((Container*) this) = std::move(other); }


	// The number of elements the buffer is capable of holding
	size_type capacity() const { return _capacity() - 1; }
	// The number of elements currently in the buffer
	size_type size() const { return _size; }
	// True if the buffer is empty, false otherwise
	bool empty() const { return size() == 0; }

	// Forward seeking iterators
	iterator begin() { return iterator(*this, start); }
	const_iterator cbegin() const { return const_iterator(*this, start); }
	iterator end() { return iterator(*this, _end()); }
	const_iterator cend() const { return const_iterator(*this, _end()); }

	// Backward seeking iterators
	reverse_iterator rbegin() { return iterator(*this, start); }
	const_reverse_iterator crbegin() const { return const_iterator(*this, start); }
	reverse_iterator rend() { return iterator(*this, _end()); }
	const_reverse_iterator crend() const { return const_iterator(*this, _end()); }

	// Function which returns the base container's allocator
	// NOTE: Only gets compiled into the class if the base container has a get_allocator function
	template<typename Q = Container> typename std::enable_if<has_get_allocator<Q, typename Q::allocator_type()>::value, typename Q::allocator_type>::type
	get_allocator(){ return Q::get_allocator(); }

	// Function which returns the base container's allocator
	// NOTE: Only gets compiled into the class if the base container DOESN'T HAVE a get_allocator function
	template<typename Q = Container> typename std::enable_if<!has_get_allocator<Q, typename Q::allocator_type()>::value, std::allocator<value_type>>::type
	get_allocator(){ return std::allocator<value_type>{}; }

	// Constructs a new element at the start of the buffer, returns an iterator to it
	template<typename... Args>
	iterator emplace_front(Args&&... args){
		if(size() < _capacity() - 1){
			decrement(start);
			_size++;
		} else decrement(start);

		get_allocator().construct(&Container::operator[](start), std::forward<Args>(args)...);
		return begin();
	}

	// Function which adds an element to the start of the buffer
	void push_front(const_reference r){
		if(size() < _capacity() - 1){
			decrement(start);
			_size++;
		} else decrement(start);

		Container::operator[](start) = r;
	}

	// Function which removes an element from the front of the buffer
	void pop_front() {
		if(size() > 0){
			increment(start);
			_size--;
		}
	}

	// Function which constructs a new element at the end of the buffer, returns an iterator to it
	template<typename... Args>
	iterator emplace_back(Args&&... args){
		get_allocator().construct(&Container::operator[](_end()), std::forward<Args>(args)...);

		if(size() < _capacity() - 1) _size++;
		else increment(start);

		return iterator(*this, _end() - 1);
	}

	// Function which adds an element to the end of the buffer
	void push_back(const_reference r){
		Container::operator[](_end()) = r;

		if(size() < _capacity() - 1) _size++;
		else increment(start);
	}

	// Function which removes an element from the end of the buffer
	void pop_back() {
		if(size() > 0)
			_size--;
	}

	// Array access, the start of the array is considered to be start and it goes until the end
	reference operator[](size_type index){ return Container::operator[]((start + index) % _capacity()); }
	const_reference operator[](size_type index) const { return Container::operator[]((start + index) % _capacity()); }

	// Bounds checked array access, the start of the array is considered to be start and it goes until the end
	reference at(size_type index){
		if(index > size())
			throw std::out_of_range("The provided index: " + to_string(index) + " is out of bounds of the buffer.");
		return (*this)[index];
	}
	const_reference at(size_type index) const {
		if(index > size())
			throw std::out_of_range("The provided index: " + to_string(index) + " is out of bounds of the buffer.");
		return (*this)[index];
	}

	// Refernce to the element at the front of the array
	reference front() { return (*this)[0]; }
	const_reference front() const { return (*this)[0]; }
	// Reference to the element at the back of the array
	reference back() { return (*this)[size() - 1]; }
	const_reference back() const { return (*this)[size() - 1]; }

protected:
	// Function which determines how large the array can be
	template<typename Q = Container> typename std::enable_if<has_capacity<Q, size_type()>::value, size_type>::type
	_capacity() const { return Container::capacity(); }

	template<typename Q = Container> typename std::enable_if<!has_capacity<Q, size_type()>::value, size_type>::type
	_capacity() const { return Container::size(); }

	// Function which increments a value in circle space
	inline void increment(size_t& n){ n = (n + 1) % _capacity(); }
	// Function which decrements a value in circle space
	inline void decrement(size_t& n){
		if(n == 0) n = _capacity() - 1;
		else n = (n - 1) % _capacity();
	}
};

#endif /* end of include guard: CIRCULAR_BUFFER_HPP */
