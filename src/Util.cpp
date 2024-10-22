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
#include "Util.h"
#include <random>

enum class CharSet {
    Numeric,
    Alphabetic,
    AlphaNumeric
};

std::string Util::generateRandomString(size_t length, CharSet charSet) {
    static const char numeric[] = "0123456789";
    static const char alphabetic[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const char alphanumeric[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    const char* characters;
    size_t char_size;

    // set character set
    switch (charSet) {
    case CharSet::Numeric:
        characters = numeric;
        char_size = sizeof(numeric) - 1;
        break;
    case CharSet::Alphabetic:
        characters = alphabetic;
        char_size = sizeof(alphabetic) - 1;
        break;
    case CharSet::AlphaNumeric:
    default:
        characters = alphanumeric;
        char_size = sizeof(alphanumeric) - 1;
        break;
    }

    // set up a static random number generator
    //  as static, initialzied once and reused
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dis(0, char_size - 1);

    // generate randon number
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += characters[dis(gen)];
    }

    return result;
}

void Util::splitCSVLine(const CString& line, CStringArray& values)
{
	CString token;
	int startPos = 0;
	BOOL inQuotes = FALSE;

	for (int i = 0; i < line.GetLength(); ++i)
	{
		char currentChar = line[i];

		if (currentChar == '"')
		{
			// toggle quotes mode
			inQuotes = !inQuotes;
		}
		else if (currentChar == ',' && !inQuotes)
		{
			// found a comma outside of quotes, so split here
			token = line.Mid(startPos, i - startPos);

			// remove any leading/trailing whitespace
			token.Trim();
			
			// add the token to the array
			values.Add(token);
			startPos = i + 1;
		}
	}

	// add the last token after the last comma
	token = line.Mid(startPos);
	token.Trim();
	values.Add(token);
}