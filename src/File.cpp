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

#include "stdafx.h"
#include "File.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <system_error>
#include "Util.h"

File::File() : opened(false) {}

File::File(const std::string& name) : mFileName(name)
{
	// ensure read file exists
	if (!std::filesystem::exists(name)) {
		throw std::runtime_error("File " + name + " does not exist!");
	}

	// open read file
	rfile.open(name, std::ios::binary);
	if (!rfile) {
		throw std::runtime_error("Error occuring while opening file " + name + "!");
	}

	// generate temporary output file name
	mTempFile = name + "." + Util::generateRandomString(4) + ".tmp";

	// ensure temp file does not already exist
	if (std::filesystem::exists(mTempFile)) {
		throw std::runtime_error("Temporary file " + mTempFile + " already exists!");
	}

	// open write file
	wfile.open(mTempFile, std::ios::binary);
	if (!wfile) {
		rfile.close();
		throw std::runtime_error("Error occured while creating temporary file " + mTempFile + "!");
	}

	opened = true;
}

void File::open(const std::string& name)
{
	if (opened) {
		throw std::logic_error("File already open!");
	}

	mFileName = name;

	// ensure read file exists
	if (!std::filesystem::exists(name)) {
		throw std::runtime_error("File " + name + " does not exist!");
	}

	// open read file
	rfile.open(name, std::ios::binary);
	if (!rfile) {
		throw std::runtime_error("Error occuring while opening file " + name + "!");
	}

	// generate temporary output file name
	mTempFile = name + "." + Util::generateRandomString(4) + ".tmp";

	// ensure temp file does not already exist
	if (std::filesystem::exists(mTempFile)) {
		throw std::runtime_error("Temporary file " + mTempFile + " already exists!");
	}

	// open temp file for writing
	wfile.open(mTempFile, std::ios::binary);
	if (!wfile)
	{
		rfile.close();
		throw std::runtime_error("Error occured while creating temporary file " + mTempFile + "!");
	}

	opened = true;
}

void File::close()
{
	// set file system namespace
	namespace fs = std::filesystem;

	// if file is open
	if (opened)
	{
		try {
			// hold the error code
			std::error_code ec;

			// try to close read file
			rfile.close();
			if (rfile.fail() && !rfile.eof())
			{
				std::ostringstream errorMsg;
				errorMsg << "Error closing read file: " << std::strerror(errno);
				throw std::runtime_error(errorMsg.str());
			}

			// try to close write file
			wfile.close();
			if (wfile.fail() && !wfile.eof())
			{
				std::ostringstream errorMsg;
				errorMsg << "Error closing write file: " << std::strerror(errno);
				throw std::runtime_error(errorMsg.str());
			}

			// store file path and attributes
			auto path = fs::path(mFileName);
			auto perms = fs::status(path).permissions();

			// remove read-only attribute
			fs::permissions(path, perms | fs::perms::owner_write, ec);
			if (ec) {
				throw std::runtime_error("Error setting file permissions: " + ec.message());
			}

			// delete original file if it exists)
			if (fs::exists(path))
			{
				fs::remove(path, ec);
				if (ec) {
					throw std::runtime_error("Cannot replace file " + mFileName + ". Error: " + ec.message());
				}
			}

			// rename temp file to original file
			fs::rename(mTempFile, mFileName, ec);
			if (ec) {
				throw std::runtime_error("Cannot rename temp file '" + mTempFile + "' to '" + mFileName + "'. Error: " + ec.message());
			}

			// reset attributes
			fs::permissions(path, perms, ec);
			if (ec) {
				throw std::runtime_error("Error restoring file permissions: " + ec.message());
			}
		}
		catch (const std::exception& e)
		{
			// mark file as closed even if error occurs
			opened = false;

			// throw the error
			throw;
		}

		// mark file closed if everything was successful
		opened = false;
	}
}

void File::revert()
{
	// if file is open
	if (opened)
	{
		try
		{
			// hold the error
			std::error_code ec;

			// try to close read file
			rfile.close();
			if (rfile.fail() && !rfile.eof())
			{
				std::ostringstream errorMsg;
				errorMsg << "Error closing read file: " << std::strerror(errno);
				throw std::runtime_error(errorMsg.str());
			}

			// try to close write file
			wfile.close();
			if (wfile.fail() && !wfile.eof())
			{
				std::ostringstream errorMsg;
				errorMsg << "Error closing write file: " << std::strerror(errno);
				throw std::runtime_error(errorMsg.str());
			}

			// delete temp file
			std::filesystem::remove(mTempFile, ec);
			if (ec) {
				throw std::runtime_error("Failed to delete temporary file: " + ec.message());
			}
		}
		catch (const std::exception& e)
		{
			// mark file as closed even if an error occurs
			opened = false;

			// throw the error
			throw;
		}

		// mark file closed if everything was successful
		opened = false;
	}
}

std::streamsize File::read(char* buf, std::streamsize size)
{
	// read only if opened
	if (!opened) {
		throw std::runtime_error("Attempting to read from a closed file.");
	}

	// read file
	rfile.read(buf, size);
	std::streamsize bytesRead = rfile.gcount();

	if (bytesRead != size && !rfile.eof()) {
		throw std::runtime_error("Error reading from file.");
	}

	return bytesRead;
}

void File::write(const char* buf, std::streamsize size)
{
	// write only if opened
	if (!opened) {
		throw std::runtime_error("Attempting to write to a closed file.");
	}

	try
	{
		std::streampos before = wfile.tellp();

		// write the file
		wfile.write(buf, size);
		if (!wfile)
		{
			std::streampos after = wfile.tellp();
			std::streamoff written = after - before;
			std::ostringstream errorMsg;
			errorMsg << "Error writing file " << mFileName
				<< ". Expected to write " << size
				<< " bytes, but wrote " << written << " bytes!";
			throw std::runtime_error(errorMsg.str());
		}
	}
	catch (const std::ios_base::failure& e)
	{
		// handle specific iostream failures
		std::ostringstream errorMsg;
		errorMsg << "I/O error occurred while writing to file " << mFileName << ": " << e.what();
		throw std::runtime_error(errorMsg.str());
	}
	catch (const std::exception& e)
	{
		// handle other standard exceptions
		std::ostringstream errorMsg;
		errorMsg << "Error occurred while writing to file " << mFileName << ": " << e.what();
		throw std::runtime_error(errorMsg.str());
	}
	catch (...)
	{
		// handle any other unexpected exceptions
		throw std::runtime_error("Unknown error occurred while writing to file " + mFileName);
	}
}

std::uintmax_t File::size() const
{
	try {
		return std::filesystem::file_size(mFileName);
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::ostringstream errorMsg;
		errorMsg << "Error getting file size. Error: " << e.what();
		throw std::runtime_error(errorMsg.str());
	}
}

bool File::eof() noexcept
{
	return rfile.eof();
}

void File::flush()
{
	// flush only if opened
	if (!opened) {
		throw std::runtime_error("Attempting to flush a closed file.");
	}

	// flush the file
	wfile.flush();
	if (!wfile) {
		throw std::runtime_error("Failed to flush file " + mFileName);
	}
}