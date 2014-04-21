#include "StackAllocator.h"
#include <malloc.h>
#include <cassert>

namespace helper {
	
	bool isPower2(int x) {
		return ((x) && ((x & (x - 1)) == 0));
	}

	byte* alignPtr(byte* pointer, size_t alignment, size_t& adjustMent) {
		size_t unalignedCurrent = reinterpret_cast<size_t>(pointer);
		adjustMent = alignment - (unalignedCurrent % alignment);
		if (adjustMent == alignment) adjustMent = 0;
		return pointer + adjustMent;
	}
}

StackAllocator::StackAllocator(size_t size, size_t alignment, bool exceedingInitialAlignment) : mStartUnaligned(nullptr),
															    mStartAligned(nullptr), 
																asByte(nullptr), 
																mEnd(nullptr),
																mInitialAlignment(alignment),
																mExceedingInitialAlignment(exceedingInitialAlignment)
{

	assert(helper::isPower2(mInitialAlignment));

	// malloc() should guarentee an alignment of <= 8 on 32bit and <= 16 on 64bit at the power of two.
	// In this case it would not be neceassarry to allocate more memory as requested.
	// But maybe not every compiler does this and therefore the size is adjusted to meet
	// the requirement for the worst misalignment (alignment - 1) in any case.
	
	size_t allocationSize = size;
	size_t extraForWorstAlignmentCase = mInitialAlignment - 1;

	allocationSize += extraForWorstAlignmentCase;

	// the unaligned memory region
	// mStartUnaligned stores the unaligned pointer for freeing
	mStartUnaligned = ::malloc(allocationSize);

	asVoid = mStartUnaligned;

	size_t unused;

	// mStartAligned stores the aligned pointer used in reset
	mStartAligned = helper::alignPtr(asByte, mInitialAlignment, unused);
	asByte = mStartAligned;
	mEnd = mStartAligned + size;
}

void* StackAllocator::allocateNotExceedingInitialAlignment(size_t size, size_t alignment) {
	assert(helper::isPower2(alignment) && alignment <= mInitialAlignment);

	void* userMem = asByte;

	asByte += size;

	return userMem;
}

void* StackAllocator::allocateExceedingInitialAlignment(size_t size, size_t alignment, size_t offset) {
	assert(helper::isPower2(alignment));

	void* userMem = nullptr;

	size += sizeof(AllocationHeader);

	size_t adjustMent = 0;
	// offset the pointer by sizeof(AllocationHeader) first, align it and offset it back
	// this method wastes less memory and ensures that the pointer to the user is still properly aligned
	asByte = helper::alignPtr(asByte + sizeof(AllocationHeader) + offset, alignment, adjustMent) - sizeof(AllocationHeader) - offset;

	if (asByte + size > mEnd) {
		return nullptr;
	}

	// store the ammount the address was adjusted + sizeof(AllocationHeader) right in front of the user pointer
	AllocationHeader* header = reinterpret_cast<AllocationHeader*>(asByte);
	header->adjustment = adjustMent + sizeof(AllocationHeader);

	// offset to start at the aligned address right after the AllocationHeader
	userMem = asByte + sizeof(AllocationHeader);

	asByte += size;

	return userMem;
}


void* StackAllocator::allocate(size_t size, size_t alignment, size_t offset) {
	if (mExceedingInitialAlignment)
		return allocateExceedingInitialAlignment(size, alignment, offset);
	else
		return allocateNotExceedingInitialAlignment(size, alignment);
}

void StackAllocator::free(void* addr) {
	if (addr) {
		if (mExceedingInitialAlignment) {
			AllocationHeader* adjustMent = reinterpret_cast<AllocationHeader*>((static_cast<byte*>(addr)-sizeof(AllocationHeader)));
			asVoid = static_cast<void*>(static_cast<byte*>(addr)-adjustMent->adjustment);
		}
		else {
			asVoid = addr;
		}
	}
}

void StackAllocator::reset() {
	asByte = mStartAligned;
}

StackAllocator::~StackAllocator() {
	::free(mStartUnaligned);
}