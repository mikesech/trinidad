#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/tee.hpp>
#include "URT/Log.h"
#include <boost/lexical_cast.hpp>

/** NCURSES STUFF **/
void startScreen();
void endScreen();
void updateStat(const std::string& title, const std::string& value);

struct ScreenGuard {
	ScreenGuard() { startScreen(); }
	~ScreenGuard() { endScreen(); }
};

/** NCURSES IOSTREAM STUFF **/
class ScreenLogDevice : public boost::iostreams::sink {
public:
	std::streamsize write(const char* s, std::streamsize n);
};
class ScreenLog : public boost::iostreams::stream<ScreenLogDevice> {
public:
	ScreenLog() : boost::iostreams::stream<ScreenLogDevice>((ScreenLogDevice())) {}
};
typedef boost::iostreams::stream<boost::iostreams::tee_device<ScreenLog, std::ostream> > ScreenCerrLog;

ScreenLog& getScreenLog();
ScreenCerrLog& getScreenCerrLog();
std::ostream& getAutoLog();

/** MISC. RELATED LOG STUFF **/
static inline void updateLog(const std::string& title, const std::string& value) {
	urt::Log::msg<<"\n\t"<<title<<": "<<value;
	updateStat(title, value);
}
template <typename T>
static inline void updateLog(const std::string& title, const T& value) {
	updateLog(title, boost::lexical_cast<std::string>(value));
}


#endif
