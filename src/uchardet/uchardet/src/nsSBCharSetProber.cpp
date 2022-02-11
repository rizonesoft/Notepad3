/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Universal charset detector code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Shy Shalom <shooshX@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stdio.h>
#include "nsSBCharSetProber.h"

nsProbingState nsSingleByteCharSetProber::HandleData(const char* aBuf, PRUint32 aLen)
{
  for (PRUint32 i = 0; i < aLen; i++)
  {
    unsigned char const order = mModel->charToOrderMap[(unsigned char)aBuf[i]];

    if (order < SYMBOL_CAT_ORDER)
    {
      mTotalChar++;
    }
    else if (order == ILL)
    {
      /* When encountering an illegal codepoint, no need
       * to continue analyzing data. */
      mState = eNotMe;
      break;
    }
    else if (order == CTR)
    {
      mCtrlChar++;
    }
    if (order < mModel->freqCharCount)
    {
      ++mFreqChar;

      if (mLastOrder < mModel->freqCharCount)
      {
        ++mTotalSeqs;
        if (!mReversed)
          ++(mSeqCounters[(int)mModel->precedenceMatrix[mLastOrder*mModel->freqCharCount+order]]);
        else // reverse the order of the letters in the lookup
          ++(mSeqCounters[(int)mModel->precedenceMatrix[order*mModel->freqCharCount+mLastOrder]]);
      }
    }
    mLastOrder = order;
  }

  if (mState == eDetecting)
    if (mTotalSeqs > SB_ENOUGH_REL_THRESHOLD)
    {
      float const cf = GetConfidence();
      if (cf >= POSITIVE_SHORTCUT_THRESHOLD)
        mState = eFoundIt;
      else if (cf <= NEGATIVE_SHORTCUT_THRESHOLD)
        mState = eNotMe;
    }

  return mState;
}

void  nsSingleByteCharSetProber::Reset(void)
{
  mState = eDetecting;
  mLastOrder = 255;
  for (PRUint32 i = 0; i < NUMBER_OF_SEQ_CAT; i++)
    mSeqCounters[i] = 0;
  mTotalSeqs = 0;
  mTotalChar = 0;
  mCtrlChar  = 0;
  mFreqChar = 0;
}

constexpr float rfactor(PRUint32 m, PRUint32 d) { 
  return ((d >= 1) ? (static_cast<float>(m) / static_cast<float>(d)) : static_cast<float>(m));
}

float nsSingleByteCharSetProber::GetConfidence()
{
  PRUint32 const neutralChar = mSeqCounters[NEUTRAL_CAT] + mCtrlChar;
  PRUint32 const netChars = (mTotalChar > neutralChar) ? (mTotalChar - neutralChar) : mTotalSeqs;

  if ((mTotalChar > 0) && (mTotalSeqs > 0))
  {
    // weighted good sequence count
    //PRUint32 const probableSeqs = mSeqCounters[POSITIVE_CAT] + (mSeqCounters[PROBABLE_CAT] >> 2);
    PRUint32 const validSeqs = mTotalSeqs - mSeqCounters[NEGATIVE_CAT];

    float r = rfactor(mSeqCounters[POSITIVE_CAT], mTotalSeqs) / mModel->mTypicalPositiveRatio;

    // negative sequence correction factor
    r *= rfactor(validSeqs, (mTotalSeqs + (netChars * mSeqCounters[NEGATIVE_CAT])));

    /* Multiply by a ratio of positive sequences per characters.
     * This would help in particular to distinguish close winners.
     * Indeed if you add a letter, you'd expect the positive sequence count
     * to increase as well. If it doesn't, it may mean that this new codepoint
     * may not have been a letter, but instead a symbol (or some other
     * character). This could make the difference between very closely related
     * charsets used for the same language.
     */
     r *= rfactor(validSeqs, netChars);

     /* The more control characters (proportionally to the size of the text), the
     * less confident we become in the current charset.
     */
    r *= rfactor(netChars, mTotalChar);
    
    // normalizing
    r *= rfactor(mFreqChar, mTotalChar);

    if (r > NO_DOUBT) { r = NO_DOUBT; }

    return r;
  }
  return SURE_NO;
}

const char* nsSingleByteCharSetProber::GetCharSetName()
{
  if (!mNameProber)
    return mModel->charsetName;
  return mNameProber->GetCharSetName();
}

#ifdef DEBUG_chardet
void nsSingleByteCharSetProber::DumpStatus()
{
  printf("  SBCS: %1.3f [%s]\r\n", GetConfidence(), GetCharSetName());
}
#endif
