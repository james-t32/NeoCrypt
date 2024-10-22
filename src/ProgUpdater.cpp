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
#include "ProgUpdater.h"
#include <algorithm>

CProgUpdater::CProgUpdater(CWnd* progressWin, CWnd* textWin)
    : pWin(progressWin), pText(textWin), mMax(100), mCurrent(0)
{
    pWin->SendMessage(PBM_SETRANGE32, 0, 100);
    pText->SetWindowText(_T("0 %"));
}

void CProgUpdater::setRange(std::uintmax_t max)
{
    mMax = max;
    reset();
}

void CProgUpdater::reset()
{
    mCurrent = 0;
    updateProgress();
}

void CProgUpdater::increment(std::uintmax_t step)
{
    mCurrent += step;
    updateProgress();
}

void CProgUpdater::complete()
{
    mCurrent = mMax;
    updateProgress();
}

void CProgUpdater::updateProgress()
{
    // avoid division by zero
    if (mMax == 0) return;

    // calculate percentage and set bounds to 0 and 100
    double percentage = std::clamp((static_cast<double>(mCurrent) * 100.0) / mMax, 0.0, 100.0);
    
    // then round to nearest integer
    int percentInt = static_cast<int>(percentage + 0.5);    

    pWin->SendMessage(PBM_SETPOS, static_cast<WPARAM>(percentInt), 0);

    CString str;
    str.Format(_T("%d %%"), percentInt);
    pText->SetWindowText(str);
}