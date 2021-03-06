#pragma once
#include "string.h"
#include <fstream>

GB_UTILS_NS_BEGIN

	GB_UTILS_CLASS file
	{
	public:
	    file(const char* filePath, bool bRead = true);
	    ~file();
	    void read(void* const buffer, const size_t bufferSize);
	    void write(const void* data, const size_t size);
	    void close();
	    
	    bool eof()const { return _file.eof(); }
	    std::uint32_t size() const { return _size; }
	private:
	    std::fstream _file;
	    std::uint32_t _size;
	    string _filePath;
	};

GB_UTILS_NS_END
