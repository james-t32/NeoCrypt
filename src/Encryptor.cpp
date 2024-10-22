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
#include "Encryptor.h"
#include "File.h"
#include "cryptlib.h"

// added
#include <fstream>
#include <iomanip>

#define BUFFER_SIZE (1024*1024)

void Encryptor::encrypt(const std::string& name, std::list<std::string>& password, Algorithm::Type algo)
{
#ifdef _DEBUG
	// generate a log file when in debug mode
	std::ofstream logFile("debug_log.txt", std::ios::app);
	logFile << "Starting encryption of file: " << name << std::endl;
#endif

	// buffer uses a pointer that is cleared even when error
	std::unique_ptr<char[]> buf(new char[BUFFER_SIZE]);

	File file(name);

	// file process wrapped in try/catch
	try {
		// get file size
		std::uintmax_t fileSize = file.size();

#ifdef _DEBUG
		logFile << "File size: " << fileSize << " bytes" << std::endl;
#endif

		if (fileSize == 0)
		{
			CString errorMsg;
			errorMsg.Format(_T("File %s is empty (0 bytes) and cannot be encrypted!"), CString(name.c_str()));

#ifdef _DEBUG
			logFile << "Error: Empty file (0 bytes) cannot be encrypted" << fileSize << " bytes" << std::endl;
#endif

			throw std::runtime_error(static_cast<LPCSTR>(CStringA(errorMsg)));
		}

		// pass file size to listener
		if (mListener)
		{
			// reset progress for each file
			mListener->reset();
			mListener->setRange(fileSize);
		}

		// check for at-least one password
		if (password.empty())
		{
#ifdef _DEBUG
			logFile << "Error: No passwords in list" << std::endl;
#endif
			throw std::logic_error("No password in password list!");
		}

		// set algorithm type
#ifdef _DEBUG
		logFile << "Setting encryption algorithm: " << algo << std::endl;
#endif
		int status = cryptSetAttribute(CRYPT_UNUSED, CRYPT_OPTION_ENCR_ALGO, algo);

		if (status != CRYPT_OK) {
#ifdef _DEBUG
			logFile << "Failed to set algorithm type. Status: " << status << std::endl;
#endif
			throw std::runtime_error("Could not set algorithm type!");
		}

		// create the envelope
		CRYPT_ENVELOPE env;
#ifdef _DEBUG
		logFile << "Creating envelope" << std::endl;
#endif
		status = cryptCreateEnvelope(&env, CRYPT_UNUSED, CRYPT_FORMAT_CRYPTLIB);
		if (status != CRYPT_OK) {
#ifdef _DEBUG
			logFile << "Failed to create envelope. Status: " << status << std::endl;
#endif
			throw std::runtime_error("Failed to create envelope!");
		}

		// add password
#ifdef _DEBUG
		logFile << "Adding password to envelope" << std::endl;
#endif
		for (const auto& pwd : password)
		{
			status = cryptSetAttributeString(env, CRYPT_ENVINFO_PASSWORD, pwd.c_str(), static_cast<int>(pwd.size()));
			if (status != CRYPT_OK) {
#ifdef _DEBUG
				logFile << "Failed to set password. Status: " << status << std::endl;
#endif
				throw std::runtime_error("Failed to set password " + pwd + "!");
			}
		}

		// allocate encryption buffer
#ifdef _DEBUG
		logFile << "Allocating encryption buffer" << std::endl;
#endif
		status = cryptSetAttribute(env, CRYPT_ATTRIBUTE_BUFFERSIZE, BUFFER_SIZE);
#ifdef _DEBUG
		logFile << "Set buffer size. Status: " << status << std::endl;
#endif
		if (status != CRYPT_OK) {
			throw std::runtime_error("Could not allocate encryption buffer!");
		}

		int n;
		int cBytes;
		std::uintmax_t totalProcessed = 0;

		// initial read of 128 bytes
#ifdef _DEBUG
		logFile << "Starting initial read of 128 bytes" << std::endl;
#endif
		if (!file.eof() && (n = static_cast<int>(file.read(buf.get(), 128))) != 0)
		{
#ifdef _DEBUG
			logFile << "Read " << n << " bytes for initial push" << std::endl;
#endif
			status = cryptPushData(env, buf.get(), n, &cBytes);
#ifdef _DEBUG
			logFile << "Initial cryptPushData status: " << status << ", pushed " << cBytes << " bytes" << std::endl;
#endif
			if (status != CRYPT_OK) {
				throw std::runtime_error("Error pushing initial data to envelope");
			}

			// Log cryptographic context information after initial push
#ifdef _DEBUG
			logCryptoContext(env, logFile);

			// Try to get and log the IV
			unsigned char iv[32];  // Increased size to accommodate potentially larger IVs
			int ivLength = sizeof(iv);
			int ivStatus = cryptGetAttributeString(env, CRYPT_CTXINFO_IV, iv, &ivLength);
			if (ivStatus == CRYPT_OK) {
				logFile << "IV (hex): ";
				for (int i = 0; i < ivLength; ++i) {
					logFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(iv[i]);
				}
				logFile << std::dec << std::endl;  // Reset to decimal output
			}
			else {
				logFile << "Unable to retrieve IV. Status: " << ivStatus << std::endl;
			}
#endif

			status = cryptPopData(env, buf.get(), BUFFER_SIZE, &cBytes);
#ifdef _DEBUG
			logFile << "Initial cryptPopData status: " << status << ", popped " << cBytes << " bytes" << std::endl;
#endif
			if (status != CRYPT_OK) {
				throw std::runtime_error("Error popping initial data from envelope");
			}

			file.write(buf.get(), cBytes);
			totalProcessed += cBytes;

			if (mListener) {
				mListener->increment(n);
			}
		}

		// main encrypot loop
		while (!file.eof() && (n = static_cast<int>(file.read(buf.get(), BUFFER_SIZE - 4096))) != 0)
		{
#ifdef _DEBUG
			logFile << "Read " << n << " bytes from file" << std::endl;
#endif
			status = cryptPushData(env, buf.get(), n, &cBytes);
#ifdef _DEBUG
			logFile << "cryptPushData status: " << status << ", pushed " << cBytes << " bytes" << std::endl;
#endif
			if (status != CRYPT_OK) {
				throw std::runtime_error("Error pushing data to envelope");
			}

			status = cryptPopData(env, buf.get(), BUFFER_SIZE, &cBytes);
#ifdef _DEBUG
			logFile << "cryptPopData status: " << status << ", popped " << cBytes << " bytes" << std::endl;
#endif
			if (status != CRYPT_OK) {
				throw std::runtime_error("Error popping data from envelope");
			}

			file.write(buf.get(), cBytes);
			totalProcessed += cBytes;

			if (mListener) {
				mListener->increment(n);
			}
		}

		// flush remaining data
#ifdef _DEBUG
		logFile << "Flushing remaining data" << std::endl;
#endif
		status = cryptFlushData(env);
#ifdef _DEBUG
		logFile << "cryptFlushData status: " << status << std::endl;
#endif
		if (status != CRYPT_OK) {
			throw std::runtime_error("Error flushing data from envelope");
		}

		// final pop data
		status = cryptPopData(env, buf.get(), BUFFER_SIZE, &cBytes);
#ifdef _DEBUG
		logFile << "Final cryptPopData status: " << status << ", popped " << cBytes << " bytes" << std::endl;
#endif
		if (status != CRYPT_OK) {
			throw std::runtime_error("Error popping final data from envelope");
		}

		// write the file
		file.write(buf.get(), cBytes);
		totalProcessed += cBytes;
		file.flush();

		// destroy envelope
		cryptDestroyEnvelope(env);

		// close file
		file.close();

		if (mListener) {
			mListener->complete();
		}

#ifdef _DEBUG
		logFile << "Encryption completed successfully. Total bytes processed: " << totalProcessed << std::endl;
#endif
	}
	catch (const std::exception& e)
	{
#ifdef _DEBUG
		logFile << "Error encrypting file " << name << ": " << e.what() << std::endl;
#endif
		std::cerr << "Error encrypting file " << name << ": " << e.what() << std::endl;

#ifdef _DEBUG
		logFile << "Attempting to revert file " << name << ": " << e.what() << std::endl;
#endif

		// revert changes
		file.revert();

#ifdef _DEBUG
		logFile << "Reverted file " << name << ": " << e.what() << std::endl;
		// end log and close file
		logFile << "----------------------------------------" << std::endl; // add an seperator
		logFile << std::endl;  // finish with a blank line
		logFile.flush();
		logFile.close();
#endif

		// throw the exception
		throw;
	}

#ifdef _DEBUG
	// end log and close file
	logFile << "----------------------------------------" << std::endl; // add an seperator
	logFile << std::endl;  // finish with a blank line
	logFile.flush();
	logFile.close();
#endif
}

void Encryptor::logCryptoContext(CRYPT_ENVELOPE env, std::ofstream& logFile)
{
	logFile << "Cryptographic Context Information:" << std::endl;

	int value;
	char buffer[256];
	int length;
	char tab[5] = "    ";

#define LOG_ATTR(attr, name) \
        if (cryptGetAttribute(env, attr, &value) == CRYPT_OK) \
            logFile << tab << name << ": " << value << std::endl; \
        else \
            logFile << tab << "Unable to retrieve " << name << std::endl;

#define LOG_ATTR_STRING(attr, name) \
        length = sizeof(buffer); \
        if (cryptGetAttributeString(env, attr, buffer, &length) == CRYPT_OK) \
            logFile << tab << name << ": " << std::string(buffer, length) << std::endl; \
        else \
            logFile << tab << "Unable to retrieve " << name << std::endl;

	LOG_ATTR(CRYPT_CTXINFO_ALGO, "Algorithm");
	LOG_ATTR(CRYPT_CTXINFO_MODE, "Mode");
	LOG_ATTR(CRYPT_CTXINFO_KEYSIZE, "Key Size (bits)");
	LOG_ATTR(CRYPT_CTXINFO_BLOCKSIZE, "Block Size (bytes)");
	LOG_ATTR_STRING(CRYPT_CTXINFO_NAME_ALGO, "Algorithm Name");
	LOG_ATTR_STRING(CRYPT_CTXINFO_NAME_MODE, "Mode Name");

	// try to get key iterations
	int keyIterations;
	if (cryptGetAttribute(env, CRYPT_CTXINFO_KEYING_ITERATIONS, &keyIterations) == CRYPT_OK) {
		logFile << tab << "Key Iterations: " << keyIterations << std::endl;
	}
	else if (cryptGetAttribute(CRYPT_UNUSED, CRYPT_OPTION_KEYING_ITERATIONS, &keyIterations) == CRYPT_OK) {
		logFile << tab << "Key Iterations (Global Setting): " << keyIterations << std::endl;
	}
	else {
		logFile << tab << "Unable to retrieve Key Iterations" << std::endl;
	}

	// Try to get keying salt
	unsigned char salt[32];
	int saltLength = sizeof(salt);
	if (cryptGetAttributeString(env, CRYPT_CTXINFO_KEYING_SALT, salt, &saltLength) == CRYPT_OK)
	{
		logFile << tab << "Keying Salt (hex): ";
		for (int i = 0; i < saltLength; ++i) {
			logFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
		}
		logFile << std::dec << std::endl;
	}
	else {
		logFile << tab << "Unable to retrieve Keying Salt" << std::endl;
	}

#undef LOG_ATTR
#undef LOG_ATTR_STRING
}