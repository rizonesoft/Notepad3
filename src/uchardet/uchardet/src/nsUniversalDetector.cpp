/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *          JoungKyun.Kim <http://oops.org>
 *            - Add mDetectedConfidence
 *            - Add mDetectedIsBOM
 *
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

#include "nscore.h"

#include "nsUniversalDetector.h"

#include "nsMBCSGroupProber.h"
#include "nsSBCSGroupProber.h"
#include "nsEscCharsetProber.h"
#include "nsLatin1Prober.h"

nsUniversalDetector::nsUniversalDetector(PRUint32 aLanguageFilter)
{
  mNbspFound = PR_FALSE;
  mDone = PR_FALSE;
  mInTag = PR_FALSE;
  mEscCharSetProber = nsnull;

  mStart = PR_TRUE;
  mDetectedCharset = nsnull;
  mDetectedConfidence = 0.0;
  mDetectedIsBOM = 0;
  mGotData = PR_FALSE;
  mInputState = ePureAscii;
  mLastChar = '\0';
  mLanguageFilter = aLanguageFilter;

  PRUint32 i;
  for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    mCharSetProbers[i] = nsnull;
}

nsUniversalDetector::~nsUniversalDetector()
{
  for (PRInt32 i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    delete mCharSetProbers[i];

  delete mEscCharSetProber;
}

void
nsUniversalDetector::Reset()
{
  mNbspFound = PR_FALSE;
  mDone = PR_FALSE;
  mInTag = PR_FALSE;

  mStart = PR_TRUE;
  mDetectedCharset = nsnull;
  mDetectedConfidence = 0.0;
  mDetectedIsBOM = 0;
  mGotData = PR_FALSE;
  mInputState = ePureAscii;
  mLastChar = '\0';

  if (mEscCharSetProber)
    mEscCharSetProber->Reset();

  PRUint32 i;
  for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    if (mCharSetProbers[i])
      mCharSetProbers[i]->Reset();
}

//---------------------------------------------------------------------

nsresult nsUniversalDetector::HandleData(const char* aBuf, PRUint32 aLen)
{
  if(mDone)
    return NS_OK;

  if (aLen > 0)
    mGotData = PR_TRUE;

  //If the data starts with BOM, we know it is UTF
  if (mStart)
  {
    mStart = PR_FALSE;
    if (aLen > 3)
      switch (aBuf[0])
        {
        case '\xEF':
          if (('\xBB' == aBuf[1]) && ('\xBF' == aBuf[2])) {
            // EF BB BF  UTF-8 encoded BOM
            mDetectedCharset = "UTF-8";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\xFE':
          if (('\xFF' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3])) {
            // FE FF 00 00  UCS-4, unusual octet order BOM (3412)
            mDetectedCharset = "X-ISO-10646-UCS-4-3412";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          } else if ('\xFF' == aBuf[1]) {
            // FE FF  UTF-16, big endian BOM
            mDetectedCharset = "UTF-16BE";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\x00':
          if (('\x00' == aBuf[1]) && ('\xFE' == aBuf[2]) && ('\xFF' == aBuf[3])) {
            // 00 00 FE FF  UTF-32, big-endian BOM
            mDetectedCharset = "UTF-32BE";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          } else if (('\x00' == aBuf[1]) && ('\xFF' == aBuf[2]) && ('\xFE' == aBuf[3])) {
            // 00 00 FF FE  UCS-4, unusual octet order BOM (2143)
            mDetectedCharset = "X-ISO-10646-UCS-4-2143";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\xFF':
          if (('\xFE' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3])) {
            // FF FE 00 00  UTF-32, little-endian BOM
            mDetectedCharset = "UTF-32LE";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          } else if ('\xFE' == aBuf[1]) {
            // FF FE  UTF-16, little endian BOM
            mDetectedCharset = "UTF-16LE";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\x2B':
          if (('\x2F' == aBuf[1]) && ('\x76' == aBuf[2])) {
            switch (aBuf[3]) {
              case '\x38':
              case '\x39':
              case '\x2B':
              case '\x2F':
                // https://en.wikipedia.org/wiki/Byte_order_mark#Byte_order_marks_by_encoding
                // 2B 2F 76 38  UTF-7
                // 2B 2F 76 39  UTF-7
                // 2B 2F 76 2B  UTF-7
                // 2B 2F 76 2F  UTF-7
                mDetectedCharset = "UTF-7";
                mDetectedConfidence = 1.0;
                mDetectedIsBOM = 1;
              break;
            }
          }
        break;
        case '\xE7':
          if (('\x64' == aBuf[1]) && ('\x4C' == aBuf[2])) {
            // E7 64 4c  UTF-1 encoded BOM
            mDetectedCharset = "UTF-1";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\xDD':
          if (('\x73' == aBuf[1]) && ('\x66' == aBuf[2]) && ('\x73' == aBuf[3])) {
            // DD 73 66 73  UTF-EBCDIC encoded BOM
            mDetectedCharset = "UTF-EBCDIC";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          } 
        break;
        case '\x0E':
          if (('\xFE' == aBuf[1]) && ('\xFF' == aBuf[2])) {
            // 0E FE FF  SCSU encoded BOM
            mDetectedCharset = "SCSU";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\xFB':
          if (('\xEE' == aBuf[1]) && ('\x28' == aBuf[2])) {
            // FB EE 28  BOCU-1 encoded BOM
            mDetectedCharset = "BOCU-1";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          }
        break;
        case '\x84':
          if (('\x31' == aBuf[1]) && ('\x95' == aBuf[2]) && ('\x33' == aBuf[3])) {
            // 84 31 95 33  GB18030 encoded BOM
            mDetectedCharset = "GB18030";
            mDetectedConfidence = 1.0;
            mDetectedIsBOM = 1;
          } 
        break;
      }  // switch

      if (mDetectedCharset)
      {
        mDone = PR_TRUE;
        return NS_OK;
      }
  }

  PRUint32 i;
  for (i = 0; i < aLen; i++)
  {
    //other than 0xa0, if every othe character is ascii, the page is ascii
    if (aBuf[i] & '\x80' && aBuf[i] != '\xA0')  //Since many Ascii only page contains NBSP
    {
      //we got a non-ascii byte (high-byte)
      if (mInputState != eHighbyte)
      {
        //adjust state
        mInputState = eHighbyte;

        //kill mEscCharSetProber if it is active
        if (mEscCharSetProber) {
          delete mEscCharSetProber;
          mEscCharSetProber = nsnull;
        }

        //start multibyte and singlebyte charset prober
        if (nsnull == mCharSetProbers[0])
        {
          mCharSetProbers[0] = new nsMBCSGroupProber(mLanguageFilter);
          if (nsnull == mCharSetProbers[0])
            return NS_ERROR_OUT_OF_MEMORY;
        }
        if (nsnull == mCharSetProbers[1] &&
            (mLanguageFilter & NS_FILTER_NON_CJK))
        {
          mCharSetProbers[1] = new nsSBCSGroupProber;
          if (nsnull == mCharSetProbers[1])
            return NS_ERROR_OUT_OF_MEMORY;
        }
        if (nsnull == mCharSetProbers[2])
        {
          mCharSetProbers[2] = new nsLatin1Prober;
          if (nsnull == mCharSetProbers[2])
            return NS_ERROR_OUT_OF_MEMORY;
        }
      }
    }
    else
    {
      if (aBuf[i] == '\xA0')
      {
        mNbspFound = PR_TRUE;
      }
      //ok, just pure ascii so far
      else if ( ePureAscii == mInputState &&
        (aBuf[i] == '\033' || (aBuf[i] == '{' && mLastChar == '~')) )
      {
        //found escape character or HZ "~{"
        mInputState = eEscAscii;
      }
      mLastChar = aBuf[i];
    }
  }

  nsProbingState st;
  switch (mInputState)
  {
  case eEscAscii:
    if (nsnull == mEscCharSetProber) {
      mEscCharSetProber = new nsEscCharSetProber(mLanguageFilter);
      if (nsnull == mEscCharSetProber)
        return NS_ERROR_OUT_OF_MEMORY;
    }
    st = mEscCharSetProber->HandleData(aBuf, aLen);
    mDone = PR_TRUE;
    if (st == eFoundIt)
    {
      mDetectedCharset = mEscCharSetProber->GetCharSetName();
      mDetectedConfidence = mEscCharSetProber->GetConfidence();
    }
    else
    {
      mDetectedCharset = mNbspFound ? "ISO-8859-1" : "ASCII";
      mDetectedConfidence = 1.0;
    }
    break;
  case eHighbyte:
    for (i = 0; i < NUM_OF_CHARSET_PROBERS; i++)
    {
      if (mCharSetProbers[i])
      {
        st = mCharSetProbers[i]->HandleData(aBuf, aLen);
        if (st == eFoundIt) 
        {
          mDone = PR_TRUE;
          mDetectedCharset = mCharSetProbers[i]->GetCharSetName();
          mDetectedConfidence = mCharSetProbers[i]->GetConfidence();
          return NS_OK;
        }
      }
    }
    break;

  default:  //pure ascii
    mDone = PR_TRUE;
    mDetectedCharset = mNbspFound ? "ISO-8859-1" : "ASCII";
    mDetectedConfidence = 1.0;
    mDetectedIsBOM = 0;
  }
  return NS_OK;
}


//---------------------------------------------------------------------
void nsUniversalDetector::DataEnd()
{
  if (!mGotData)
  {
    // we haven't got any data yet, return immediately
    // caller program sometimes call DataEnd before anything has been sent to detector
    return;
  }

  if (mDetectedCharset)
  {
    mDone = PR_TRUE;
    Report(mDetectedCharset, mDetectedConfidence);
    return;
  }

  switch (mInputState)
  {
  case eHighbyte:
    {
      float maxProberConfidence = (float)0.0;
      PRInt32 maxProber = 0;

      for (PRInt32 i = 0; i < NUM_OF_CHARSET_PROBERS; ++i)
      {
        if (mCharSetProbers[i])
        {
          float const proberConfidence = mCharSetProbers[i]->GetConfidence();
          if (proberConfidence > maxProberConfidence)
          {
            maxProberConfidence = proberConfidence;
            maxProber = i;
          }
        }
      }
      mDetectedConfidence = maxProberConfidence;

      //do not report anything because we are not confident of it, that's in fact a negative answer
      if (maxProberConfidence > MINIMUM_THRESHOLD) {
        Report(mCharSetProbers[maxProber]->GetCharSetName(), maxProberConfidence);
        mDetectedConfidence = mCharSetProbers[maxProber]->GetConfidence();
      }
    }
    break;
  case eEscAscii:
    break;
  default:
    break;
  }
  return;
}
