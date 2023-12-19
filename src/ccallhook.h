#ifndef CCALLHOOK_H
#define CCALLHOOK_H

#include "cshortasm.h"
#include "memsafe.h"
#include "fn2void.hpp"

enum eCodePos {
	cp_skip = 0,
	cp_before,
	cp_after
};

enum eSafeCall {
	sc_not = 0,
	sc_registers,
	sc_flags,
	sc_all
};

enum eEntry {
	e_jmp = 0,
	e_call
};

enum eFuncType {
	ft_callback = 0,
	ft_method
};

/**
 * \brief Класс хуков
 *
 * Пример:
 *
 * Есть код (создание диалога от сервера)
 * \code
 * .text:1000CD3F		push    1
 * .text:1000CD41		lea     ecx, [esp+1*8h+arg_214]
 * .text:1000CD48		push    ecx
 * .text:1000CD49		lea     edx, [esp+1Ch+arg_31C]
 * .text:1000CD50		push    edx
 * .text:1000CD51		movzx   edx, [esp+20h+var_D]
 * .text:1000CD56		lea     eax, [esp+20h+arg_424]
 * .text:1000CD5D		push    eax
 * .text:1000CD5E		movsx   eax, [esp+24h+var_C]
 * .text:1000CD63		lea     ecx, [esp+24h+arg_10C]
 * .text:1000CD6A		push    ecx
 * .text:1000CD6B		mov     ecx, dword_1021A0B8
 * .text:1000CD71		push    edx
 * .text:1000CD72		push    eax
 * .text:1000CD73		call    sub_1006B9C
 * \endcode
 *
 * Мы хотим перехватить вызов и изменить параметры (дописать id диалогу),
 * тогда мы создадим хук:
 * \code
 * hook = new CCallHook((void*)(g_handle.dwSAMP + 0xCD73),    // Адрес хука - хукаем вызов команды
 * 						5,								   // Длина оригинального кода
 * 						3								   // Количество аргументов, которые мы возьмем со стека
 * 						);								   // Остальные аргументы имеют значения по умолчанию
 * hook->setStackOffset(0);								   // Смещение от начала стека - 0 байт
 * hook->enable(DXUT_CreateDialog);						   // Включаем хук на функцию DXUT_CreateDialog
 * \endcode
 *
 * Рассмотри функцию DXUT_CreateDialog:
 * \code
 * void DXUT_CreateDialog(int &id, int &type, char *&title)
 * {
 * 	static char dxut_title[0xFF];
 * 	title[0xF6] = 0;
 * 	strcpy(dxut_title, title);
 * 	sprintf(title, "[%d] %s", id, dxut_title);
 * }
 * \endcode
 * Заметим очень важный момент - все аргументы хуку переданны в виде ссылок, хотя оригинальной функции они передавались по значению!
 */
class CCallHook
{
public:
	/**
	 * \brief Конструктор
	 * \param addr адрес, который будет хукаться
	 * \param size количество байт оригинального кода
	 * \param stack количество переменных, которые будут браться со стека
	 * \param pos позиция оригинального кода в хуке (до хука, после хука, нигде)
	 * \param save сохранение данных перед хуком (значение регистров и/или флагов)
	 * \param entry способ вхождения в хук (call или jmp). Выбор может игнорироваться из-за особенностей других параметров
	 */
	CCallHook(void* addr, uint32_t size = 5, uint32_t stack = 0, eCodePos pos = cp_after,
		eSafeCall save = sc_all, eEntry entry = e_call);
	virtual ~CCallHook();

	/**
	 * \brief Указание функции хука
	 * \param func функция, которую будет вызывать хук
	 */
	void setFunc(void(__stdcall* func)());
	/**
	 * \brief Включение хука, с указанием функции
	 * \param func функция, вызываемая хуком
	 */
	void enable(void* func);
	/**
	 * \brief Включение хука, с указанием метода класса
	 * \param obj указатель на класс
	 * \param method указатель на метод
	 */
	void enable(void* obj, void* method);
	/**
	 * \brief Включение хука
	 */
	void enable();
	/**
	 * \brief Выключение хука
	 * \detail Хук не удаляется, и его можно потом включить обратно
	 */
	uint32_t disable();

	/**
	 * \brief Указание функции хука
	 * \param func функция, которую будет вызывать хук
	 */
	template<class ... Args>
	void setFunc(void(__stdcall* func)(Args ...)) {
		_func = fn2void(func);
		_funcType = eFuncType::ft_callback;
	}
	/**
	 * \brief Включение хука, с указанием функции
	 * \param func функция, вызываемая хуком
	 */
	template<class ... Args>
	void enable(void(__stdcall* func)(Args ...)) {
		setFunc(func);
		enable(_func);
	}
	/**
	 * \brief Указание метода класса
	 * \param obj указатель на класс
	 * \param method указатель на метод
	 */
	template<class C, class ... Args>
	void setFunc(C* obj, void(C::* method)(Args ...)) {
		_object = reinterpret_cast<void*>(obj);
		_func = fn2void(method);
		_funcType = eFuncType::ft_method;
	}
	/**
	 * \brief Включение хука, с указанием метода класса
	 * \param obj указатель на класс
	 * \param method указатель на метод
	 */
	template<class C, class ... Args>
	void enable(C* obj, void(C::* method)(Args ...)) {
		setFunc(obj, method);
		enable(_object, _func);
	}

	/**
	 * \brief Указание смещения от начала стека, для передачи аргументов
	 * \detail Например, по умолчанию стоит смещение 4 байта, что бы исключить адрес возврата из списка аргументов
	 * \param so количество байт, которые будут пропущены
	 */
	void setStackOffset(uint32_t so);
	/**
	 * \brief Получение смещения от начала стека, для передачи аргументов
	 * \return количество байт, которые будут пропущены
	 */
	uint32_t stackOffset();

	/**
	 * \brief Перезаписывает значения регистров на ввыходе из хука
	 * \detail Игнорирует флаг save указанный в конструкторе
	 * \param r регистр, значение которого будет перезаписано
	 * \param value значение, которое будет записано
	 */
	void setReg86(r86 r, uint32_t value);
	/**
	 * \brief Получает значение регистра
	 * \param r реегистр
	 */
	uint32_t reg86(r86 r);

protected:
	byte* orig_bytes;
	uint32_t _size;
	uint32_t _stack;
	uint32_t _stackOffset;
	uint32_t hook_offset;
	void* hook_addr;
	eCodePos hook_pos;
	void* _func;
	void* _object;
	eFuncType _funcType;
	eEntry _entry;
	eSafeCall _save;
	uint32_t _registers[8];

	template<typename T>
	bool checkFlag(T value, T flag);

private:
	CShortAsm* rasm;

	void setNops();
	void ModOriginalBytesStart(uint32_t offset);
	void ModOriginalBytesEnd(uint32_t offset);
};

class CJmpHook : public CCallHook {
public:
	CJmpHook(void* addr, uint32_t size = 5, uint32_t stack = 0, eCodePos pos = cp_after,
		eSafeCall save = sc_all, eEntry entry = e_jmp)
		: CCallHook(addr, size, stack, pos, save, entry) {
		_stackOffset = 0;
	}
};

#endif // CCALLHOOK_H
