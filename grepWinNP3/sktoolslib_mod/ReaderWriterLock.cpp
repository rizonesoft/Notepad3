/*********************************************************************
CReaderWriterLock: A simple and fast reader-writer lock class in C++
has characters of .NET ReaderWriterLock class
Copyright (C) 2006 Quynh Nguyen Huu

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

Email questions, comments or suggestions to quynhnguyenhuu@gmail.com
*********************************************************************/

#include "stdafx.h"
#include <crtdbg.h>
#include "ReaderWriterLock.h"

/////////////////////////////////////////////////////////////////
// Following macros make this file can be used in non-MFC project
#ifndef ASSERT
// Define ASSERT macro
#   define ASSERT _ASSERT

// Define VERIFY macro
#   ifdef _DEBUG
#       define VERIFY ASSERT
#       define DEBUG_ONLY(f) ((void)(f))
#   else
#       define VERIFY(f) ((void)(f))
#       define DEBUG_ONLY(f)
#   endif
#endif

///////////////////////////////////////////////////////
// CReaderWriterLockNonReentrance implementation

CReaderWriterLockNonReentrance::CReaderWriterLockNonReentrance()
{
    SecureZeroMemory(this, sizeof(*this));
#if (_WIN32_WINNT >= 0x0403)
    InitializeCriticalSectionAndSpinCount(&m_cs, READER_WRITER_SPIN_COUNT);
#else
    InitializeCriticalSection(&m_cs);
#endif
}

CReaderWriterLockNonReentrance::~CReaderWriterLockNonReentrance()
{
    _ASSERT( (NULL == m_hSafeToReadEvent) &&
        (NULL == m_hSafeToWriteEvent) );
    DeleteCriticalSection(&m_cs);
}

bool CReaderWriterLockNonReentrance::_ReaderWait(DWORD dwTimeout)
{
    bool blCanRead;

    ++m_iNumOfReaderWaiting;
    if (NULL == m_hSafeToReadEvent)
    {
        m_hSafeToReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    if (INFINITE == dwTimeout) // INFINITE is a special value
    {
        do
        {
            LeaveCS();
            WaitForSingleObject(m_hSafeToReadEvent, INFINITE);
            // There might be one or more Writers entered, that's
            // why we need DO-WHILE loop here
            EnterCS();
        }
        while (0 != m_iNumOfWriter);

        ++m_iNumOfReaderEntered;
        blCanRead = TRUE;
    }
    else
    {
        LeaveCS();

        DWORD const dwBeginTime = GetTickCount();
        DWORD dwConsumedTime = 0;

        for(;;)
        {
            blCanRead = (WAIT_OBJECT_0 == WaitForSingleObject(m_hSafeToReadEvent,
                dwTimeout - dwConsumedTime));

            EnterCS();

            if (0 == m_iNumOfWriter)
            {
                // Regardless timeout or not, there is no Writer
                // So it's safe to be Reader right now
                ++m_iNumOfReaderEntered;
                blCanRead = TRUE;
                break;
            }

            if (FALSE == blCanRead)
            {
                // Timeout after waiting
                break;
            }

            // There are some Writers have just entered
            // So leave CS and prepare to try again
            LeaveCS();

            dwConsumedTime = GetTickCount() - dwBeginTime;
            if (dwConsumedTime > dwTimeout)
            {
                // Don't worry why the code here looks stupid
                // Because this case rarely happens, it's better
                //  to optimize code for the usual case
                blCanRead = FALSE;
                EnterCS();
                break;
            }
        }
    }

    if (0 == --m_iNumOfReaderWaiting)
    {
        CloseHandle(m_hSafeToReadEvent);
        m_hSafeToReadEvent = NULL;
    }

    return blCanRead;
}

void CReaderWriterLockNonReentrance::_ReaderRelease()
{
    INT _iNumOfReaderEntered = --m_iNumOfReaderEntered;
    _ASSERT(0 <= _iNumOfReaderEntered);

    if ( (0 == _iNumOfReaderEntered) &&
        (NULL != m_hSafeToWriteEvent) )
    {
        SetEvent(m_hSafeToWriteEvent);
    }
}

bool CReaderWriterLockNonReentrance::_WriterWaitAndLeaveCSIfSuccess(DWORD dwTimeout)
{
    //EnterCS();
    _ASSERT(0 != dwTimeout);

    // Increase Writer-counter & reset Reader-event if necessary
    INT _iNumOfWriter = ++m_iNumOfWriter;
    if ( (1 == _iNumOfWriter) && (NULL != m_hSafeToReadEvent) )
    {
        ResetEvent(m_hSafeToReadEvent);
    }

    if (NULL == m_hSafeToWriteEvent)
    {
        m_hSafeToWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    LeaveCS();

    bool blCanWrite = (WAIT_OBJECT_0 == WaitForSingleObject(m_hSafeToWriteEvent, dwTimeout));
    if (FALSE == blCanWrite)
    {
        // Undo what we changed after timeout
        EnterCS();
        if (0 == --m_iNumOfWriter)
        {
            CloseHandle(m_hSafeToWriteEvent);
            m_hSafeToWriteEvent = NULL;

            if (0 == m_iNumOfReaderEntered)
            {
                // Although it was timeout, it's still safe to be writer now
                ++m_iNumOfWriter;
                LeaveCS();
                blCanWrite = TRUE;
            }
            else if (m_hSafeToReadEvent)
            {
                SetEvent(m_hSafeToReadEvent);
            }
        }
    }
    return blCanWrite;
}

bool CReaderWriterLockNonReentrance::_UpgradeToWriterLockAndLeaveCS(DWORD dwTimeout)
{
    _ASSERT(m_iNumOfReaderEntered > 0);

    if (0 == dwTimeout)
    {
        LeaveCS();
        return FALSE;
    }

    --m_iNumOfReaderEntered;
    bool blCanWrite = _WriterWaitAndLeaveCSIfSuccess(dwTimeout);
    if (FALSE == blCanWrite)
    {
        // Now analyze why it was failed to have suitable action
        if (0 == m_iNumOfWriter)
        {
            _ASSERT(0 < m_iNumOfReaderEntered);
            // There are some readers still owning the lock
            // It's safe to be a reader again after failure
            ++m_iNumOfReaderEntered;
        }
        else
        {
            // Reach to here, it's NOT safe to be a reader immediately
            _ReaderWait(INFINITE);
            if (1 == m_iNumOfReaderEntered)
            {
                // After wait, now it's safe to be writer
                _ASSERT(0 == m_iNumOfWriter);
                m_iNumOfReaderEntered = 0;
                m_iNumOfWriter = 1;
                blCanWrite = TRUE;
            }
        }
        LeaveCS();
    }

    return blCanWrite;
}

void CReaderWriterLockNonReentrance::_WriterRelease(bool blDowngrade)
{
    _ASSERT(0 == m_iNumOfReaderEntered);

    if (blDowngrade)
    {
        ++m_iNumOfReaderEntered;
    }

    if (0 == --m_iNumOfWriter)
    {
        if (NULL != m_hSafeToWriteEvent)
        {
            CloseHandle(m_hSafeToWriteEvent);
            m_hSafeToWriteEvent = NULL;
        }

        if (m_hSafeToReadEvent)
        {
            SetEvent(m_hSafeToReadEvent);
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////////////
        // Some WRITERs are queued
        _ASSERT( (0 < m_iNumOfWriter) && (NULL != m_hSafeToWriteEvent));

        if (FALSE == blDowngrade)
        {
            SetEvent(m_hSafeToWriteEvent);
        }
    }
}

bool CReaderWriterLockNonReentrance::AcquireReaderLock(DWORD dwTimeout)
{
    bool blCanRead;

    EnterCS();
    if (0 == m_iNumOfWriter)
    {
        // Enter successful without wait
        ++m_iNumOfReaderEntered;
        blCanRead = TRUE;
    }
    else
    {
        blCanRead = (dwTimeout)? _ReaderWait(dwTimeout) : FALSE;
    }
    LeaveCS();

    return blCanRead;
}

void CReaderWriterLockNonReentrance::ReleaseReaderLock()
{
    EnterCS();
    _ReaderRelease();
    LeaveCS();
}

bool CReaderWriterLockNonReentrance::AcquireWriterLock(DWORD dwTimeout)
{
    bool blCanWrite ;

    EnterCS();
    if (0 == (m_iNumOfWriter | m_iNumOfReaderEntered))
    {
        ++m_iNumOfWriter;
        blCanWrite = TRUE;
    }
    else if (0 == dwTimeout)
    {
        blCanWrite = FALSE;
    }
    else
    {
        blCanWrite = _WriterWaitAndLeaveCSIfSuccess(dwTimeout);
        if (blCanWrite)
        {
            return TRUE;
        }
    }

    LeaveCS();
    return blCanWrite;
}

void CReaderWriterLockNonReentrance::ReleaseWriterLock()
{
    EnterCS();
    _WriterRelease(FALSE);
    LeaveCS();
}

void CReaderWriterLockNonReentrance::DowngradeFromWriterLock()
{
    EnterCS();
    _WriterRelease(TRUE);
    LeaveCS();
}

bool CReaderWriterLockNonReentrance::UpgradeToWriterLock(DWORD dwTimeout)
{
    EnterCS();
    return _UpgradeToWriterLockAndLeaveCS(dwTimeout);
}

// END CReaderWriterLockNonReentrance implementation
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
// CReaderWriterLock implementation

#define READER_RECURRENCE_UNIT 0x00000001
#define READER_RECURRENCE_MASK 0x0000FFFF
#define WRITER_RECURRENCE_UNIT 0x00010000

CReaderWriterLock::CReaderWriterLock()
{
}

CReaderWriterLock::~CReaderWriterLock()
{
}

bool CReaderWriterLock::AcquireReaderLock(DWORD dwTimeout)
{
    const DWORD dwCurrentThreadId = GetCurrentThreadId();

    m_impl.EnterCS();
    CMapThreadToState::iterator ite = m_map.find(dwCurrentThreadId);

    if (ite != m_map.end())
    {
        //////////////////////////////////////////////////////////////////////////
        // Current thread was already a WRITER or READER
        _ASSERT(0 < ite->second);
        ite->second += READER_RECURRENCE_UNIT;
        m_impl.LeaveCS();

        return TRUE;
    }

    if (0 == m_impl.m_iNumOfWriter)
    {
        // There is NO WRITER on this RW object
        // Current thread is going to be a READER
        ++m_impl.m_iNumOfReaderEntered;
        m_map.insert(std::make_pair(dwCurrentThreadId, READER_RECURRENCE_UNIT));

        m_impl.LeaveCS();
        return TRUE;
    }

    if (0 == dwTimeout)
    {
        m_impl.LeaveCS();
        return FALSE;
    }

    bool blCanRead = m_impl._ReaderWait(dwTimeout);
    if (blCanRead)
    {
        m_map.insert(std::make_pair(dwCurrentThreadId, READER_RECURRENCE_UNIT));
    }
    m_impl.LeaveCS();

    return blCanRead;
}

void CReaderWriterLock::ReleaseReaderLock()
{
    const DWORD dwCurrentThreadId = GetCurrentThreadId();
    m_impl.EnterCS();

    CMapThreadToState::iterator ite = m_map.find(dwCurrentThreadId);
    _ASSERT( (ite != m_map.end()) && (READER_RECURRENCE_MASK & ite->second));

    const DWORD dwThreadState = (ite->second -= READER_RECURRENCE_UNIT);
    if (0 == dwThreadState)
    {
        m_map.erase(ite);
        m_impl._ReaderRelease();
    }
    m_impl.LeaveCS();
}

bool CReaderWriterLock::AcquireWriterLock(DWORD dwTimeout)
{
    const DWORD dwCurrentThreadId = GetCurrentThreadId();
    bool blCanWrite;

    m_impl.EnterCS();
    CMapThreadToState::iterator ite = m_map.find(dwCurrentThreadId);

    if (ite != m_map.end())
    {
        _ASSERT(0 < ite->second);

        if (ite->second >= WRITER_RECURRENCE_UNIT)
        {
            // Current thread was already a WRITER
            ite->second += WRITER_RECURRENCE_UNIT;
            m_impl.LeaveCS();
            return TRUE;
        }

        // Current thread was already a READER
        _ASSERT(1 <= m_impl.m_iNumOfReaderEntered);
        if (1 == m_impl.m_iNumOfReaderEntered)
        {
            // This object is owned by ONLY current thread for READ
            // There might be some threads queued to be WRITERs
            // but for effectiveness (higher throughput), we allow current
            // thread upgrading to be WRITER right now
            m_impl.m_iNumOfReaderEntered = 0;
            ++m_impl.m_iNumOfWriter;
            ite->second += WRITER_RECURRENCE_UNIT;
            m_impl.LeaveCS();
            return TRUE;
        }

        // Try upgrading from reader to writer
        blCanWrite = m_impl._UpgradeToWriterLockAndLeaveCS(dwTimeout);
        if (blCanWrite)
        {
            m_impl.EnterCS();
            ite = m_map.find(dwCurrentThreadId);
            ite->second += WRITER_RECURRENCE_UNIT;
            m_impl.LeaveCS();
        }
    }
    else
    {
        if (0 == (m_impl.m_iNumOfWriter | m_impl.m_iNumOfReaderEntered))
        {
            // This RW object is not owned by any thread
            // --> it's safe to make this thread to be WRITER
            ++m_impl.m_iNumOfWriter;
            m_map.insert(std::make_pair(dwCurrentThreadId, WRITER_RECURRENCE_UNIT));
            m_impl.LeaveCS();
            return TRUE;
        }

        if (0 == dwTimeout)
        {
            m_impl.LeaveCS();
            return FALSE;
        }

        blCanWrite = m_impl._WriterWaitAndLeaveCSIfSuccess(dwTimeout);
        if (blCanWrite)
        {
            m_impl.EnterCS();
            m_map.insert(std::make_pair(dwCurrentThreadId, WRITER_RECURRENCE_UNIT));
        }
        m_impl.LeaveCS();
    }

    return blCanWrite;
}

void CReaderWriterLock::ReleaseWriterLock()
{
    const DWORD dwCurrentThreadId = GetCurrentThreadId();
    m_impl.EnterCS();

    CMapThreadToState::iterator ite = m_map.find(dwCurrentThreadId);
    _ASSERT( (ite != m_map.end()) && (WRITER_RECURRENCE_UNIT <= ite->second));

    const DWORD dwThreadState = (ite->second -= WRITER_RECURRENCE_UNIT);
    if (0 == dwThreadState)
    {
        m_map.erase(ite);
        m_impl._WriterRelease(FALSE);
    }
    else if (WRITER_RECURRENCE_UNIT > dwThreadState)
    {
        // Down-grading from writer to reader
        m_impl._WriterRelease(TRUE);
    }
    m_impl.LeaveCS();
}

void CReaderWriterLock::ReleaseAllLocks()
{
    const DWORD dwCurrentThreadId = GetCurrentThreadId();

    m_impl.EnterCS();
    CMapThreadToState::iterator ite = m_map.find(dwCurrentThreadId);
    if (ite != m_map.end())
    {
        const DWORD dwThreadState = ite->second;
        m_map.erase(ite);
        if (WRITER_RECURRENCE_UNIT <= dwThreadState)
        {
            m_impl._WriterRelease(FALSE);
        }
        else
        {
            _ASSERT(0 < dwThreadState);
            m_impl._ReaderRelease();
        }
    }
    m_impl.LeaveCS();
}

DWORD CReaderWriterLock::GetCurrentThreadStatus() const
{
    DWORD dwThreadState;
    const DWORD dwCurrentThreadId = GetCurrentThreadId();

    m_impl.EnterCS();
    CMapThreadToState::const_iterator ite = m_map.find(dwCurrentThreadId);
    if (ite != m_map.end())
    {
        dwThreadState = ite->second;
        m_impl.LeaveCS();
        _ASSERT(dwThreadState > 0);
    }
    else
    {
        dwThreadState = 0;
        m_impl.LeaveCS();
    }

    return dwThreadState;
}

void CReaderWriterLock::GetCurrentThreadStatus(DWORD* lpdwReaderLockCounter,
    DWORD* lpdwWriterLockCounter) const
{
    const DWORD dwThreadState = GetCurrentThreadStatus();

    if (NULL != lpdwReaderLockCounter)
    {
        *lpdwReaderLockCounter = (dwThreadState & READER_RECURRENCE_MASK);
    }

    if (NULL != lpdwWriterLockCounter)
    {
        *lpdwWriterLockCounter = (dwThreadState / WRITER_RECURRENCE_UNIT);
    }
}

