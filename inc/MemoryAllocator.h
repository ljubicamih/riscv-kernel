//
// Created by User on 07.05.2026..
//
#include "../lib/hw.h"

#ifndef UNTITLED1_MEMORYALLOCATOR_H
#define UNTITLED1_MEMORYALLOCATOR_H


class MemoryAllocator {
public:
    static MemoryAllocator* getInstance();
	static void init();
	static void* mem_alloc(size_t size);
	static int mem_free(void* ptr);
private:
    static MemoryAllocator* instance;
    MemoryAllocator();
	struct BlockHeader {
		size_t size;
		BlockHeader* next;
	};

	static BlockHeader* freeList;

};


#endif //UNTITLED1_MEMORYALLOCATOR_H