//
// Created by User on 07.05.2026..
//

#include "../inc/MemoryAllocator.h"

MemoryAllocator* MemoryAllocator::instance = nullptr;
MemoryAllocator::BlockHeader* MemoryAllocator::freeList = nullptr;



void* MemoryAllocator::mem_alloc(size_t size){
    if(size ==0) return nullptr;

    // size je broj blokova
    // dodajemo jos 1 blok za zaglavlje (BlockHeader).
    size_t numBlocks = size + 1;

    BlockHeader* block = freeList;
    BlockHeader* prev = nullptr;

    while(block != nullptr) {
        if(block->size >= numBlocks) {
            if(block->size > numBlocks) {
                BlockHeader* newBlock = (BlockHeader*)((char*)block + numBlocks * MEM_BLOCK_SIZE);
                newBlock -> size = block->size - numBlocks;
                newBlock -> next = block->next;

                if(prev) prev->next = newBlock;
                else freeList = newBlock;
            }

            else{
                if(prev) prev->next = block->next;
                else freeList = block->next;
            }
            block->size = numBlocks;
            block -> next = nullptr;

            return (char*)block + MEM_BLOCK_SIZE;

        }
        prev = block;
        block = block->next;
    }
    return nullptr;
}

void MemoryAllocator::init(){
    freeList = (BlockHeader*)HEAP_START_ADDR;
    freeList -> size = ((size_t)HEAP_END_ADDR - (size_t)HEAP_START_ADDR) / MEM_BLOCK_SIZE;
    freeList -> next = nullptr;
}

int MemoryAllocator::mem_free(void* ptr){
    if(ptr == nullptr) return -1;

    BlockHeader* block = (BlockHeader*)((char*)ptr - MEM_BLOCK_SIZE);

    block -> next = freeList;
    freeList = block;

    return 0;
}