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
#include "prmem.h"

#include "nsSBCharSetProber.h"
#include "nsSBCSGroupProber.h"

#include "nsHebrewProber.h"


nsSBCSGroupProber::nsSBCSGroupProber()
  : mNumOfProbers(MAX_NUM_OF_SBCS_PROBERS), mBestGuess(-1), mActiveNum(0)
{
  PRUint32 i = 0;

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252GermanModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1GermanModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252FrenchModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1FrenchModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15FrenchModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252SpanishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1SpanishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15SpanishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252PortugueseModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1PortugueseModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9PortugueseModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15PortugueseModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250HungarianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2HungarianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_3EsperantoModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252AfricaansModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1AfricaansModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9AfricaansModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15AfricaansModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252NederlandsModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1NederlandsModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9NederlandsModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15NederlandsModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252DanishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15DanishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1DanishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13LithuanianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_10LithuanianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_4LithuanianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13LatvianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_10LatvianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_4LatvianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_3MalteseModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250CzechModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2CzechModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeCzechModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852CzechModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250SlovakModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2SlovakModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeSlovakModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852SlovakModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250PolishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2PolishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13PolishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_16PolishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Mac_CentraleuropePolishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852PolishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252FinnishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1FinnishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_4FinnishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9FinnishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13FinnishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15FinnishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252ItalianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1ItalianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_3ItalianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9ItalianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15ItalianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250CroatianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2CroatianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13CroatianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_16CroatianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeCroatianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852CroatianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252EstonianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1257EstonianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_4EstonianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_13EstonianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15EstonianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252IrishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1IrishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9IrishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15IrishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250RomanianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2RomanianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_16RomanianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852RomanianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1250SloveneModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_2SloveneModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_16SloveneModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeSloveneModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm852SloveneModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1252SwedishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_1SwedishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_4SwedishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9SwedishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_15SwedishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1251BelarusianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Win1251RussianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Koi8rRussianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Latin5RussianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&MacCyrillicRussianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm866RussianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Ibm855RussianModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_7GreekModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1253GreekModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Latin5BulgarianModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Win1251BulgarianModel);

  nsHebrewProber* hebprober = new nsHebrewProber();
  // Notice: Any change in these indexes - 10,11,12 must be reflected
  // in the code below as well.
  PRUint32 const heb = i;
  mProbers[i++] = hebprober;
  mProbers[i++] = new nsSingleByteCharSetProber(&Win1255Model, PR_FALSE, hebprober); // Logical Hebrew
  mProbers[i++] = new nsSingleByteCharSetProber(&Win1255Model, PR_TRUE, hebprober);  // Visual Hebrew
  // Tell the Hebrew prober about the logical and visual probers
  if (mProbers[heb] && mProbers[heb + 1] && mProbers[heb + 2]) // all are not null
  {
    hebprober->SetModelProbers(mProbers[heb + 1], mProbers[heb + 2]);
  }
  else // One or more is null. avoid any Hebrew probing, null them all
  {
    for (PRUint32 j = heb + 2; j >= heb; --j)
    {
      delete mProbers[j];
      mProbers[j] = nsnull;
    }
  }

  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_3TurkishModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_9TurkishModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1256ArabicModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_6ArabicModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Windows_1258VietnameseModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&VisciiVietnameseModel);

  mProbers[i++] = new nsSingleByteCharSetProber(&Tis_620ThaiModel);
  mProbers[i++] = new nsSingleByteCharSetProber(&Iso_8859_11ThaiModel);

  mNumOfProbers = i;

  for (; i < MAX_NUM_OF_SBCS_PROBERS; ++i) { mProbers[i] = nsnull; }

  Reset();
}

nsSBCSGroupProber::~nsSBCSGroupProber()
{
  for (PRUint32 i = 0; i < MAX_NUM_OF_SBCS_PROBERS; i++)
  {
    if (mProbers[i]) { delete mProbers[i]; }
  }
}


const char* nsSBCSGroupProber::GetCharSetName()
{
  //if we have no answer yet
  if (mBestGuess == -1)
  {
    GetConfidence();
    //no charset seems positive
    if (mBestGuess == -1)
      //we will use default.
      mBestGuess = 0;
  }
  return mProbers[mBestGuess]->GetCharSetName();
}

void  nsSBCSGroupProber::Reset(void)
{
  mActiveNum = 0;
  for (PRUint32 i = 0; i < MAX_NUM_OF_SBCS_PROBERS; ++i)
  {
    if (mProbers[i]) // not null
    {
      mProbers[i]->Reset();
      mIsActive[i] = PR_TRUE;
      ++mActiveNum;
    }
    else {
      mIsActive[i] = PR_FALSE;
    }
  }
  mBestGuess = -1;
  mState = eDetecting;
}


nsProbingState nsSBCSGroupProber::HandleData(const char* aBuf, PRUint32 aLen)
{
  nsProbingState st;
  PRUint32 i;
  char *newBuf1 = 0;
  PRUint32 newLen1 = 0;

  //apply filter to original buffer, and we got new buffer back
  //depend on what script it is, we will feed them the new buffer
  //we got after applying proper filter
  //this is done without any consideration to KeepEnglishLetters
  //of each prober since as of now, there are no probers here which
  //recognize languages with English characters.
  if (!FilterWithoutEnglishLetters(aBuf, aLen, &newBuf1, newLen1))
    goto done;

  if (newLen1 == 0)
    goto done; // Nothing to see here, move on.

  for (i = 0; i < mNumOfProbers; i++)
  {
     if (!mIsActive[i])
       continue;
     st = mProbers[i]->HandleData(newBuf1, newLen1);
     if (st == eFoundIt)
     {
       mBestGuess = i;
       mState = eFoundIt;
       break;
     }
     else if (st == eNotMe)
     {
       mIsActive[i] = PR_FALSE;
       mActiveNum--;
       if (mActiveNum <= 0)
       {
         mState = eNotMe;
         break;
       }
     }
  }

done:
  PR_FREEIF(newBuf1);

  return mState;
}

float nsSBCSGroupProber::GetConfidence(void)
{
  float bestConf = 0.0f;

  switch (mState)
  {
  case eFoundIt:
    return SURE_YES;
  case eNotMe:
    return SURE_NO;
  default:
    for (PRUint32 i = 0; i < mNumOfProbers; i++)
    {
      if (!mIsActive[i])
        continue;
      float const cf = mProbers[i]->GetConfidence();
      if (bestConf < cf)
      {
        bestConf = cf;
        mBestGuess = i;
      }
    }
  }
  return bestConf;
}

#ifdef DEBUG_chardet
void nsSBCSGroupProber::DumpStatus()
{
  PRUint32 i;
  float cf;

  cf = GetConfidence();
  printf(" SBCS Group Prober --------begin status \r\n");
  for (i = 0; i < mNumOfProbers; i++)
  {
    if (!mIsActive[i])
      printf("  inactive: [%s] (i.e. confidence is too low).\r\n", mProbers[i]->GetCharSetName());
    else
      mProbers[i]->DumpStatus();
  }
  printf(" SBCS Group found best match [%s] confidence %f.\r\n",
         mProbers[mBestGuess]->GetCharSetName(), cf);
}
#endif
