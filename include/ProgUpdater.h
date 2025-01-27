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

#pragma once

#include <afxwin.h>  // For CWnd
#include "ProgressListener.h"  // Assuming this is where your ProgressListener interface is defined

class CProgUpdater : public ProgressListener
{
public:
    CProgUpdater(CWnd* progressWin, CWnd* textWin);
    void increment(std::uintmax_t step) override;
    void setRange(std::uintmax_t max) override;
    void complete() override;
    void reset() override;

private:
    std::uintmax_t mMax;
    std::uintmax_t mCurrent;
    CWnd* pWin;
    CWnd* pText;

    void updateProgress();
};