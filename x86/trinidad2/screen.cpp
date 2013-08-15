#include "screen.h"
#include <curses.h>
#include <string.h>
#include <boost/unordered_map.hpp>
#include <boost/iostreams/concepts.hpp>
#include <iostream>

/** NCURSES STUFF **/
struct InfoLine {
	std::string value;
	int row;
	
	InfoLine(const std::string& v = std::string(), int r = -1) : value(v), row(r) {}
};
typedef boost::unordered_map<std::string, InfoLine> LineMap;

static WINDOW *scrn, *lineWin, *logWin;
static LineMap lines;
static int numLines = 0;
static bool streamEnabled = false;

void drawBase() {
	//print window box
	clear();
	box(scrn, 0, 0);
	
	//print title
	attron(A_BOLD);
	mvaddstr(1, (COLS - 2 - strlen("Trinidad v2"))/2, "Trinidad v2");
	attroff(A_BOLD);
	refresh();
}

void startScreen() {
	scrn = initscr();
	noecho();
	nonl();
	cbreak();
	
	lineWin = newwin(1, COLS - 4, 3, 2);
	logWin = newwin(LINES - 6, COLS - 4, 5, 2);
	scrollok(logWin, TRUE);
	
	drawBase();
	streamEnabled = true;
}
void endScreen() {
	delwin(logWin);
	delwin(lineWin);
	endwin();
	streamEnabled = false;
}
void updateStat(const std::string& title, const std::string& value) {
	InfoLine& line = lines[title];
	line.value = value;

	if(line.row == -1) {
		line.row = numLines++;
		
		//rows are zero-indexed
		for(int numLogRows = getcury(logWin) + 1; numLogRows > LINES - 5 - numLines; numLogRows--)
			scroll(logWin);
		wresize(logWin, LINES - 5 - numLines, COLS - 4);
		mvwin(logWin, 4 + numLines, 2);
		wrefresh(logWin);
		
		wresize(lineWin, numLines + 1, COLS - 4);
		mvwprintw(lineWin, line.row, 0, "%s: %s", title.c_str(), value.c_str());
	} else {
		mvwaddstr(lineWin, line.row, title.size() + 2, value.c_str());
		wclrtoeol(lineWin);
	}
	wrefresh(lineWin);
}

/** NCURSES IOSTREAM STUFF **/
std::streamsize ScreenLogDevice::write(const char* s, std::streamsize n) {
	if(streamEnabled) {
		waddnstr(logWin, s, n);
		wrefresh(logWin);
	}
	return n;
}

ScreenLog& getScreenLog() {
	static ScreenLog sl;
	return sl;
}
ScreenCerrLog& getScreenCerrLog() {
	static boost::iostreams::tee_device<ScreenLog, std::ostream> md(getScreenLog(), std::cerr);
	static boost::iostreams::stream<boost::iostreams::tee_device<ScreenLog, std::ostream> > s(md);
	return s;
}
std::ostream& getAutoLog() {
	if(isatty(STDERR_FILENO))
		return getScreenLog();
	else
		return getScreenCerrLog();
}
