#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <Windows.h>
#include <algorithm>
#include <fstream>

namespace JWEngine
{
	#define THREAD_LOGGER_DECL JWEngine::JWLogger thread_logger{}
	#define THREAD_LOGGER thread_logger

	#define THREAD_LOG(thread_id, comment, LogLevel) thread_logger.Log(__FILE__, __LINE__, thread_id, comment, LogLevel)
	#define THREAD_LOG_D(thread_id, comment) THREAD_LOG(thread_id, comment, JWEngine::ELogLevel::Default)
	#define THREAD_LOG_I(thread_id, comment) THREAD_LOG(thread_id, comment, JWEngine::ELogLevel::Important)
	#define THREAD_LOG_W(thread_id, comment) THREAD_LOG(thread_id, comment, JWEngine::ELogLevel::Warning)
	#define THREAD_LOG_F(thread_id, comment) THREAD_LOG(thread_id, comment, JWEngine::ELogLevel::Fatal)

	#define THREAD_LOGGER_SEND_OUT(out) if (out) { *out = thread_logger; }

	#define THREAD_LOG_METHOD_START(thread_id) THREAD_LOG(thread_id, (std::string(typeid(*this).name()) + "::" + __func__ + "() Start").c_str(), JWEngine::ELogLevel::Default)
	#define THREAD_LOG_METHOD_END(thread_id) THREAD_LOG(thread_id, (std::string(typeid(*this).name()) + "::" + __func__ + "() End").c_str(), JWEngine::ELogLevel::Default)

	#define THREAD_LOG_FREE_FUNC_START(thread_id) THREAD_LOG(thread_id, (std::string(__func__) + "() Start").c_str(), JWEngine::ELogLevel::Default)
	#define THREAD_LOG_FREE_FUNC_END(thread_id) THREAD_LOG(thread_id, (std::string(__func__) + "() End").c_str(), JWEngine::ELogLevel::Default)

	#define THREAD_LOGGER_DECL_METHOD_START(thread_id) THREAD_LOGGER_DECL; THREAD_LOG_METHOD_START(thread_id)
	#define THREAD_LOGGER_SEND_OUT_METHOD_END(thread_id, out) THREAD_LOG_METHOD_END(thread_id); THREAD_LOGGER_SEND_OUT(out)

	#define THREAD_LOGGER_DECL_FREE_FUNC_START(thread_id) THREAD_LOGGER_DECL; THREAD_LOG_FREE_FUNC_START(thread_id)
	#define THREAD_LOGGER_SEND_OUT_FREE_FUNC_END(thread_id, out) THREAD_LOG_FREE_FUNC_START(thread_id); THREAD_LOGGER_SEND_OUT(out)

	#define JOIN_THREAD_LOG(thread_logger) ex_logger.JoinLog(thread_logger)

	#define GLOBAL_LOGGER_DECL extern JWEngine::JWLogger ex_logger{}
	#define GLOBAL_LOGGER_USE extern JWEngine::JWLogger ex_logger
	#define GLOBAL_LOGGER ex_logger

	#define GLOBAL_LOG(comment, LogLevel) ex_logger.LogPrint(__FILE__, __LINE__, 0, comment, LogLevel)
	#define GLOBAL_LOG_D(comment) GLOBAL_LOG(comment, JWEngine::ELogLevel::Default)
	#define GLOBAL_LOG_I(comment) GLOBAL_LOG(comment, JWEngine::ELogLevel::Important)
	#define GLOBAL_LOG_W(comment) GLOBAL_LOG(comment, JWEngine::ELogLevel::Warning)
	#define GLOBAL_LOG_F(comment) GLOBAL_LOG(comment, JWEngine::ELogLevel::Fatal)

	#define GLOBAL_LOG_METHOD_START GLOBAL_LOG((std::string(typeid(*this).name()) + "::" + __func__ + "() Start").c_str(), JWEngine::ELogLevel::Default)
	#define GLOBAL_LOG_METHOD_END GLOBAL_LOG((std::string(typeid(*this).name()) + "::" + __func__ + "() End").c_str(), JWEngine::ELogLevel::Default)
	
	#define GLOBAL_LOG_FREE_FUNC_START GLOBAL_LOG((std::string(__func__) + "() Start").c_str(), JWEngine::ELogLevel::Default)
	#define GLOBAL_LOG_FREE_FUNC_END GLOBAL_LOG((std::string(__func__) + "() End").c_str(), JWEngine::ELogLevel::Default)

	static constexpr char KLogHead[]{ "        date        |    time    |  thread  |        file       :line\t| lev | comment  \n" };
	static constexpr int KSZFileLen{ 20 };

	enum class ELogLevel
	{
		Default,
		Important,
		Warning,
		Fatal,
	};

	struct SLogDatum
	{
		SLogDatum() {};
		SLogDatum(long long _time, ELogLevel _level, const std::string& _log) : time{ _time }, level{ _level }, log{ _log } {};

		long long time{};
		ELogLevel level{};
		std::string log{};
		bool is_printed{ false };
	};

	static bool SortLogLess(const SLogDatum& a, const SLogDatum& b)
	{
		return (a.time < b.time);
	}

	class JWLogger
	{
	public:
		void PrintHead()
		{
			printf(KLogHead);
		}

		void LogPrint(const char* file, int line, int thread_id, const char* comment, ELogLevel level = ELogLevel::Default)
		{
			Log(file, line, thread_id, comment, level);

			auto& current_datum = m_data[m_data.size() - 1];
			
			switch (current_datum.level)
			{
			case JWEngine::ELogLevel::Default:
				SetConsoleTextAttribute(m_hConsole, FOREGROUND_INTENSITY);
				break;
			case JWEngine::ELogLevel::Important:
				SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN);
				break;
			case JWEngine::ELogLevel::Warning:
				SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
				break;
			case JWEngine::ELogLevel::Fatal:
				SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED);
				break;
			default:
				break;
			}

			printf(current_datum.log.c_str());

			current_datum.is_printed = true;
		}

		void Log(const char* file, int line, int thread_id, const char* comment, ELogLevel level = ELogLevel::Default)
		{
			char sz_date[200]{};
			auto time_now{ m_clock.to_time_t(m_clock.now()) };
			std::tm tm{};
			localtime_s(&tm, &time_now);
			strftime(sz_date, 200, "%Y-%m-%d %T", &tm);
			sz_date[strlen(sz_date)] = NULL;

			auto ll_time = m_clock.now().time_since_epoch().count();

			std::string str_file{ file };
			str_file = str_file.substr(str_file.find_last_of('\\') + 1);

			char sz_file[KSZFileLen]{};
			memset(sz_file, ' ', KSZFileLen);
			for (size_t i = 0; i < ((str_file.size() > KSZFileLen - 2) ? KSZFileLen - 2 : str_file.size()); ++i)
			{
				sz_file[i] = str_file[i];
			}
			sz_file[KSZFileLen - 2] = ':';
			sz_file[KSZFileLen - 1] = '\0';

			char sz_level[4]{};
			switch (level)
			{
			case JWEngine::ELogLevel::Default:
				strcpy_s(sz_level, "---");
				break;
			case JWEngine::ELogLevel::Important:
				strcpy_s(sz_level, "IMP");
				break;
			case JWEngine::ELogLevel::Warning:
				strcpy_s(sz_level, "WRN");
				break;
			case JWEngine::ELogLevel::Fatal:
				strcpy_s(sz_level, "FTL");
				break;
			default:
				break;
			}
			
			char temp_str[300]{};
			sprintf_s(temp_str, "%s | %lld | thread:%d | %s%d\t| %s | %s\n",
				sz_date, ll_time % 10000000000, thread_id, sz_file, line, sz_level, comment);

			m_data.emplace_back(ll_time, level, temp_str);
		};

		void JoinLog(const JWLogger& thread_log) 
		{
			for (const auto& iter : thread_log.m_data)
			{
				m_data.emplace_back(iter);
			}

			std::sort(m_data.begin(), m_data.end(), SortLogLess);
		};

		void DisplayNonPrintedLog()
		{
			printf(KLogHead);
			for (const auto& iter : m_data)
			{
				if (iter.is_printed)
				{
					continue;
				}

				switch (iter.level)
				{
				case JWEngine::ELogLevel::Default:
					SetConsoleTextAttribute(m_hConsole, FOREGROUND_INTENSITY);
					break;
				case JWEngine::ELogLevel::Important:
					SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN);
					break;
				case JWEngine::ELogLevel::Warning:
					SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
					break;
				case JWEngine::ELogLevel::Fatal:
					SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED);
					break;
				default:
					break;
				}

				printf("%s", iter.log.c_str());
			}
		};

		void SaveToFile(const std::string& FileName)
		{
			std::ofstream ofs{ FileName.c_str() };

			if (ofs.is_open())
			{
				ofs << KLogHead;

				for (const auto& iter : m_data)
				{
					ofs << iter.log;
				}

				ofs.close();
			}
		};

	private:
		std::vector<SLogDatum> m_data{};
		std::chrono::system_clock m_clock{};
		HANDLE m_hConsole{ GetStdHandle(STD_OUTPUT_HANDLE) };
	};
};