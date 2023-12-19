#include "cshortasm.h"
#include "main.h"
#ifdef WIN32
#	include <windows.h>
#else
#	include <sys/mman.h>
#endif

CShortAsm::CShortAsm(uint32_t pages) {
	_size = pages * PAGE_SIZE;
	_code = static_cast<uint8_t*>(myalloc(_size));
}

CShortAsm::~CShortAsm() {
	myfree(_code, _size);
}

void CShortAsm::insert(uint8_t* array, uint32_t size) {
	for (int i = 0; i < static_cast<int>(size); ++i)
		write(array[i]);
}

void CShortAsm::insert(std::vector<uint8_t> array) {
	insert(reinterpret_cast<uint8_t*>(array.data()), static_cast<uint32_t>(array.size()));
}

void CShortAsm::push(uint8_t value) {
	write(0x6a);
	write(value);
}

void CShortAsm::push(int value)
{
	write(0x68);
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::push(r86 r)
{
	write(0x50 + static_cast<uint8_t>(r));
}

void CShortAsm::pushad()
{
	write(0x60);
}

void CShortAsm::pushfd()
{
	write(0x9C);
}

void CShortAsm::pop(r86 r)
{
	write(0x58 + static_cast<uint8_t>(r));
}

void CShortAsm::popad()
{
	write(0x61);
}

void CShortAsm::popfd()
{
	write(0x9D);
}

void CShortAsm::label(std::string label)
{
	_labels[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	if (_jmps[label] != 0) {
		setWriteOffset(_jmps[label]);
		jmp(static_cast<int>(_labels[label]));
	}
	if (_jes[label] != 0) {
		setWriteOffset(_jes[label]);
		je(static_cast<int>(_labels[label]));
	}
	if (_jnes[label] != 0) {
		setWriteOffset(_jnes[label]);
		jne(static_cast<int>(_labels[label]));
	}
	if (_jls[label] != 0) {
		setWriteOffset(_jls[label]);
		jl(static_cast<int>(_labels[label]));
	}
	if (_jles[label] != 0) {
		setWriteOffset(_jles[label]);
		jle(static_cast<int>(_labels[label]));
	}
	if (_jgs[label] != 0) {
		setWriteOffset(_jgs[label]);
		jg(static_cast<int>(_labels[label]));
	}
	if (_jges[label] != 0) {
		setWriteOffset(_jges[label]);
		jge(static_cast<int>(_labels[label]));
	}
	if (_jbs[label] != 0) {
		setWriteOffset(_jbs[label]);
		jb(static_cast<int>(_labels[label]));
	}
	if (_jbes[label] != 0) {
		setWriteOffset(_jbes[label]);
		jbe(static_cast<int>(_labels[label]));
	}
	if (_jas[label] != 0) {
		setWriteOffset(_jas[label]);
		ja(static_cast<int>(_labels[label]));
	}
	if (_jaes[label] != 0) {
		setWriteOffset(_jaes[label]);
		jae(static_cast<int>(_labels[label]));
	}
	setWriteOffset(_labels[label]);
}

void CShortAsm::jmp(int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0xEB);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0xE9);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jmp(std::string label)
{
	if (_labels[label] == 0)
		_jmps[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jmp(static_cast<int>(_labels[label]));
}

void CShortAsm::jmp(r86 r)
{
	write(0xff);
	write(0xE0 + static_cast<uint8_t>(r));
}

void CShortAsm::ret(int pops)
{
	if (!pops)
		write(0xC3);
	else {
		byteValue<uint16_t> v;
		v.value = static_cast<uint16_t>(pops);
		write(0xC2);
		write(v.bytes[0]);
		write(v.bytes[1]);
	}
}

void CShortAsm::nop()
{
	write(0x90);
}

void CShortAsm::call(int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	write(0xE8);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::call(std::string label)
{
	call(static_cast<int>(_labels[label]));
}

void CShortAsm::call(r86 r)
{
	write(0xff);
	write(0xD0 + static_cast<uint8_t>(r));
}

void CShortAsm::mov(int& var, r86 r)
{
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(&var);
	if (r != EAX)
		write(0x89);
	switch (r) {
	case EAX:
		write(0xa3);
		break;
	case ECX:
		write(0x0d);
		break;
	case EDX:
		write(0x15);
		break;
	case EBX:
		write(0x1d);
		break;
	case ESP:
		write(0x25);
		break;
	case EBP:
		write(0x2D);
		break;
	case ESI:
		write(0x35);
		break;
	case EDI:
		write(0x3d);
		break;
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::mov(r86 r, int& var)
{
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(&var);
	if (r != EAX)
		write(0x8b);
	switch (r) {
	case EAX:
		write(0xa1);
		break;
	case ECX:
		write(0x0d);
		break;
	case EDX:
		write(0x15);
		break;
	case EBX:
		write(0x1d);
		break;
	case ESP:
		write(0x25);
		break;
	case EBP:
		write(0x2D);
		break;
	case ESI:
		write(0x35);
		break;
	case EDI:
		write(0x3d);
		break;
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::mov(r86 r, const int value)
{
	write(0xB8 + static_cast<uint8_t>(r));
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::mov(r86 r1, r86 r2)
{
	write(0x89);
	write(0x08 * static_cast<uint8_t>(r2) + static_cast<uint8_t>(r1) + 0xC0);
}

void CShortAsm::mov(r86 r1, r86 r2, uint8_t offset) {
	write(0x8B);
	write(0x08 * static_cast<uint8_t>(r1) + static_cast<uint8_t>(r2) + (offset == 0 ? 0x00 : 0x40));
	if (offset != 0)
		write(offset);
}

void CShortAsm::mov(r86 r1, uint8_t offset, r86 r2) {
	write(0x89);
	if (r1 != ESP && r1 != EBP) {
		write(static_cast<uint8_t>(r1) + 0x08 * static_cast<uint8_t>(r2) +
			(offset == 0 ? 0x00 : 0x40));
	}
	else if (r1 == ESP) {
		write(0x04 + 0x08 * static_cast<uint8_t>(r2) + (offset == 0 ? 0x00 : 0x40));
	}
	else if (r1 == EBP) {
		write(0x45 + static_cast<uint8_t>(r2));
	}
	if (offset != 0 || r1 == EBP)
		write(offset);
}

void CShortAsm::xchg(r86 r1, r86 r2, uint8_t offset) {
	if (r2 == EAX)
		std::swap(r1, r2);
	if (!offset) {
		if (r1 == EAX)
			write(0x90 + static_cast<uint8_t>(r2));
		else {
			write(0x87);
			write(0xC8 + static_cast<uint8_t>(r1) + 0x08 * (static_cast<uint8_t>(r2) - 1));
		}
	}
	else {
		write(0x87);
		write(0x40 + static_cast<uint8_t>(r2) + 0x08 * static_cast<uint8_t>(r1));
		write(offset);
	}
}

void CShortAsm::xchg(r86 r, int& var)
{
	write(0x87);
	write(0x05 + static_cast<uint8_t>(r));
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(&var);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::add(r86 r, uint8_t value) {
	write(0x83);
	write(0xC0 + static_cast<uint8_t>(r));
	write(value);
}

void CShortAsm::add(r86 r, const int value)
{
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	if (r == EAX) {
		write(0x05);
	}
	else {
		write(0x81);
		write(0xC0 + static_cast<uint8_t>(r));
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::add(r86 r, int& var)
{
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(&var);
	write(0x03);
	write(0x05 + 0x08 * static_cast<uint8_t>(r));
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::sub(r86 r, uint8_t value) {
	write(0x83);
	write(0xE8 + static_cast<uint8_t>(r));
	write(value);
}

void CShortAsm::sub(r86 r, const int value)
{
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	if (r == EAX) {
		write(0x2D);
	}
	else {
		write(0x81);
		write(0xE8 + static_cast<uint8_t>(r));
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::sub(r86 r, int& var)
{
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(&var);
	write(0x2B);
	write(0x05 + 0x08 * static_cast<uint8_t>(r));
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::mul(r86 r)
{
	write(0xF7);
	write(0xE0 + static_cast<uint8_t>(r));
}

void CShortAsm::imul(r86 r)
{
	write(0xF7);
	write(0xE8 + static_cast<uint8_t>(r));
}

void CShortAsm::imul(r86 r1, r86 r2)
{
	write(0x0F);
	write(0xAF);
	write(0xC0 + static_cast<uint8_t>(r2) + 0x08 * r1);
}

void CShortAsm::imul(r86 r, uint8_t value) {
	write(0x6B);
	write(0xC0 + 0x09 * r);
	write(value);
}

void CShortAsm::imul(r86 r, const int value)
{
	write(0x69);
	write(0xC0 + 0x09 * r);
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::imul(r86 r1, r86 r2, uint8_t value) {
	write(0x6B);
	write(0xC0 + static_cast<uint8_t>(r2) + 0x08 * r1);
	write(value);
}

void CShortAsm::imul(r86 r1, r86 r2, const int value)
{
	write(0x69);
	write(0xC0 + static_cast<uint8_t>(r2) + 0x08 * r1);
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::div(r86 r)
{
	write(0xF7);
	write(0xF0 + static_cast<uint8_t>(r));
}

void CShortAsm::idiv(r86 r)
{
	write(0xF7);
	write(0xF8 + static_cast<uint8_t>(r));
}

void CShortAsm::cmp(r86 r1, r86 r2)
{
	write(0x39);
	write(0xC0 + static_cast<uint8_t>(r1) + 0x08 * static_cast<uint8_t>(r2));
}

void CShortAsm::cmp(r86 r, uint8_t value) {
	write(0x83);
	write(0xF8 + static_cast<uint8_t>(r));
	write(value);
}

void CShortAsm::cmp(r86 r, const int value)
{
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	if (r == EAX)
		write(0x3D);
	else {
		write(0x81);
		write(0xf8 + static_cast<uint8_t>(r));
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::XOR(r86 r1, r86 r2)
{
	write(0x31);
	write(0xC0 + static_cast<uint8_t>(r1) + 0x08 * static_cast<uint8_t>(r2));
}

void CShortAsm::XOR(r86 r, uint8_t value) {
	write(0x83);
	write(0xF0 + static_cast<uint8_t>(r));
	write(value);
}

void CShortAsm::XOR(r86 r, const int value)
{
	byteValue<uint32_t> v;
	v.value = static_cast<uint32_t>(value);
	if (r == EAX)
		write(0x35);
	else {
		write(0x81);
		write(0xf0 + static_cast<uint8_t>(r));
	}
	write(v.bytes[0]);
	write(v.bytes[1]);
	write(v.bytes[2]);
	write(v.bytes[3]);
}

void CShortAsm::je(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x74);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x84);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}

}

void CShortAsm::je(std::string label)
{
	if (_labels[label] == 0)
		_jes[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else je(static_cast<int>(_labels[label]));
}

void CShortAsm::jne(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x75);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x85);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jne(std::string label)
{
	if (_labels[label] == 0)
		_jnes[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jne(static_cast<int>(_labels[label]));
}

void CShortAsm::jl(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x7C);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x8C);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jl(std::string label)
{
	if (_labels[label] == 0)
		_jls[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jl(static_cast<int>(_labels[label]));
}

void CShortAsm::jle(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x7E);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x8E);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jle(std::string label)
{
	if (_labels[label] == 0)
		_jles[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jle(static_cast<int>(_labels[label]));
}

void CShortAsm::jg(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x7F);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x8F);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jg(std::string label)
{
	if (_labels[label] == 0)
		_jgs[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jg(static_cast<int>(_labels[label]));
}

void CShortAsm::jge(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x7D);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x8D);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jge(std::string label)
{
	if (_labels[label] == 0)
		_jges[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jge(static_cast<int>(_labels[label]));
}

void CShortAsm::jb(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x72);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x82);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jb(std::string label)
{
	if (_labels[label] == 0)
		_jbs[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jb(static_cast<int>(_labels[label]));
}

void CShortAsm::jbe(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x76);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x86);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jbe(std::string label)
{
	if (_labels[label] == 0)
		_jbes[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jbe(static_cast<int>(_labels[label]));
}

void CShortAsm::ja(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x77);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x87);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::ja(std::string label)
{
	if (_labels[label] == 0)
		_jas[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else ja(static_cast<int>(_labels[label]));
}

void CShortAsm::jae(const int addr)
{
	byteValue<uint32_t> v;
	v.value = getRelativeAddress(static_cast<uint32_t>(addr));
	if (v.value < 127 && static_cast<int>(v.value) > -127) {
		write(0x73);
		write(static_cast<uint8_t>(v.value));
	}
	else {
		write(0x0F);
		write(0x83);
		write(v.bytes[0]);
		write(v.bytes[1]);
		write(v.bytes[2]);
		write(v.bytes[3]);
	}
}

void CShortAsm::jae(std::string label)
{
	if (_labels[label] == 0)
		_jaes[label] = reinterpret_cast<uint32_t>(_code) + _offset;
	else jae(static_cast<int>(_labels[label]));
}












void CShortAsm::resetWriteOffset()
{
	_offset = 0;
}

uint32_t CShortAsm::getWriteOffset() {
	return _offset;
}

void CShortAsm::setWriteOffset(uint32_t offset) {
	_offset = offset;
}

uint32_t CShortAsm::arrayToPages(size_t size_array) {
	return (size_array / PAGE_SIZE) + 1;
}

const uint8_t* CShortAsm::getAddr() {
	return _code;
}

uint32_t CShortAsm::getSize() {
	return _peak;
}

void CShortAsm::resize(uint32_t pages) {
	uint8_t* new_code = static_cast<uint8_t*>(myalloc(pages * PAGE_SIZE));
	memcpy(new_code, _code, _size);
	_size = pages * PAGE_SIZE;
	myfree(_code, _size);
	_code = new_code;
}

uint32_t CShortAsm::getRelativeAddress(uint32_t addr) {
	return addr - ((reinterpret_cast<uint32_t>(_code) + _offset) + 5);
}

void CShortAsm::write(uint8_t v) {
	if (_offset + 1 >= _size)
		resize(arrayToPages(_offset * 2));
	_code[_offset++] = v;
	if (_offset > _peak) _peak = _offset;
}

void* CShortAsm::myalloc(size_t size) {
#ifdef WIN32
	auto Unprotect = [](size_t address, size_t size) // Allow execute code. By FYP
		{
			do {
				MEMORY_BASIC_INFORMATION mbi;
				if (!VirtualQuery(reinterpret_cast<PVOID>(address), &mbi, sizeof(mbi)))
					throw "virtual query error";
				if (size > mbi.RegionSize)
					size -= mbi.RegionSize;
				else
					size = 0;
				DWORD oldp;
				if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldp))
					throw "virtual protect error";
				if (reinterpret_cast<size_t>(mbi.BaseAddress) + mbi.RegionSize < address + size)
					address = reinterpret_cast<size_t>(mbi.BaseAddress) + mbi.RegionSize;
			} while (size);
		};
	auto addr = malloc(size);
	Unprotect(reinterpret_cast<size_t>(addr), size);
	return addr;
#else
	return mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
#endif
}

void CShortAsm::myfree(void* ptr, size_t size) {
#ifdef WIN32
	VirtualFree(ptr, size, 0);
#else
	munmap(ptr, size);
#endif
}
