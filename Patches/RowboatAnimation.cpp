/**
* Copyright (C) 2018 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

// Predefined code bytes
constexpr BYTE RowboatSearchBytes[]{ 0x8B, 0x56, 0x08, 0x89, 0x10, 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x50, 0xC3 };
constexpr BYTE RowboatRETNBytes[]{ 0xC3 };
constexpr BYTE RowboatSearchPtrBytes[] = { 0x56, 0x6A, 0x0A, 0x6A, 0x00, 0x50, 0xE8 };
constexpr BYTE RowboatCMPBytes[] = { 0x3B, 0x05 };
constexpr BYTE RowboatPointerBytes[] = { 0x67, 0x45, 0x23, 0x01 };

// ASM functions to update Rowboat Animation
__declspec(naked) void __stdcall RowboatAnimationASM()
{
	__asm
	{
		cmp eax, 0x4C
		je near RowboatUpdate
		retn
		RowboatUpdate:
		mov dword ptr ds : [0x01234567], 0xFFFFFFFF
		retn
	}
}

// Update SH2 code to Fix Rowboat Animation
void UpdateRowboatAnimation()
{
	// Get Rowboat Animation address
	DWORD RowboatAddr = (DWORD)GetAddressOfData(RowboatSearchBytes, sizeof(RowboatSearchBytes), 1, 0x000049FBA5, 2600);
	if (!RowboatAddr)
	{
		Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	RowboatAddr += 0x22;

	// Get memory pointer for Rowboat Animation
	DWORD RowboatMemoryPtr = (DWORD)GetAddressOfData(RowboatSearchPtrBytes, sizeof(RowboatSearchPtrBytes), 1, 0x000053F356, 2600);
	if (!RowboatMemoryPtr)
	{
		Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	RowboatMemoryPtr -= 0x3D;

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)RowboatAddr, (void*)RowboatRETNBytes, sizeof(RowboatRETNBytes)) ||
		!CheckMemoryAddress((void*)RowboatMemoryPtr, (void*)RowboatCMPBytes, sizeof(RowboatCMPBytes)))
	{
		Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update 0x0123456 pointer in ASM code with the real pointer
	if (!ReplaceMemoryBytes((void*)RowboatPointerBytes, (void*)(RowboatMemoryPtr + 2), sizeof(DWORD), (DWORD)*RowboatAnimationASM, 0x10, 1))
	{
		Log() << __FUNCTION__ << " Error: replacing pointer in ASM!";
		return;
	}

	// Update SH2 code
	Log() << "Setting Rowboat Animation Fix...";
	WriteJMPtoMemory((BYTE*)RowboatAddr, *RowboatAnimationASM);
}