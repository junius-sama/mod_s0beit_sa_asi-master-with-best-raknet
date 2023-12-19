#include "ccallhook.h"
#include "main.h"

CCallHook::CCallHook(void* addr, uint32_t size, uint32_t stack, eCodePos pos, eSafeCall save, eEntry entry) {
	hook_addr = addr;
	_size = size;
	_stack = stack;
	if (_stack > 0)
		entry = e_jmp;
	_stackOffset = 4;
	hook_pos = pos;
	_entry = entry;
	_save = save;
	rasm = new CShortAsm();
	_func = nullptr;
	_object = nullptr;

	orig_bytes = new byte[size + 1];
	memsafe::copy(orig_bytes, addr, size);

	if (pos == cp_before) {
		if (orig_bytes[0] == 0xE9)
			pos = cp_after;
		else {
			ModOriginalBytesStart(reinterpret_cast<uint32_t>(rasm->getAddr()) + rasm->getWriteOffset());
			rasm->insert(orig_bytes, size);
		}
	}

	hook_offset = rasm->getWriteOffset();
	if (pos == cp_skip)
		ModOriginalBytesStart(reinterpret_cast<uint32_t>(rasm->getAddr()) + rasm->getWriteOffset());
	uint32_t endCode = disable();

	rasm->setWriteOffset(endCode);

	if (pos == cp_after) {
		ModOriginalBytesStart(reinterpret_cast<uint32_t>(rasm->getAddr()) + rasm->getWriteOffset());
		rasm->insert(orig_bytes, size);
	}

	if (entry == e_call)
		rasm->ret();
	else rasm->jmp(reinterpret_cast<int>(addr) + static_cast<int>(size));

	memsafe::set(addr, 0x90, size);
	byteValue<uint32_t> v;
	v.value = reinterpret_cast<uint32_t>(rasm->getAddr()) - (reinterpret_cast<uint32_t>(addr) + 5);
	if (entry == e_call)
		memsafe::set(addr, 0xE8, 1);
	else memsafe::set(addr, 0xE9, 1);
	memsafe::set(reinterpret_cast<void*>(reinterpret_cast<uint32_t>(addr) + 1), v.bytes[0], 1);
	memsafe::set(reinterpret_cast<void*>(reinterpret_cast<uint32_t>(addr) + 2), v.bytes[1], 1);
	memsafe::set(reinterpret_cast<void*>(reinterpret_cast<uint32_t>(addr) + 3), v.bytes[2], 1);
	memsafe::set(reinterpret_cast<void*>(reinterpret_cast<uint32_t>(addr) + 4), v.bytes[3], 1);
}

CCallHook::~CCallHook()
{
	disable();
	//delete rasm;
}

void CCallHook::setFunc(void(__stdcall* func)())
{
	_func = fn2void(func);
	_funcType = eFuncType::ft_callback;
}

void CCallHook::enable(void* func)
{
	setNops();
	if (func == nullptr) {
		if (hook_pos == cp_skip) {
			rasm->setWriteOffset(hook_offset);
			rasm->insert(orig_bytes, _size);
		}
		return;
	}
	rasm->setWriteOffset(hook_offset);

	byteValue<uint32_t> regAddr;
	regAddr.value = (uint32_t)this + offsetof(CCallHook, _registers);

	rasm->insert({ 0xA3, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x0D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x15, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x1D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x25, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x2D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x35, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x3D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	uint32_t so = _stackOffset + 4;
	if (checkFlag(_save, sc_registers)) {
		rasm->pushad();
		so += 32;
	}
	if (checkFlag(_save, sc_flags)) {
		rasm->pushfd();
		so += 4;
	}

	rasm->nop();
	rasm->push(r86::EAX);
	if (_entry == eEntry::e_call)
		so += 4;
	for (int i = _stack - 1; i >= 0; --i) {
		rasm->mov(r86::EAX, r86::ESP);
		rasm->add(r86::EAX, (const int)(so + i * 4));
		rasm->push(r86::EAX);
		so += 4;
	}

	rasm->call(reinterpret_cast<int>(func));
	//	for (uint32_t i = 0; i < _stack; ++i){
	//		rasm->pop(r86::EAX);
	//	}
	// TODO: use ret by hook for breaking function
	rasm->pop(r86::EAX);

	if (checkFlag(_save, sc_flags))
		rasm->popfd();
	if (checkFlag(_save, sc_registers)) rasm->popad();

	regAddr.value = (uint32_t)this + offsetof(CCallHook, _registers);

	rasm->insert({ 0xA1, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x0D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x15, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x1D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x25, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x2D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x35, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x3D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	_func = func;
	_funcType = eFuncType::ft_callback;
}

void CCallHook::enable(void* obj, void* method)
{
	setNops();
	if (method == nullptr) {
		if (hook_pos == cp_skip) {
			rasm->setWriteOffset(hook_offset);
			rasm->insert(orig_bytes, _size);
		}
		return;
	}

	rasm->setWriteOffset(hook_offset);

	byteValue<uint32_t> regAddr;
	regAddr.value = (uint32_t)this + offsetof(CCallHook, _registers);

	rasm->insert({ 0xA3, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x0D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x15, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x1D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x25, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x2D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x35, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x89, 0x3D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	uint32_t so = _stackOffset + 4;
	if (checkFlag(_save, sc_registers)) {
		rasm->pushad();
		so += 32;
	}
	if (checkFlag(_save, sc_flags)) {
		rasm->pushfd();
		so += 4;
	}

	rasm->push(r86::EAX);
	if (_entry == eEntry::e_call)
		so += 4;
	for (int i = _stack - 1; i >= 0; --i) {
		rasm->mov(r86::EAX, r86::ESP);
		rasm->add(r86::EAX, (const int)(so + i * 4));
		rasm->push(r86::EAX);
		so += 4;
	}

	rasm->mov(r86::ECX, reinterpret_cast<const int>(obj));
	rasm->call(reinterpret_cast<int>(method));

	// WARNING: may be use need pop args???
	rasm->pop(r86::EAX);

	if (checkFlag(_save, sc_flags))
		rasm->popfd();
	if (checkFlag(_save, sc_registers)) rasm->popad();

	regAddr.value = (uint32_t)this + offsetof(CCallHook, _registers);

	rasm->insert({ 0xA1, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x0D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x15, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x1D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x25, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x2D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x35, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });

	regAddr.value += 4;
	rasm->insert({ 0x8B, 0x3D, regAddr.bytes[0], regAddr.bytes[1], regAddr.bytes[2], regAddr.bytes[3] });
}

void CCallHook::enable()
{
	if (_funcType == eFuncType::ft_callback)
		enable(_func);
	else if (_funcType == eFuncType::ft_method)
		enable(_object, _func);
}

uint32_t CCallHook::disable() {
	setNops();
	uint32_t result = rasm->getWriteOffset();
	if (hook_pos == cp_skip) {
		rasm->setWriteOffset(hook_offset);
		rasm->insert(orig_bytes, _size);
	}
	return result;
}

void CCallHook::setStackOffset(uint32_t so) {
	_stackOffset = so;
}

uint32_t CCallHook::stackOffset() {
	return _stackOffset;
}

void CCallHook::setReg86(r86 r, uint32_t value) {
	_registers[r] = value;
}

uint32_t CCallHook::reg86(r86 r) {
	return _registers[r];
}

template<typename T> inline
bool CCallHook::checkFlag(T value, T flag)
{
	return (value & flag);
}

void CCallHook::setNops()
{
	rasm->setWriteOffset(hook_offset);
	uint32_t sz = 114 + _size;
	if (sz < 119)
		sz = 119;
	for (uint32_t i = 0; i < sz + _stack * 8; ++i) rasm->nop();
}

void CCallHook::ModOriginalBytesStart(uint32_t offset) {
	// call and jmp (long)
	if (orig_bytes[0] == 0xE9 || orig_bytes[0] == 0xE8) {
		//		if (orig_bytes[0] == 0xE8)
		//			_stackOffset += 4;
		byteValue<uint32_t> v;
		v.bytes[0] = orig_bytes[1];
		v.bytes[1] = orig_bytes[2];
		v.bytes[2] = orig_bytes[3];
		v.bytes[3] = orig_bytes[4];
		uint32_t o_addr = v.value + (reinterpret_cast<uint32_t>(hook_addr) + 5);
		v.value = o_addr - (offset + 5);
		orig_bytes[1] = v.bytes[0];
		orig_bytes[2] = v.bytes[1];
		orig_bytes[3] = v.bytes[2];
		orig_bytes[4] = v.bytes[3];
	}
	// conditionals jmp's (long)
	else if (orig_bytes[0] == 0x0F) {
		if (orig_bytes[1] >= 0x81 && orig_bytes[1] <= 0x8F) {
			byteValue<uint32_t> v;
			v.bytes[0] = orig_bytes[2];
			v.bytes[1] = orig_bytes[3];
			v.bytes[2] = orig_bytes[4];
			v.bytes[3] = orig_bytes[5];
			uint32_t o_addr = v.value + (reinterpret_cast<uint32_t>(hook_addr) + 6);
			v.value = o_addr - (offset + 6);
			orig_bytes[2] = v.bytes[0];
			orig_bytes[3] = v.bytes[1];
			orig_bytes[4] = v.bytes[2];
			orig_bytes[5] = v.bytes[3];
		}
	}
}

void CCallHook::ModOriginalBytesEnd(uint32_t offset) {
	// call and jmp (long)
	if (orig_bytes[_size - 5] == 0xE9 || orig_bytes[_size - 5] == 0xE8) {
		byteValue<uint32_t> v;
		v.bytes[0] = orig_bytes[_size - 4];
		v.bytes[1] = orig_bytes[_size - 3];
		v.bytes[2] = orig_bytes[_size - 2];
		v.bytes[3] = orig_bytes[_size - 1];
		uint32_t o_addr = v.value + (reinterpret_cast<uint32_t>(hook_addr) + 5);
		v.value = offset - (o_addr + 5);
		orig_bytes[_size - 4] = v.bytes[0];
		orig_bytes[_size - 3] = v.bytes[1];
		orig_bytes[_size - 2] = v.bytes[2];
		orig_bytes[_size - 1] = v.bytes[3];
	}
	// conditionals jmp's (long)
	else if (orig_bytes[_size - 6] == 0x0F) {
		if (orig_bytes[_size - 5] >= 0x81 && orig_bytes[_size - 5] <= 0x8F) {
			byteValue<uint32_t> v;
			v.bytes[0] = orig_bytes[_size - 4];
			v.bytes[1] = orig_bytes[_size - 3];
			v.bytes[2] = orig_bytes[_size - 2];
			v.bytes[3] = orig_bytes[_size - 1];
			uint32_t o_addr = v.value + (reinterpret_cast<uint32_t>(hook_addr) + 6);
			v.value = offset - (o_addr + 6);
			orig_bytes[_size - 4] = v.bytes[0];
			orig_bytes[_size - 3] = v.bytes[1];
			orig_bytes[_size - 2] = v.bytes[2];
			orig_bytes[_size - 1] = v.bytes[3];
		}
	}
}
