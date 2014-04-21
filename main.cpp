// StackAllocator.cpp : Defines the entry point for the console application.
//

#include <new>
#include <type_traits>
#include <cstdio>
#include "StackAllocator.h"

StackAllocator a(1024, 4);

class MyClass {

public:

	MyClass() {
		//printf(":\n");
	}

	static void operator delete(void* ptr, size_t size) {
		(void)size;
		a.free(ptr);
	}

	static void operator delete[](void* ptr, size_t size) {
		(void)size;
		a.free(ptr);
	}

	static void* operator new(size_t size){
		return a.allocate(size, std::alignment_of<MyClass>::value);
	}

	static void* operator new[](size_t size){
		// the returned pointer will be aligned to ret + sizeof(size_t)
		// the first four bytes safe the size
		return a.allocate(size, std::alignment_of<MyClass>::value, sizeof(size_t));
	}

	~MyClass() {
		//printf(".\n");
	}
	short member1;
private:

	
	short member2;
};


int main()
{

	int* ints = (int*)a.allocate(4, 4);

	char* chars = (char*)a.allocate(3, 4);
	chars = "012";

	int* intstwo = (int*)a.allocate(4, 4);


#if 0
	MyClass* c = new MyClass;
	delete c;

	MyClass* b = new MyClass;
	delete b;

	MyClass* arr = new MyClass[10];
	arr[7].member1 = 19;
	delete[] arr;

	MyClass* arr2 = new MyClass[10];
	delete[] arr2;
#endif


#if 0
	void* mem = a.allocate(256, 32);
	a.free(mem);
	printf("0x%08x\n", mem);
	mem = a.allocate(256, 32);
	printf("0x%08x\n", mem);
#endif

#if 0
	for (int i = 0; i < 1024; i++) {
		void* mem = a.allocate(1, 1);
		//if (!mem) __debugbreak();
	}
#endif

	return 0;
}

