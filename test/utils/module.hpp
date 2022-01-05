#ifndef MODULE_HPP
#define MODULE_HPP

#include <iostream>
#include <string>
#include <regex>

class Module {
private:
	std::string name = "";
	void *base = nullptr;
	size_t size = 0;
	void *handle = nullptr;
public:
	Module(std::string name);
	std::string GetName();
	void *GetBase();
	size_t GetSize();
	void *GetHandle();
	void *FindSymbol(const char *symbol);
	template <typename T>
	inline T FindSymbol(const char *symbol)
	{
		return reinterpret_cast<T>(this->FindSymbol(symbol));
	}
};

#endif
