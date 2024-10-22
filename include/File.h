/*
-------------------------------------------------------------------------------
 This source file is part of NeoCrypt (a file encryption software).

 Copyright (C) 2002-2003, Arijit De <arijit1985@yahoo.co.in>

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 http://www.gnu.org/copyleft/gpl.html.
-------------------------------------------------------------------------------
*/

#ifndef _File_H__
#define _File_H__

#include <string>
#include <fstream>
#include <cstdint>

class File
{
public:
	File();
	File(const std::string& name);

	~File() { revert(); }

	void open(const std::string& name);

	// replaces input file with temporary output file.
	void close();

	// Reads a miximum of specified number of bytes from file and returns the
	// number of bytes read.
	std::streamsize read(char* buf, std::streamsize size);

	// writes specified number of bytes from buf to temporary output file.
	void write(const char* buf, std::streamsize size);

	// resets any changes and closes the file.
	void revert();

	// gets the input file size
	std::uintmax_t size() const;

	// flushes file
	void flush();

	// returns end of file
	bool eof() noexcept;

private:
	std::string mFileName;
	std::string mTempFile;
	bool opened;
	std::ifstream rfile;		// file for reading
	std::ofstream wfile;		// file for writing
};

#endif // _File_H__