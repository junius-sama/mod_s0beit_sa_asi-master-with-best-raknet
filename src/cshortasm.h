#ifndef CSHORTASM_H
#define CSHORTASM_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>

#define CALLHOOK void __stdcall

typedef enum r86 { EAX = 0, ECX, EDX, EBX, ESP, EBP, ESI, EDI } r86;

template<typename T> union byteValue {
	T value;
	uint8_t bytes[sizeof(T)];
};

class CShortAsm {
public:
	CShortAsm(uint32_t pages = 1);
	~CShortAsm();

	void insert(uint8_t* array, uint32_t size);
	void insert(std::vector<uint8_t> array);
	void push(uint8_t value);
	void push(int value);
	void push(r86 r);
	void pushad();
	void pushfd();
	void pop(r86 r);
	void popad();
	void popfd();
	void label(std::string label);
	void jmp(int addr);
	void jmp(std::string label);
	void jmp(r86 r);
	void ret(int pops = 0);
	void nop();
	void call(int addr);
	void call(std::string label);
	void call(r86 r);
	void mov(int& var, r86 r);				  // mov var, EAX
	void mov(r86 r, int& var);				  // mov EAX, var
	void mov(r86 r, const int value);			  // mov EAX, value
	void mov(r86 r1, r86 r2);					  // mov EAX, ECX
	void mov(r86 r1, r86 r2, uint8_t offset);   // mov EAX, [EAX + 4]
	void mov(r86 r1, uint8_t offset, r86 r2);   // mov [EAX + 4], EAX
	void xchg(r86 r1, r86 r2, uint8_t offset = 0); // swap registers
	void xchg(r86 r, int& var);
	void add(r86 r, uint8_t value);
	void add(r86 r, const int value);
	void add(r86 r, int& var);
	void sub(r86 r, uint8_t value);
	void sub(r86 r, const int value);
	void sub(r86 r, int& var);
	void mul(r86 r);
	void imul(r86 r);
	void imul(r86 r1, r86 r2);
	void imul(r86 r, uint8_t value);
	void imul(r86 r, const int value);
	void imul(r86 r1, r86 r2, uint8_t value);
	void imul(r86 r1, r86 r2, const int value);
	void div(r86 r);
	void idiv(r86 r);
	void cmp(r86 r1, r86 r2);
	void cmp(r86 r, uint8_t value);
	void cmp(r86 r, const int value);
	void XOR(r86 r1, r86 r2);
	void XOR(r86 r, uint8_t value);
	void XOR(r86 r, const int value);
	void je(const int addr);
	void je(std::string label);
	void jne(const int addr);
	void jne(std::string label);
	void jl(const int addr);
	void jl(std::string label);
	void jle(const int addr);
	void jle(std::string label);
	void jg(const int addr);
	void jg(std::string label);
	void jge(const int addr);
	void jge(std::string label);
	void jb(const int addr);
	void jb(std::string label);
	void jbe(const int addr);
	void jbe(std::string label);
	void ja(const int addr);
	void ja(std::string label);
	void jae(const int addr);
	void jae(std::string label);

	void resetWriteOffset();
	uint32_t getWriteOffset();
	void setWriteOffset(uint32_t offset);

	const uint8_t* getAddr();
	uint32_t getSize();

	static uint32_t arrayToPages(size_t size_array);
	static const uint32_t PAGE_SIZE = 4096;

protected:
	void resize(uint32_t pages);
	uint32_t getRelativeAddress(uint32_t addr);
	void write(uint8_t v);
	void* myalloc(size_t size);
	void myfree(void* ptr, size_t size);

	uint8_t* _code = nullptr;
	uint32_t _size = 0;

private:
	uint32_t _offset = 0;
	uint32_t _peak = 0;
	std::map<std::string, uint32_t> _labels;
	std::map<std::string, uint32_t> _jmps;
	std::map<std::string, uint32_t> _jes;
	std::map<std::string, uint32_t> _jnes;
	std::map<std::string, uint32_t> _jls;
	std::map<std::string, uint32_t> _jles;
	std::map<std::string, uint32_t> _jgs;
	std::map<std::string, uint32_t> _jges;
	std::map<std::string, uint32_t> _jbs;
	std::map<std::string, uint32_t> _jbes;
	std::map<std::string, uint32_t> _jas;
	std::map<std::string, uint32_t> _jaes;
};

#endif // CSHORTASM_H
