#include "Hook.h"

#include <assert.h>
#include <iostream>

namespace Hook64 {
bool Hook64::IsHooked(void) {
    return this->hooked_;
}

bool MidFunctionHook::Hook(DWORD64 source, DWORD64 destination, unsigned int hook_length, unsigned int extra_bytes, bool use_shadow_space) {
    if (!hooked_) {
        if (hook_length < minimum_number_of_bytes_to_overwrite_) {
            assert(hook_length >= minimum_number_of_bytes_to_overwrite_);
            return false;
        }

        static BYTE asm_mov_rax[] = {0x48, 0xB8};
        static BYTE asm_push_rax = 0x50;
        static BYTE asm_pop_rsp = 0x5C;
        static BYTE asm_pop_rax = 0x58;
        static BYTE asm_jmp_rax[] = {0xFF, 0xE0};
        static BYTE asm_ret = 0xC3;
        static BYTE asm_nop = 0x90;

        static BYTE asm_pushad_equivalent[] = {0x50, 0x53, 0x51, 0x52, 0x55, 0x57, 0x56, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57};
        static BYTE asm_popad_equivalent[] = {0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5E, 0x5F, 0x5D, 0x5A, 0x59, 0x5B, 0x58};

        static BYTE asm_restore_rax[] = {0x48, 0x8B, 0x44, 0x24, sizeof(DWORD64)};

        BYTE asm_allocate_shadow_space[] = {0x48, 0x83, 0xEC, 0x20};  // 32 bytes for shadow space + extra bytes for 16 bit stack allignment
        BYTE asm_deallocate_shadow_space[] = {0x48, 0x83, 0xC4, 0x20};

        static BYTE asm_allign_stack[] = {0x48, 0x89, 0xE0, 0x48, 0x83, 0xE8, 0x08, 0x48, 0x83, 0xE0, 0x0F, 0x48, 0x29, 0xC4};
        static BYTE asm_add_rsp_rax[] = {0x48, 0x01, 0xC4};

        // Save the original bytes
        original_bytes_ = new BYTE[hook_length];
        memcpy(original_bytes_, (void*)source, hook_length);

        // Get the address of the next instruction after where the hook is being placed
        DWORD64 next_instruction_address = source + hook_length - 1;

        // Hook
        /*
        __asm{
            push rax (1 byte) // Save rax
            movabs rax, executable_buffer (10 bytes)
            jmp rax (2 bytes)
            pop rax (1 byte) // Restore rax
        }

        */

        // Executable buffer
        /*
        __asm {
            pop rax // (1 byte) Get original rax from source function, also change RSP back to original value

            save all registers (23 bytes)

            // Stack allignment (14 bytes)
            // Get rsp and sub 8 to it, because we want 16 bit allignment on the shadow space, and we are going to push rax (8 bytes) before the shadow space. Then check if that rsp value (stored in rax) is 16 byte alligned. Get the remainder by AND with 0x15h (4 high bits, as 2^4 = 16) then sub it to rsp value (stored in rax) (move the stack down) to ensure 16 byte allignment.
            // Because rsp prior to alignment is pushed to stack then popped from stack after function returns, no need to reverse the allignment
            mov    rax,rsp
            sub    rax,0x08 //Sub 8 because rsp will decrease by 8 bytes from here to the shadow space before the detour function is called, due to push rax (the remainder from allignment checking)
            and    rax, 15 (0xF)
            sub    rsp,rax // subtracting from rsp because stack grows downwards (towards lower addresses)

            push rax // 1 byte -> (save the amount of bytes we are alligning by)

            sub rsp, 0x20 (32) // (4 bytes) Allocate shadow space for 4 8-byte values of the registers for function call, these 32 bytes on the stack need to be right before the return address when calling a function

            movabs rax, after_jump_address (*) // (10 bytes) After the jmp on the next line, we want to return to the instruction after the jump, so this is the "return address" for the function going to be called (the detour function)
            push rax (1 bytes)

            movabs rax, destination (10 bytes (2 opcode, 8 dest)) // Move hook destination in to rax
            jmp rax (2 bytes) // Jmp to rax

            (*) add rsp, 0x20 (32) // (4 bytes) deallocate shadow space after function call, restore rsp

            pop rax //(1 byte) (restore rax which holds the remainder from allignment)
            add rsp, rax // (3 bytes) (move the stack back up by the extra allignment bytes)

            restore all registers (23 bytes)

            // (hook_size bytes) Copy all the original instructions here

            push rax // (1 byte) Push it back onto stack so it matches with source function return where it pops rax before continuing

            movabs rax, next_instruction_address // (10 bytes) move return address (to original function) into rax
            jmp rax // (2 bytes)
        }
        */

        // Create the buffer
        unsigned int buffer_size = sizeof(asm_pop_rax) + sizeof(asm_push_rax) + sizeof(asm_pushad_equivalent) + sizeof(asm_allign_stack) + sizeof(asm_push_rax) + (use_shadow_space ? sizeof(asm_allocate_shadow_space) : 0) + sizeof(asm_mov_rax) + sizeof(asm_push_rax) + sizeof(DWORD64) + sizeof(asm_mov_rax) + sizeof(DWORD64) + sizeof(asm_jmp_rax) + (use_shadow_space ? sizeof(asm_deallocate_shadow_space) : 0) + sizeof(asm_pop_rax) + sizeof(asm_add_rsp_rax) + sizeof(asm_popad_equivalent) + hook_length + sizeof(asm_mov_rax) + sizeof(DWORD64) + sizeof(asm_jmp_rax);

        executeable_buffer_ = VirtualAlloc(NULL, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        BYTE* executeable_buffer = (BYTE*)executeable_buffer_;

        int index = 0;

        // Restore rax
        executeable_buffer[index] = asm_pop_rax;
        index += sizeof(asm_pop_rax);

        // Save all registers
        memcpy(executeable_buffer + index, asm_pushad_equivalent, sizeof(asm_pushad_equivalent));  // Save all registers
        index += sizeof(asm_pushad_equivalent);

        // Check if stack needs to be alligned
        memcpy(executeable_buffer + index, asm_allign_stack, sizeof(asm_allign_stack));
        index += sizeof(asm_allign_stack);
        executeable_buffer[index] = asm_push_rax;
        index += sizeof(BYTE);

        if (use_shadow_space) {
            // Allocate shadow space
            memcpy(executeable_buffer + index, asm_allocate_shadow_space, sizeof(asm_allocate_shadow_space));
            index += sizeof(asm_allocate_shadow_space);
        }

        // Push address to return to after executing detour function
        memcpy(executeable_buffer + index, asm_mov_rax, sizeof(asm_mov_rax));
        index += sizeof(asm_mov_rax);
        *((DWORD64*)(executeable_buffer + index)) = (DWORD64)executeable_buffer + index + sizeof(DWORD64) + sizeof(asm_push_rax) + 0 * sizeof(asm_allocate_shadow_space) + sizeof(asm_mov_rax) + sizeof(DWORD64) + sizeof(asm_jmp_rax);
        index += sizeof(DWORD64);
        executeable_buffer[index] = asm_push_rax;
        index += sizeof(BYTE);

        // Jump into detour function
        memcpy(executeable_buffer + index, asm_mov_rax, sizeof(asm_mov_rax));
        index += sizeof(asm_mov_rax);
        *((DWORD64*)(executeable_buffer + index)) = (DWORD64)destination;
        index += sizeof(DWORD64);
        memcpy(executeable_buffer + index, asm_jmp_rax, sizeof(asm_jmp_rax));
        index += sizeof(asm_jmp_rax);

        if (use_shadow_space) {
            // Deallocate shadow space
            memcpy(executeable_buffer + index, asm_deallocate_shadow_space, sizeof(asm_deallocate_shadow_space));
            index += sizeof(asm_deallocate_shadow_space);
        }

        // (*) Return here from detour function
        // Pop rax, the extra allignment value
        executeable_buffer[index] = asm_pop_rax;
        index += sizeof(asm_pop_rax);
        // Add the extra allignment back into rsp
        memcpy(executeable_buffer + index, asm_add_rsp_rax, sizeof(asm_add_rsp_rax));
        index += sizeof(asm_add_rsp_rax);

        // Restore registers
        memcpy(executeable_buffer + index, asm_popad_equivalent, sizeof(asm_popad_equivalent));
        index += sizeof(asm_popad_equivalent);

        // Execute original code
        memcpy(executeable_buffer + index, original_bytes_, hook_length);
        index += hook_length;

        // Save rax
        executeable_buffer[index] = asm_push_rax;
        index += sizeof(asm_push_rax);

        // Jump back to source function
        memcpy(executeable_buffer + index, asm_mov_rax, sizeof(asm_mov_rax));
        index += sizeof(asm_mov_rax);
        *((DWORD64*)(executeable_buffer + index)) = next_instruction_address;
        index += sizeof(DWORD64);
        memcpy(executeable_buffer + index, asm_jmp_rax, sizeof(asm_jmp_rax));
        index += sizeof(asm_jmp_rax);

        assert(index == buffer_size);

        DWORD protection;
        VirtualProtect(executeable_buffer_, buffer_size, PAGE_EXECUTE_READ, &protection);

        ////////////////////////////////////////////////////
        VirtualProtect((void*)source, hook_length, PAGE_EXECUTE_READWRITE, &protection);

        // NOP everything in the hook location
        for (int i = 0; i < hook_length; i++) {
            *((BYTE*)(source + i)) = asm_nop;
        }

        index = 0;

        BYTE* source_pointer = (BYTE*)source;

        // Push rax and save it in stack
        *((BYTE*)(source_pointer + index)) = asm_push_rax;
        index += sizeof(asm_push_rax);

        // Move address of executable buffer into rax
        memcpy(source_pointer + index, asm_mov_rax, sizeof(asm_mov_rax));
        index += sizeof(asm_mov_rax);
        *((DWORD64*)(source_pointer + index)) = (DWORD64)executeable_buffer;
        index += sizeof(DWORD64);

        // Jump rax
        memcpy(source_pointer + index, asm_jmp_rax, sizeof(asm_jmp_rax));
        index += sizeof(asm_jmp_rax);

        // assert(next_instruction_address == source + index);
        //*/

        // Pop rax and restore from stack
        *((BYTE*)next_instruction_address) = asm_pop_rax;

        // index += sizeof(asm_pop_rax);
        //*((BYTE*)(source + hook_length - 1)) = asm_pop_rax;

        VirtualProtect((void*)source, hook_length, protection, &protection);

        hook_address_ = source;
        hooked_ = true;
        return true;
    } else {
        return false;
    }
}

MidFunctionHook::MidFunctionHook(DWORD64 source, DWORD64 destination, int start_index, int end_index, bool padding, bool use_shadow_space) {
    if (end_index == 0) {
        end_index = start_index + minimum_number_of_bytes_to_overwrite_;
    }
    assert(start_index < end_index);
    bool result = Hook(source + start_index, destination, end_index - start_index, padding * 8, use_shadow_space);
}

}  // namespace Hook64