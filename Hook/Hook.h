#pragma once
#include <Windows.h>

namespace Hook64 {

//typedef DWORD64 QWORD;
class Hook64 {
   protected:
    bool hooked_ = false;

   public:
    bool IsHooked(void);
};

class MidFunctionHook : public Hook64 {
   public:
    static const BYTE minimum_number_of_bytes_to_overwrite_ = 14;

   public:
    void* original_bytes_;
    void* executeable_buffer_;
    DWORD64 hook_address_;

   public:
    bool Hook(DWORD64 source, DWORD64 destination, unsigned int hook_length, unsigned int extra_bytes = 0, bool use_shadow_space = true);
    bool UnHook(void);
    MidFunctionHook(DWORD64 source, DWORD64 destination, int start_offset, int end_offset, bool padding = false, bool use_shadow_space = true);
    // DWORD GetHookAddress(void);
};

}  // namespace Hook64