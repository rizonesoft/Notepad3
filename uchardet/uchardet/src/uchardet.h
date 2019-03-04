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
 *          Jehan <jehan at girinstud.io>
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
#ifndef UCHARDET_H___
#define UCHARDET_H___

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * A handle for a uchardet encoding detector.
 */
typedef struct uchardet * uchardet_t;

/**
 * Create an encoding detector.
 * @return an instance of uchardet_t.
 */
uchardet_t uchardet_new(void);

/**
 * Delete an encoding detector.
 * @param ud [in] the uchardet_t handle to delete.
 */
void uchardet_delete(uchardet_t ud);

#define HANDLE_DATA_RESULT_ERROR -1
#define HANDLE_DATA_RESULT_DETECTED 0
#define HANDLE_DATA_RESULT_NEED_MORE_DATA 1

/**
 * Feed data to an encoding detector.
 * The detector is able to shortcut processing when it reaches certainty
 * for an encoding, so you should not worry about limiting input data.
 * As far as you should be concerned: the more the better.
 *
 * @param ud [in] handle of an instance of uchardet
 * @param data [in] data
 * @param len [in] number of byte of data
 * @return non-zero number on failure.
 */
int uchardet_handle_data(uchardet_t ud, const char * data, size_t len);

/**
 * Notify an end of data to an encoding detector.
 * @param ud [in] handle of an instance of uchardet
 */
void uchardet_data_end(uchardet_t ud);

/**
 * Reset an encoding detector.
 * @param ud [in] handle of an instance of uchardet
 */
void uchardet_reset(uchardet_t ud);

/**
 * Get an iconv-compatible name of the encoding that was detected.
 * @param ud [in] handle of an instance of uchardet
 * @return name of charset on success and "" on failure.
 */
const char * uchardet_get_charset(uchardet_t ud);

float uchardet_get_confidence(uchardet_t ud);

#ifdef __cplusplus
}
#endif

#endif
