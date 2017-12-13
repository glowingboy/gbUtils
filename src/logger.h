#pragma once
#include <iostream>
#include <string>
#include "time.h" 

#ifdef gbLUAAPI
#include "luacpp.h"
#endif

#ifdef _MSC_VER
#include<Windows.h>

// colors are 0=black 1=blue 2=green ... 9=light blue 10=light green ...and so on to 15=white  
// colorattribute = foreground + background * 16
// to get red text on yellow use 4 + 14*16 = 228
// light red on yellow would be 12 + 14*16 = 236
#define GB_LOGGER_DEFAULT_LOG_MS_COLOR_CODE 15
#define GB_LOGGER_DEFAULT_ERROR_MS_COLOR_CODE 12
#define GB_LOGGER_DEFAULT_WARNING_MS_COLOR_CODE 14
#define GB_LOGGER_DEFAULT_PROGRESS_MS_COLOR_CODE 9
#define GB_LOGGER_DEFAULT_PROGRESS_BAR_MS_COLOR_CODE 160

#elif __GNUC__

// ref https://en.wikipedia.org/wiki/ANSI_escape_code#Windows_and_DOS
#define GB_LOGGER_COLOR_BEGIN "\033["
#define GB_LOGGER_COLOR_END "m"
#define GB_LOGGER_COLOR_BACKTONORMAL "\033[0m"

#define GB_LOGGER_DEFAULT_LOG_COLOR_CODE "1;37;40"
#define GB_LOGGER_DEFAULT_ERROR_COLOR_CODE "1;31;40"
#define GB_LOGGER_DEFAULT_WARNING_COLOR_CODE "1;33;40"
#define GB_LOGGER_DEFAULT_PROGRESS_COLOR_CODE "1;34;40"
#define GB_LOGGER_DEFAULT_PROGRESS_BAR_COLOR_CODE "1;32;42"
#endif

/*
 *TODO:
 *1.multi-threads support
 *2.permanent store
 */

#define GB_LOGGER_DEFAULT_PROGRESS_BAR_WIDTH 20
#define GB_LOGGER_DEFAULT_PROGRESS_TOTAL_WIDTH 64

//len 10
#define GB_LOGGER_DEFAULT_PROGRESS_FIXED_CHARS ">>>[]100% "
namespace gb
{
    namespace utils
    {
	GB_UTILS_CLASS  logger
	{
#ifdef _MSC_VER
		typedef WORD color_code_t;
#elif __GNUC__
		typedef std::string color_code_t;
#endif
	    GB_SINGLETON_EXCLUDECTOR(logger);
	    logger();
	    ~logger();
	public:
	    void log(const char* msg)const;
	    void set_log_color_code(const color_code_t szCode);
	    /*
	     *@param streambuf, if streambuf == nullptr,
	     *then set back to default streambuf.
	     */
	    void set_log_streambuf(std::streambuf* streambuf);
	    void error(const char* msg)const;
	    void set_error_color_code(const color_code_t szCode);
	    void set_error_streambuf(std::streambuf* streambuf);
	    void warning(const char* msg)const;
	    void set_warning_color_code(const color_code_t szCode);

	    /*
	     *@brief, progress print, string printed will be like this ">>>tile[]100% ETA: 1s...",
	     *thread unsafe, will disable all other logger function until progress_done()
	     */
	    void progress(const float value, const char* title = nullptr);
	    void progress_done();
	    
	    void set_progress_width(const std::uint8_t barWidth, const std::uint8_t totalWidth);

	    inline void enable_color(bool bState = true)
	    {
		_bEnableColor = bState;
	    }
	private:
#ifdef _MSC_VER
#elif __GNUC__
#endif
		color_code_t _normal_color_code;
		color_code_t _log_color_code;
		color_code_t _error_color_code;
		color_code_t _warning_color_code;
		color_code_t _progress_color_code[2];
	    
	    /*
	     *log and waring share same streambuf
	     */
	    std::streambuf* _log_default_streambuf;
	    std::streambuf* _error_default_streambuf;

	    std::uint8_t _progress_bar_width;
	    std::uint8_t _progress_total_width;
	    std::uint8_t _progress_flexible_width;
	    bool _bProgressing;
	    bool _bEnableColor;
#ifdef _MSC_VER
	    HANDLE _hConsole;
	    CONSOLE_SCREEN_BUFFER_INFO _preConsoleAttrib;
#endif
	    
#ifdef gbLUAAPI
	    gb_LC_EXPORT_FUNC(log);
	    gb_LC_EXPORT_FUNC(error);
	    gb_LC_EXPORT_FUNC(warning);
	    gb_LC_Register_PrvCns(log);
#endif

	};
	
    };
};
