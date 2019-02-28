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
 *          BYVoid <byvoid.kcp@gmail.com>
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
#include "uchardet.h"
#include <string.h>
#include <stdlib.h>
#include "nscore.h"
#include "nsUniversalDetector.h"

class HandleUniversalDetector : public nsUniversalDetector
{
protected:
    char *m_charset;
    float m_confidence;
public:
    HandleUniversalDetector()
    : nsUniversalDetector(NS_FILTER_ALL)
    , m_charset(0)
    {
        m_confidence = 0.0;
    }

    virtual ~HandleUniversalDetector()
    {
        if (m_charset) {
            free(m_charset);
            m_confidence = 0.0;
        }
    }

    virtual void Report(const char* charset, float confidence)
    {
        if (m_charset)
            free(m_charset);
        m_charset = strdup(charset);
        m_confidence = confidence;
    }

    virtual void Reset()
    {
        nsUniversalDetector::Reset();
        if (m_charset)
            free(m_charset);
        m_charset = strdup("");
        m_confidence = 0.0;
    }

    const char* GetCharset() const
    {
        return m_charset? m_charset : "";
    }

    float GetConfidence() {
        return m_confidence;
    }

    PRBool HasDone() {
        return mDone;
    }
};

uchardet_t uchardet_new(void)
{
    return reinterpret_cast<uchardet_t> (new HandleUniversalDetector());
}

void uchardet_delete(uchardet_t ud)
{
    delete reinterpret_cast<HandleUniversalDetector*>(ud);
}

int uchardet_handle_data(uchardet_t ud, const char * data, size_t len)
{
    nsresult ret = reinterpret_cast<HandleUniversalDetector*>(ud)->HandleData(data, (PRUint32)len);
    if (ret == NS_ERROR_OUT_OF_MEMORY) {
        return HANDLE_DATA_RESULT_ERROR;
    }

    if (reinterpret_cast<HandleUniversalDetector*>(ud)->HasDone()) {
        return HANDLE_DATA_RESULT_DETECTED;
    }

    return HANDLE_DATA_RESULT_NEED_MORE_DATA;
}

void uchardet_data_end(uchardet_t ud)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->DataEnd();
}

void uchardet_reset(uchardet_t ud)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->Reset();
}

const char* uchardet_get_charset(uchardet_t ud)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetCharset();
}

float uchardet_get_confidence(uchardet_t ud)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetConfidence();
}
