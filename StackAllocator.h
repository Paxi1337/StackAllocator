typedef unsigned char byte;

class StackAllocator {
public:
	/**
	 * Constructor
	 * 
	 * @remark The constructor will allocate size + alignment - 1 Bytes
	 *		   for handling the worst case of misalignment that may happen.
	 * 
	 * @param size The size in bytes of the stack.
	 * @param alignment The initial alignment.
	 * @param notExceedingInitialAlignment Flag for indicating if any allocation with allocate()
										   has a greater alignment than the initial alignment.
										   See allocateNotExceedingInitialAlignment()
	 */
	StackAllocator(size_t size, size_t alignment, bool exceedingInitialAlignment = true);

	/**
	 * Destructor
	 *
	 * Frees all allocated memory.
	 */
	~StackAllocator();
	
	/**
	 * Allocate size byte from the stack while aligning the returned address to alignment.
	 *
	 * @remark Each allocation from the stack has an overhead of 4 Bytes to handle 
	 *		   the custom alignment for each allocation.
	 *
	 * @param size The size in bytes to allocate.
	 * @param alignment The alignment of the returned address.
	 * @param offset Ptr+offset is aligned to alignment boundary.
	 *
	 * @return void* The aligned address or nullptr if there is no memory left.
	 */
	void* allocate(size_t size, size_t alignment, size_t offset = 0);
	
	/**
	 * Sets the current stackpointer to the alignment before the allocation of
	 * addr.
	 *
	 * @param addr An address returned in previous allocation.
	 */
	void free(void* addr);
	
	/**
	 * Resets the stack pointer to the aligned start address.
	 *
	 * @return void
	 */
	void reset();

private:
	/**
	 * Allocation header placed in front
	 * of each user allocation to store
	 * adjustment from unaligned to aligned address.
	 */
	struct AllocationHeader {
		size_t adjustment;
	};

	// Copying and assigning is prohibited.
	StackAllocator(const StackAllocator&); 
	StackAllocator& operator=(const StackAllocator&);

	/**
	 * The default implementation of allocate().
	 *
	 * @param size The size in bytes to allocate.
	 * @param alignment The alignment of the returned address.
	 * @param offset Specifies an offset to which the returned Ptr is aligned to. 
	 *				 Only supported when exceedingInitialAlignment == true.
	 *
	 * @return void* The aligned address or nullptr if there is no memory left.
	 */
	void* allocateExceedingInitialAlignment(size_t size, size_t alignment, size_t offset);

	/**
	* Special implementation of allocate() for alignments <= initial alignment.
	* This is somehow an experimental feature as proof of concept.
	*
	* @remark:
	* This method was added along with the flag for telling the stack
	* if any alignment will exceed the initial alignment to provide
	* a way of allocating size bytes without producing any overhead on the stack.
	*
	* --------------------------------------------------------------------------------------------------------
	* For example:
	*
	* LinearAllocator alloc(1024,4,true);
	* can allocate 1024 individual bytes from the stack.
	*
	* LinearAllocator alloc(1024,4,false);
	* will need to store the misalignment in every allocation in case the alignment exceeds the initial alignment
	* and produces  4 bytes overhead for each allocation.
	* In case of allocating individual bytes each allocation takes 5 Bytes and only 204 bytes can be allocated
	* because of the overhead (204*5 = 1020).
	* --------------------------------------------------------------------------------------------------------
	*
	* @param size The size in bytes to allocate.
	* @param alignment The alignment of the returned address.
	*
	* @return void* The aligned address or nullptr if there is no memory left.
	*/
	void* allocateNotExceedingInitialAlignment(size_t size, size_t alignment);

	// members
	void* mStartUnaligned;
	byte* mStartAligned;
	union {
		byte* asByte;
		void* asVoid;
	};
	byte* mEnd;
	size_t mInitialAlignment;
	bool mExceedingInitialAlignment;
};