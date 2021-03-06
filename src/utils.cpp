/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#include "utils.h"
#if defined(_WIN32)			// getcwd (unix) or __getcwd (win)
	#include <direct.h>
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include <cstdarg>
#include <sys/stat.h>   // stat (gDirExists)
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <sstream>
#include <limits.h>
#if defined(__APPLE__)
	#include <libgen.h>     // basename unix
#endif



bool gFileExists(const char *filename) {
	FILE *fh = fopen(filename, "rb");
	if (!fh) {
		return 0;
	}
	else {
		fclose(fh);
		return 1;
	}
}


/* ------------------------------------------------------------------ */


bool gIsDir(const char *path) {
	bool ret;

#if defined(__linux__)

	struct stat s1;
	stat(path, &s1);
	ret = S_ISDIR(s1.st_mode);

#elif defined(__APPLE__)

	if (strcmp(path, "")==0)
		ret = false;
	else {
		struct stat s1;
		stat(path, &s1);
		ret = S_ISDIR(s1.st_mode);

		/* check if ret is a bundle, a special OS X folder which must be
		 * shown as a regular file (VST).
		 * FIXME - consider native functions CFBundle... */

		if (ret) {
			std::string tmp = path;
			tmp += "/Contents/Info.plist";
			if (gFileExists(tmp.c_str()))
				ret = false;
		}
	}

#elif defined(__WIN32)

  unsigned dwAttrib = GetFileAttributes(path);
  ret = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif

	return ret & !gIsProject(path);
}


/* ------------------------------------------------------------------ */


bool gDirExists(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0 && errno == ENOENT)
		return false;
	return true;
}


/* ------------------------------------------------------------------ */


bool gMkdir(const char *path) {
#if defined(__linux__) || defined(__APPLE__)
	if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
#else
	if (_mkdir(path) == 0)
#endif
		return true;
	return false;
}


/* ------------------------------------------------------------------ */


std::string gBasename(const char *path) {
	std::string out = path;
	out.erase(0, out.find_last_of(gGetSlash().c_str())+1);
	return out;
}


/* ------------------------------------------------------------------ */


std::string gDirname(const char *path) {
	std::string out = path;
	out.erase(out.find_last_of(gGetSlash().c_str()));
	return out;
}


/* ------------------------------------------------------------------ */


std::string gGetCurrentPath() {
 char buf[PATH_MAX];
#if defined(__WIN32)
	if (_getcwd(buf, PATH_MAX) != NULL)
#else
	if (getcwd(buf, PATH_MAX) != NULL)
#endif
		return buf;
	else
		return "";
}


/* ------------------------------------------------------------------ */


std::string gGetExt(const char *file) {
	int len = strlen(file);
	int pos = len;
	while (pos>0) {
		if (file[pos] == '.')
			break;
		pos--;
	}
	if (pos==0)
		return "";
	std::string out = file;
	return out.substr(pos+1, len);
}


/* ------------------------------------------------------------------ */


std::string gStripExt(const char *file) {
	int len = strlen(file);
	int pos = -1;
	for (int i=0; i<len; i++)
		if (file[i] == '.') {
			pos = i;
			break;
		}
	std::string out = file;
	return pos == -1 ? out : out.substr(0, pos);
}


/* ------------------------------------------------------------------ */


bool gIsProject(const char *path) {

	/** FIXME - checks too weak */

	if (gGetExt(path) == "gprj" && gDirExists(path))
		return 1;
	return 0;
}


/* ------------------------------------------------------------------ */


bool gIsPatch(const char *path) {
	if (gGetExt(path) == "gptc")
		return 1;
	return 0;
}


/* ------------------------------------------------------------------ */


std::string gGetProjectName(const char *path) {
	std::string out;
	out = gStripExt(path);

	int i = out.size();
	while (i>=0) { /// TODO - use gGetSlash()
#if defined(__linux__) || defined(__APPLE__)
		if (out[i] == '/')
#elif defined(_WIN32)
		if (out[i] == '\\')
#endif
			break;
		i--;
	}

	out.erase(0, i+1);	// includes the '/' (or '\' on windows)
	return out;
}


/* ------------------------------------------------------------------ */


std::string gGetSlash() {
#if defined(_WIN32)
	return "\\";
#else
	return "/";
#endif
}


/* ------------------------------------------------------------------ */


std::string gItoa(int i) {
	std::stringstream out;
	out << i;
	return out.str();
}


/* ------------------------------------------------------------------ */


std::string gGetHomePath() {

	char path[PATH_MAX];

#if   defined(__linux__)

	snprintf(path, PATH_MAX, "%s/.giada", getenv("HOME"));

#elif defined(_WIN32)

	snprintf(path, PATH_MAX, ".");

#elif defined(__APPLE__)

	struct passwd *p = getpwuid(getuid());
	if (p == NULL) {
		gLog("[gGetHomePath] unable to fetch user infos\n");
		return "";
	}
	else {
		const char *home = p->pw_dir;
		snprintf(path, PATH_MAX, "%s/Library/Application Support/Giada", home);
	}

#endif

	return std::string(path);
}
