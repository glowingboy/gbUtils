#include "logger.h"
#include <cassert>
#include <limits>

using gb::utils::logger;
using gb::utils::time;

logger::logger() :
#ifdef _MSC_VER
    _normal_color_code(0),
    _log_color_code(GB_LOGGER_DEFAULT_LOG_MS_COLOR_CODE),
    _error_color_code(GB_LOGGER_DEFAULT_ERROR_MS_COLOR_CODE),
    _warning_color_code(GB_LOGGER_DEFAULT_WARNING_MS_COLOR_CODE),
    _progress_color_code{
    GB_LOGGER_DEFAULT_PROGRESS_MS_COLOR_CODE,
	GB_LOGGER_DEFAULT_PROGRESS_BAR_MS_COLOR_CODE },
//#elif __GNUC__
#else
    _normal_color_code(GB_LOGGER_COLOR_BACKTONORMAL),
    _log_color_code(GB_LOGGER_COLOR_BEGIN GB_LOGGER_DEFAULT_LOG_COLOR_CODE GB_LOGGER_COLOR_END),
    _error_color_code(GB_LOGGER_COLOR_BEGIN GB_LOGGER_DEFAULT_ERROR_COLOR_CODE GB_LOGGER_COLOR_END),
    _warning_color_code(GB_LOGGER_COLOR_BEGIN GB_LOGGER_DEFAULT_WARNING_COLOR_CODE GB_LOGGER_COLOR_END),
    _progress_color_code{
	GB_LOGGER_COLOR_BEGIN GB_LOGGER_DEFAULT_PROGRESS_COLOR_CODE GB_LOGGER_COLOR_END,
	    GB_LOGGER_COLOR_BEGIN GB_LOGGER_DEFAULT_PROGRESS_BAR_COLOR_CODE GB_LOGGER_COLOR_END },
#endif
    _log_default_streambuf(std::cout.rdbuf()),
    _error_default_streambuf(std::cerr.rdbuf()),
    _progress_bar_width(GB_LOGGER_DEFAULT_PROGRESS_BAR_WIDTH),
    _progress_total_width(GB_LOGGER_DEFAULT_PROGRESS_TOTAL_WIDTH),
    _bProgressing(false),
    _bEnableColor(true)
	    {
		_progress_flexible_width = _progress_total_width - (_progress_bar_width + (std::uint8_t)strlen(GB_LOGGER_DEFAULT_PROGRESS_FIXED_CHARS));
#ifdef _MSC_VER
		_hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::memset(&_preConsoleAttrib, 0, sizeof(PCONSOLE_SCREEN_BUFFER_INFO));
		::GetConsoleScreenBufferInfo(_hConsole, &_preConsoleAttrib);
#endif
	    }

logger::~logger()
{
    if (_bEnableColor)
    {
#ifdef _MSC_VER
	::SetConsoleTextAttribute(_hConsole, _preConsoleAttrib.wAttributes);
//#elif __GNUC__
#else
	std::cout << _normal_color_code;
#endif
    }
}

#ifdef GB_UTILS_MULTI_THREADS
#define _GB_UTILS_LOGGER_LOCK			\
    std::lock_guard<std::mutex> lck(_mtx);
#else
#define _GB_UTILS_LOGGER_LOCK
#endif

void logger::enable_color(bool bState)
{
    _GB_UTILS_LOGGER_LOCK;
    _bEnableColor = bState;
}

void logger::set_log_streambuf(std::streambuf* streambuf)
{
    _GB_UTILS_LOGGER_LOCK;
    std::cout.rdbuf(streambuf != nullptr ? streambuf : _log_default_streambuf);
}

void logger::set_error_streambuf(std::streambuf* streambuf)
{
    _GB_UTILS_LOGGER_LOCK;
    std::cerr.rdbuf(streambuf != nullptr ? streambuf : _error_default_streambuf);
}

#ifdef _MSC_VER

#define _gb_fancy_print(ostream, title, win_color_code)		\
    _GB_UTILS_LOGGER_LOCK;					\
    assert(!_bProgressing);					\
    assert(szMsg != nullptr);					\
    GB_GET_LOCALTIME(timeBuf);					\
    if(_bEnableColor)						\
	::SetConsoleTextAttribute(_hConsole, win_color_code);	\
    ostream << timeBuf << '\n'					\
    << title << szMsg << std::endl;

//#elif __GNUC__
#else

#define _gb_fancy_print(ostream, title, color_code)	\
    _GB_UTILS_LOGGER_LOCK;				\
    assert(!_bProgressing);				\
    assert(szMsg != nullptr);				\
    GB_GET_LOCALTIME(timeBuf);				\
    if(_bEnableColor)					\
	ostream << color_code;				\
    ostream << timeBuf << '\n'				\
    << title << szMsg;					\
    ostream << std::endl;	

#endif

void logger::log(const char* szMsg)const
{
    _gb_fancy_print(std::cout, "LOG: ", _log_color_code);
}

void logger::set_log_color_code(const color_code_t szCode)
{
    _GB_UTILS_LOGGER_LOCK;
#ifdef _MSC_VER
    _log_color_code = szCode;
//#elif __GNUC__
#else
    _log_color_code = GB_LOGGER_COLOR_BEGIN + szCode + GB_LOGGER_COLOR_END;
#endif
}

void logger::error(const char * szMsg)const
{
    _gb_fancy_print(std::cerr, "ERROR: ", _error_color_code);
}
void logger::set_error_color_code(const color_code_t szCode)
{
    _GB_UTILS_LOGGER_LOCK;
#ifdef _MSC_VER
    _error_color_code = szCode;
//#elif __GNUC__
#else	
    _error_color_code = GB_LOGGER_COLOR_BEGIN + szCode + GB_LOGGER_COLOR_END;
#endif
}
void logger::warning(const char* szMsg)const
{
    _gb_fancy_print(std::cout, "WARNING: ", _warning_color_code);
}

void logger::set_warning_color_code(const color_code_t szCode)
{
    _GB_UTILS_LOGGER_LOCK;
#ifdef _MSC_VER
    _warning_color_code = szCode;
//#elif __GNUC__
#else	
    _warning_color_code = GB_LOGGER_COLOR_BEGIN + szCode + GB_LOGGER_COLOR_END;
#endif
}

void logger::progress(const float value, const char* title)
{
    _GB_UTILS_LOGGER_LOCK;
    assert(value >= 0.0f && value <= 1.0f);
    _bProgressing = true;

    static std::uint64_t preTime = 0;
    const std::uint64_t curTime = time::Instance().timestamp();
    static constexpr std::uint8_t etaCount = 5;
    static std::uint64_t eta[etaCount] = {0};

    string strEta;
    std::uint8_t widthLeft;
    if (value < 1.0f)
    {
	if (curTime - preTime > 1000)//take sample per second
	{
	    static float preValue = 0;
	    if (preValue != 0)
	    {
		const float speed = value - preValue;
		static std::uint8_t eta_idx = 0;
		eta_idx++;
		std::uint64_t& curEta = eta[eta_idx % etaCount];
		if (speed > 0.0f)
		    curEta = (std::uint64_t)((1.0f - value) / speed);
		else
		    curEta = 0;
	    }
	    preValue = value;
	    preTime = curTime;
	}

	std::uint64_t average_eta = 0;
	std::uint8_t valid_count = 0;
	for (int i = 0; i < etaCount; i++)
	{
	    std::uint64_t e = eta[i];
		
#ifdef _MSC_VER
#undef max
#endif
	    assert((std::numeric_limits<std::uint64_t>::max() - average_eta) > e);
	    average_eta += e;

	    valid_count++;
	}

	if (valid_count != 0)
	    average_eta = average_eta / valid_count;
	else
	    average_eta = 0;
	strEta = "ETA: " + time::Instance().format(average_eta);
	widthLeft = _progress_flexible_width - (std::uint8_t)(strEta.length()) - 3/*anim ...*/;
    }
    else
	widthLeft = _progress_flexible_width;

    std::uint8_t lenTitle;
    if (title != nullptr)
	lenTitle = (std::uint8_t)strlen(title);
    else
	lenTitle = 0;
    assert(widthLeft > lenTitle);
    const std::uint8_t paddingWidth = widthLeft - lenTitle;
    //    if(paddingWidth)

    const color_code_t& c0 = _progress_color_code[0];
    const color_code_t& c1 = _progress_color_code[1];
#ifdef _MSC_VER
    ::SetConsoleTextAttribute(_hConsole, c0);
//#elif __GNUC__
#else	
    std::cout << c0;
#endif
    std::cout << ">>>";
    if (title != nullptr)
	std::cout << title;
    std::cout << '[';
#ifdef _MSC_VER
    ::SetConsoleTextAttribute(_hConsole, c1);
//#elif __GNUC__
#else	
    std::cout << c1;
#endif

    const std::uint8_t width = (std::uint8_t)(value * _progress_bar_width);
    for (int i = 0; i < width; i++)
	std::cout << ' ';

#ifdef _MSC_VER
    ::SetConsoleTextAttribute(_hConsole, c0);
//#elif __GNUC__
#else	
    std::cout << c0;
#endif

    const std::uint8_t width_left = _progress_bar_width - width;
    for (int i = 0; i < width_left; i++)
	std::cout << ' ';

    std::cout << ']' << int(value * 100) << '%' << ' ';

    //anim
    static const char anim[6][4] = { {'.', ' ', ' ', '\0'},
				     {' ', '.', ' ', '\0'},
				     {' ', ' ', '.', '\0'},
				     {'.', ' ', '.', '\0'},
				     {' ', '.', '.', '\0'},
				     {'.', '.', '.', '\0'} };
    static unsigned char count = 0;
    count++;

    if (value < 1.0f)
	std::cout << strEta << anim[count % 6];
    for (int i = 0; i < paddingWidth; i++)
	std::cout << ' ';
    std::cout << '\r';

    std::cout.flush();
}

void logger::progress_done()
{
    _GB_UTILS_LOGGER_LOCK;
    std::cout << std::endl;
    _bProgressing = false;
}
void logger::set_progress_width(const std::uint8_t barWidth, const std::uint8_t totalWidth)
{
    _GB_UTILS_LOGGER_LOCK;
    _progress_bar_width = barWidth;
    _progress_total_width = totalWidth;
    _progress_flexible_width = _progress_total_width - (_progress_bar_width + (std::uint8_t)(strlen(GB_LOGGER_DEFAULT_PROGRESS_FIXED_CHARS)));
}

// #ifdef gbLUAAPI
// void logger::gb_LC_Reg(lua_State* L)
// {
//     gb_LC_Func_Def;
//     gb_LC_Func_push("log", &log::gb_LC_EF_log);
//     gb_LC_Func_push("error", &log::gb_LC_EF_error);
//     gb_LC_Func_push("warning", &log::gb_LC_EF_warning);

//     gbLuaCPP_PrvCns<logger>::Register(L, "logger", funcs);

//     gb_LC_Singleton_Instance_Reg(log);
// }
// #endif
