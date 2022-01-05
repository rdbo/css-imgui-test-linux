#include "module.hpp"
#include <dlfcn.h>
#include <link.h>

struct dl_iterate_cb_t {
	std::regex exp;
	const char *name;
	void *addr;
	size_t size;
};

static int dl_iterate_cb(dl_phdr_info *dl_info, size_t size, void *data)
{
	dl_iterate_cb_t *cbarg = reinterpret_cast<dl_iterate_cb_t *>(data);

	if (std::regex_match(dl_info->dlpi_name, cbarg->exp)) {
		cbarg->name = dl_info->dlpi_name;
		cbarg->addr = (void *)(dl_info->dlpi_addr + dl_info->dlpi_phdr[0].p_vaddr);
		cbarg->size = dl_info->dlpi_phdr[0].p_memsz;;
		return 1;
	}

	return 0;
}

Module::Module(std::string name)
{
	dl_iterate_cb_t cbarg = {
		std::regex(".*" + name),
		nullptr,
		nullptr,
		0
	};

	dl_iterate_phdr(dl_iterate_cb, reinterpret_cast<void *>(&cbarg));

	if (cbarg.addr) {
		this->name = std::string(cbarg.name);
		this->base = cbarg.addr;
		this->size = cbarg.size;
		this->handle = dlopen(cbarg.name, RTLD_NOLOAD | RTLD_NOW);
		dlclose(handle);
	}
}

std::string Module::GetName()
{
	return this->name;
}

void *Module::GetBase()
{
	return this->base;
}

size_t Module::GetSize()
{
	return this->size;
}

void *Module::GetHandle()
{
	return this->handle;
}

void *Module::FindSymbol(const char *symbol)
{
	return dlsym(this->handle, symbol);
}
