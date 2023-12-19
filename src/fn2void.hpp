#ifndef FN2VOID_H
#define FN2VOID_H

template<typename R, typename... Args> void* fn2void(R(*fn)(Args...)) {
	struct f {
		decltype(fn) fn;
	} _{ fn };
	return *reinterpret_cast<void**>(&_);
}

#ifdef WIN32
template<typename R, typename... Args> void* fn2void(R(__stdcall* fn)(Args...)) {
#else
template<typename R, typename... Args> void* fn2void(R(__attribute__((stdcall)) * fn)(Args...)) {
#endif
	struct f {
		decltype(fn) fn;
	} _{ fn };
	return *reinterpret_cast<void**>(&_);
}

template<typename R, class C, typename... Args> void* fn2void(R(C:: * fn)(Args...)) {
	struct f {
		decltype(fn) fn;
	} _{ fn };
	return *reinterpret_cast<void**>(&_);
}

#endif // FN2VOID_H
