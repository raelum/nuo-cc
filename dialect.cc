// Exploration of how the various types in Nuo would be implemented.
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using Int = int;
using UInt = uint;
using Char = char;

// Allocation layout for Heap.
template <typename T>
struct HeapAllocation {
  UInt refCount;
  T data;

  HeapAllocation(T data) {
    this->refCount = 1;
    this->data = data;
  }
};

// Reference-counted owning pointers.
template <typename T>
struct Heap {
  HeapAllocation<T>* alloc;

  // Create a Heap object with the given data.
  Heap(T data) {
    print("Allocating memory.");
    this->alloc = new HeapAllocation(std::move(data));
  }

  // Create a Heap object with a pre-allocated HeapAllocation.
  Heap(HeapAllocation<T>* alloc) {
    print("Allocating memory.");
    this->alloc = alloc;
  };

  static Heap<T> allocate(UInt dataSize) {
    UInt heapAllocationSize = sizeof(HeapAllocation<T>) - sizeof(T) + dataSize;
    HeapAllocation<T>* alloc = (HeapAllocation<T>*)malloc(heapAllocationSize);
    alloc->refCount = 1;
    return Heap<T>(alloc);
  }

  // Increment reference count when copying heap struct.
  Heap(const Heap& other) {
    // print("Incrementing ref.")
    this->alloc = other.alloc;
    this->alloc->refCount++;
  }

  // Decrement reference count or free the allocation upon destruction.
  ~Heap() {
    if (this->alloc->refCount > 1) {
      this->alloc->refCount--;
    } else {
      print("Freeing memory.");
      free(this->alloc);
    }
  }

  // T& operator&() { return this->alloc->data; }

  // Dereference operator.
  T operator*() { return this->alloc->data; }
};

// Fixed-size Array type. Usage examples below.
//
// Create int stack array with elements. (this is auto-typed)
// auto arr = Array<>::of(1, 2, 3);
//
// Create int stack array with specified size.
// auto arr = Array<int, 5>();
//
// Create int array with flexible size.
// auto arr = Array<int>();
template <typename T = Int, UInt N = 0>
struct Array {
  UInt size = N;
  T elements[N];

  // Creates a stack array with the given elements.
  template <typename... Args>
  static Array<std::common_type_t<Args...>, sizeof...(Args)> of(Args... args) {
    return Array<std::common_type_t<Args...>, sizeof...(Args)>(args...);
  }

  static Heap<Array<T>> allocate(UInt length) {
    UInt arraySize = sizeof(Array<T>) + length * sizeof(T);
    auto arr = Heap<Array<T>>::allocate(arraySize);
    arr.alloc->data.size = length;
    return std::move(arr);
  }

  // Default constructor.
  Array<T, N>() {}

  // Constructor to initialize the array from the given args as elements.
  template <typename... Args>
  Array<T, N>(Args... args) {
    static_assert(areArgsSameAsT<Args...>,
                  "All arguments to Array constructor must be of type T.");
    static_assert(sizeof...(args) == N,
                  "Number of arguments to Array constructor must match N.");
    this->initializeArray(0, args...);
  }

  // Helper const expression to verify if all arguments are the same type as T.
  template <typename... Args>
  static constexpr bool areArgsSameAsT =
      std::conjunction_v<std::is_same<T, Args>...>;

  // Helper function to unroll variadic args and insert them into the array.
  template <typename... Args>
  void initializeArray(UInt index, T first, Args... rest) {
    this->data[index] = first;
    if constexpr (sizeof...(rest) > 0) {
      this->initializeArray(index + 1, rest...);
    }
  }
};

struct CustomString {
  Heap<Array<Char>> data;

  // Create a string from the given input.
  static CustomString of(const char* str) {
    auto stringArr = Array<Char>::allocate(strlen(str));
    for (Int i = 0; i < stringArr.alloc->data.size; i++) {
      stringArr.alloc->data.elements[i] = str[i];
    }
    return CustomString{.data = std::move(stringArr)};
  }
};

// Support for printing String.
std::ostream& operator<<(std::ostream& os, const CustomString& s) {
  for (int i = 0; i < s.data.alloc->data.size; i++) {
    os << s.data.alloc->data.elements[i];
  }
  return os;
}

struct StringSlice {
  Heap<Array<Char>> data;
  UInt start;
  UInt end;
};

// Support for printing StringSlice.
std::ostream& operator<<(std::ostream& os, const StringSlice& s) {
  for (int i = s.start; i < s.end; i++) {
    os << s.data.alloc->data.elements[i];
  }
  return os;
}

// Usage examples

// auto arr = Array<int>{size : 2, elements : new {1, 2}};
// auto c = Heap("Hi");
// print(*c);

// auto arr = Array<Int, 5>();
// arr.data = {1, 2, 3, 4, 5};
// print(sizeof(arr));

// auto arr = Array<Int, 5>(1, 2, 3, 4, 5);
// for (auto v : arr.elements) {
//   print(v);
// }

// auto arr3 = Array<>::from(9, 10, 11);
// for (auto v : arr3.elements) {
//   print(v);
// }

// auto carr = Array<Char>();
// print(sizeof(carr));

// auto arr = Array<Int>::allocate(5);
// for (Int i = 0; i < arr.alloc->data.size; i++) {
//   print(arr.alloc->data.elements[i]);
// }

// String2 name = String2::of("allen");
// print(name);