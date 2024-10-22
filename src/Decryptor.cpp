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
#include "Decryptor.h"
#include "File.h"
#include "cryptlib.h"

// added
#include <fstream>
#include <iomanip>

#define BUFFER_SIZE (1024*1024)

void Decryptor::decrypt(const std::string& name, std::list<std::string>& pwds)
{
#ifdef _DEBUG
	// generate a log file when in debug mode
	std::ofstream logFile("debug_log.txt", std::ios::app);
	logFile << "Starting decryption of file: " << name << std::endl;
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

		// pass file size to listener
		if (mListener)
		{
			// reset progress for each file
			mListener->reset();
			mListener->setRange(fileSize);
		}

		// check for password(s)
		if (pwds.empty()) {
#ifdef _DEBUG
			logFile << "Error: No passwords specified" << std::endl;
#endif
			throw std::logic_error("No passwords specified!");
		}

		// create the envelope
		CRYPT_ENVELOPE env;
		int status = cryptCreateEnvelope(&env, CRYPT_UNUSED, CRYPT_FORMAT_AUTO);
		if (status != CRYPT_OK) {
#ifdef _DEBUG
			logFile << "Failed to create envelope. Status: " << status << std::endl;
#endif
			throw std::runtime_error("Failed to create envelope!");
		}

		status = cryptSetAttribute(env, CRYPT_ATTRIBUTE_BUFFERSIZE, BUFFER_SIZE);
#ifdef _DEBUG
		logFile << "Set buffer size. Status: " << status << std::endl;
#endif

		int n;
		std::uintmax_t totalProcessed = 0;

		// note file read returns streamsize but cast as int to match Cryptlib PushData 
		while (!file.eof() && (n = static_cast<int>(file.read(buf.get(), BUFFER_SIZE - 4096))) != 0)
		{
#ifdef _DEBUG
			logFile << "Read " << n << " bytes from file" << std::endl;
#endif

			int bytesDone = 0;
			int bytesLeft = n;

			while (bytesLeft > 0)
			{
				int cBytes;
				status = cryptPushData(env, buf.get() + bytesDone, bytesLeft, &cBytes);
#ifdef _DEBUG
				logFile << "cryptPushData status: " << status << ", pushed " << cBytes << " bytes" << std::endl;
#endif

				if (status == CRYPT_ENVELOPE_RESOURCE)
				{
#ifdef _DEBUG
					logFile << "CRYPT_ENVELOPE_RESOURCE encountered, trying passwords" << std::endl;

					if (!tryPasswords(env, pwds, name, logFile))
					{
						logFile << "Error: No valid passwords found" << std::endl;
						throw std::runtime_error("None of the passwords are valid for file " + name);
					}
#else
					if (!tryPasswords(env, pwds, name))
					{
						throw std::runtime_error("None of the passwords are valid for file " + name);
					}
#endif

#ifdef _DEBUG
					// try to get crypto info after password acceptance
					logCryptoContext(env, logFile);

					// try to get and log the IV
					unsigned char iv[32];  // Increased size to accommodate potentially larger IVs
					int ivLength = sizeof(iv);
					int ivStatus = cryptGetAttributeString(env, CRYPT_CTXINFO_IV, iv, &ivLength);
					if (ivStatus == CRYPT_OK)
					{
						logFile << "IV (hex): ";
						for (int i = 0; i < ivLength; ++i) {
							logFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(iv[i]);
						}
						logFile << std::dec << std::endl;  // reset to decimal output
					}
					else {
						logFile << "Unable to retrieve IV. Status: " << ivStatus << std::endl;
					}
#endif
				}
				else if (status != CRYPT_OK)
				{
					if (status != 0)
					{
#ifdef _DEBUG
						logFile << "Invalid encrypted file. Status: " << status << std::endl;
#endif
						throw std::runtime_error("The selected file " + name + " is not a valid encrypted file.");
					}
					else
					{
#ifdef _DEBUG
						logFile << "Error processing file. Status: " << status << std::endl;
#endif
						throw std::runtime_error("Error processing file " + name + ". Status: " + std::to_string(status));
					}
				}

				bytesLeft -= cBytes;
				bytesDone += cBytes;

				int popStatus = cryptPopData(env, buf.get(), BUFFER_SIZE, &cBytes);
#ifdef _DEBUG
				logFile << "cryptPopData status: " << popStatus << ", popped " << cBytes << " bytes" << std::endl;
#endif

				file.write(buf.get(), cBytes);
				totalProcessed += cBytes;
			}

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
		status = cryptPopData(env, buf.get(), BUFFER_SIZE, &n);
#ifdef _DEBUG
		logFile << "Final cryptPopData status: " << status << ", popped " << n << " bytes" << std::endl;
#endif
		
		if (status != CRYPT_OK) {
			throw std::runtime_error("Error popping final data from envelope");
		}

		// write the file and flush
		file.write(buf.get(), n);
		file.flush();

		// destroy envelope
		cryptDestroyEnvelope(env);

		// close file
		file.close();

		if (mListener) {
			mListener->complete();
		}

#ifdef _DEBUG
		logFile << "Decryption completed successfully. Total bytes processed: " << totalProcessed << std::endl;
#endif
	}
	catch (const std::exception& e)
	{
#ifdef _DEBUG
		logFile << "Error decrypting file " << name << ": " << e.what() << std::endl;
#endif
		std::cerr << "Error decrypting file " << name << ": " << e.what() << std::endl;


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

bool Decryptor::tryPasswords(CRYPT_ENVELOPE env, const std::list<std::string>& pwds, const std::string& name
#ifdef _DEBUG
	, std::ofstream& logFile
#endif
)
{
	for (const auto& pwd : pwds) {
#ifdef _DEBUG
		logFile << "Trying password (length: " << pwd.length() << ")" << std::endl;
#endif
        if (cryptSetAttribute(env, CRYPT_ATTRIBUTE_CURRENT_GROUP, CRYPT_CURSOR_FIRST) == CRYPT_OK) {
            do {
                CRYPT_ATTRIBUTE_TYPE attrib;
                cryptGetAttribute(env, CRYPT_ATTRIBUTE_CURRENT_GROUP, (int*)&attrib);
                if (attrib == CRYPT_ENVINFO_PASSWORD)
				{
                    int status = cryptSetAttributeString(env, CRYPT_ENVINFO_PASSWORD, pwd.c_str(), static_cast<int>(pwd.size()));
                    if (status == CRYPT_OK)
					{
#ifdef _DEBUG
                        logFile << "Password accepted" << std::endl;
#endif
                        return true;
                    }
#ifdef _DEBUG
                    logFile << "Password rejected. Status: " << status << std::endl;
#endif
                }
            } while (cryptSetAttribute(env, CRYPT_ATTRIBUTE_CURRENT_GROUP, CRYPT_CURSOR_NEXT) == CRYPT_OK);
        }
    }
#ifdef _DEBUG
    logFile << "All passwords rejected" << std::endl;
#endif

    return false;
}

// context logger
void Decryptor::logCryptoContext(CRYPT_ENVELOPE env, std::ofstream& logFile)
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

	// attempt to get key iterations
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

	// attempt to get keying salt
	unsigned char salt[32];
	int saltLength = sizeof(salt);
	if (cryptGetAttributeString(env, CRYPT_CTXINFO_KEYING_SALT, salt, &saltLength) == CRYPT_OK)
	{
		logFile << tab << "Keying Salt (hex): ";
		for (int i = 0; i < saltLength; ++i)
		{
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