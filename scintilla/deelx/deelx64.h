// deelx64.h
//
// DEELX Regular Expression Engine (v1.3)
//
// Copyright 2006 ~ 2013 (c) RegExLab.com
// All Rights Reserved.
//
// http://www.regexlab.com/deelx/
// https://github.com/sswater/deelx
//
// Author:  ∑ ŸŒ∞ (sswater shi)
// sswater@gmail.com
//
// + adaption for 64-bit usage: "basetsd : INT_PTR" replaces int-pointer arithmetic and buffer indexes
// + Cppcheck cleanup 
//
// Good Overview:
// https://www.regular-expressions.info/
// 



#ifndef __DEELX_REGEXP64__H__
#define __DEELX_REGEXP64__H__

#include <exception>
#include <memory.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

namespace deelx
{
    // integer type for pointer arithmetic & casts (64-bit aware)
    //typedef int index_t;   // preserve original "deelx.h" v1.3 behavior 
    using index_t = ptrdiff_t; //INT_PTR

extern "C" {
    using POSIX_FUNC = int(*)(int);
    int isblank(int c);
}


class regex_exception : public std::exception
{
  const char* what() const throw() override
  {
    return "DeelX: regex exception occurred!";
  }
} regexExcp;


//
// Data Reference
//
template <class ELT> class CBufferRefT
{
public:
    explicit CBufferRefT(const ELT * pcsz, index_t length);
    explicit CBufferRefT(const ELT * pcsz);

public:
    int nCompare(const ELT * pcsz) const;
    int nCompareNoCase(const ELT * pcsz) const;
    int  Compare(const ELT * pcsz) const;
    int  CompareNoCase(const ELT * pcsz) const;
    int  Compare(const CBufferRefT <ELT> &) const;
    int  CompareNoCase(const CBufferRefT <ELT> &) const;

    ELT At(index_t nIndex, ELT def = 0) const;
    ELT operator [] (index_t nIndex) const;

    const ELT * GetBuffer() const;
    index_t GetSize() const;

public:
    virtual ~CBufferRefT();

    // Content
protected:
    ELT * m_pBuffer;
    index_t  m_nSize;
};

//
// Implemenation
//
template <class ELT> CBufferRefT <ELT> ::CBufferRefT(const ELT * pcsz, index_t length)
    : m_pBuffer((ELT *)pcsz)
    , m_nSize(length)
{
}

template <class ELT> CBufferRefT <ELT> ::CBufferRefT(const ELT * pcsz)
    : m_pBuffer((ELT *)pcsz)
    , m_nSize(0)
{
    if (pcsz != 0) while (m_pBuffer[m_nSize] != 0) m_nSize++;
}

template <class ELT> int CBufferRefT <ELT> ::nCompare(const ELT * pcsz) const
{
    for (index_t i = 0; i < m_nSize; i++)
    {
        if (m_pBuffer[i] != pcsz[i])
            return m_pBuffer[i] - pcsz[i];
    }
    return 0;
}

template <class ELT> int CBufferRefT <ELT> ::nCompareNoCase(const ELT * pcsz) const
{
    for (index_t i = 0; i < m_nSize; i++)
    {
        if (m_pBuffer[i] != pcsz[i])
        {
            if (toupper((int)m_pBuffer[i]) != toupper((int)pcsz[i]))
                return m_pBuffer[i] - pcsz[i];
        }
    }

    return 0;
}

template <class ELT> inline int CBufferRefT <ELT> ::Compare(const ELT * pcsz) const
{
    return nCompare(pcsz) ? 1 : (int)pcsz[m_nSize];
}

template <class ELT> inline int CBufferRefT <ELT> ::CompareNoCase(const ELT * pcsz) const
{
    return nCompareNoCase(pcsz) ? 1 : (int)pcsz[m_nSize];
}

template <class ELT> inline int CBufferRefT <ELT> ::Compare(const CBufferRefT <ELT> & cref) const
{
    return m_nSize == cref.m_nSize ? nCompare(cref.GetBuffer()) : 1;
}

template <class ELT> inline int CBufferRefT <ELT> ::CompareNoCase(const CBufferRefT <ELT> & cref) const
{
    return m_nSize == cref.m_nSize ? nCompareNoCase(cref.GetBuffer()) : 1;
}

template <class ELT> inline ELT CBufferRefT <ELT> ::At(index_t nIndex, ELT def) const
{
    return nIndex >= m_nSize ? def : m_pBuffer[nIndex];
}

template <class ELT> inline ELT CBufferRefT <ELT> :: operator [] (index_t nIndex) const
{
    return nIndex >= m_nSize ? 0 : m_pBuffer[nIndex];
}

template <class ELT> const ELT * CBufferRefT <ELT> ::GetBuffer() const
{
    static const ELT _def[] = { 0 }; return m_pBuffer ? m_pBuffer : _def;
}

template <class ELT> inline index_t CBufferRefT <ELT> ::GetSize() const
{
    return m_nSize;
}

template <class ELT> CBufferRefT <ELT> :: ~CBufferRefT() = default;

//
// Data Buffer
//
template <class ELT> class CBufferT : public CBufferRefT <ELT>
{
public:
    explicit CBufferT(const ELT * pcsz, index_t length);
    explicit CBufferT(const ELT * pcsz);
    CBufferT();

public:
    ELT & operator [] (index_t nIndex);
    const ELT & operator [] (index_t nIndex) const;
    void  Append(const ELT * pcsz, index_t length, index_t eol = 0);
    void  Append(ELT el, index_t eol = 0);

public:
    void  Push(ELT   el);
    void  Push(const CBufferRefT<ELT> & buf);
    int   Pop(ELT & el);
    int   Pop(CBufferT<ELT> & buf);
    int   Peek(ELT & el) const;

public:
    const ELT * GetBuffer() const;
    ELT * GetBuffer();
    ELT * Detach();
    void  Release();
    void  Prepare(index_t index, int fill = 0);
    void  Restore(index_t size);

    ELT * PrepareInsert(index_t nPos, index_t nSize)
    {
        index_t nOldSize = CBufferRefT<ELT>::m_nSize;
        Restore(nPos > CBufferRefT<ELT>::m_nSize ? nPos : CBufferRefT<ELT>::m_nSize + nSize);

        if (nPos < nOldSize)
        {
            ELT * from = CBufferRefT<ELT>::m_pBuffer + nPos, *to = CBufferRefT<ELT>::m_pBuffer + nPos + nSize;
            memmove(to, from, sizeof(ELT) * (nOldSize - nPos));
        }

        return CBufferRefT<ELT>::m_pBuffer + nPos;
    }

    void Insert(index_t nIndex, const ELT & rT)
    {
        Insert(nIndex, &rT, 1);
    }

    void Insert(index_t nIndex, const ELT * pT, index_t nSize)
    {
        memcpy(PrepareInsert(nIndex, nSize), pT, sizeof(ELT) * nSize);
    }

    void Remove(index_t nIndex)
    {
        Remove(nIndex, 1);
    }

    void Remove(index_t nIndex, index_t nSize)
    {
        if (nIndex < CBufferRefT <ELT> ::m_nSize)
        {
            if (nIndex + nSize >= CBufferRefT <ELT> ::m_nSize)
            {
                Restore(nIndex);
            }
            else
            {
                memmove(CBufferRefT <ELT> ::m_pBuffer + nIndex, CBufferRefT <ELT> ::m_pBuffer + nIndex + nSize, sizeof(ELT) * (CBufferRefT <ELT> ::m_nSize - nIndex - nSize));
                Restore(CBufferRefT <ELT> ::m_nSize - nSize);
            }
        }
    }

    void SetMaxLength(index_t nSize)
    {
        if (nSize > m_nMaxLength)
        {
            if (m_nMaxLength < 8)
                m_nMaxLength = 8;

            if (nSize > m_nMaxLength)
                m_nMaxLength *= 2;

            if (nSize > m_nMaxLength)
            {
                m_nMaxLength = nSize + 11;
                m_nMaxLength -= m_nMaxLength & 0x07;
            }

            CBufferRefT <ELT> ::m_pBuffer = (ELT *)realloc(CBufferRefT <ELT> ::m_pBuffer, sizeof(ELT) * m_nMaxLength);
        }
    }

public:
    virtual ~CBufferT();

    // Content
protected:
    index_t   m_nMaxLength;
};

//
// Implemenation
//
template <class ELT> CBufferT <ELT> ::CBufferT(const ELT * pcsz, index_t length) : CBufferRefT <ELT>(0, length)
{
    m_nMaxLength = CBufferRefT <ELT> ::m_nSize + 1;

    CBufferRefT <ELT> ::m_pBuffer = (ELT *)malloc(sizeof(ELT) * m_nMaxLength);
    memcpy(CBufferRefT<ELT>::m_pBuffer, pcsz, sizeof(ELT) * CBufferRefT <ELT> ::m_nSize);
    CBufferRefT<ELT>::m_pBuffer[CBufferRefT <ELT> ::m_nSize] = 0;
}

template <class ELT> CBufferT <ELT> ::CBufferT(const ELT * pcsz) : CBufferRefT <ELT>(pcsz)
{
    m_nMaxLength = CBufferRefT <ELT> ::m_nSize + 1;

    CBufferRefT <ELT> ::m_pBuffer = (ELT *)malloc(sizeof(ELT) * m_nMaxLength);
    memcpy(CBufferRefT<ELT>::m_pBuffer, pcsz, sizeof(ELT) * CBufferRefT <ELT> ::m_nSize);
    CBufferRefT<ELT>::m_pBuffer[CBufferRefT <ELT> ::m_nSize] = 0;
}

template <class ELT> CBufferT <ELT> ::CBufferT() : CBufferRefT <ELT>(0, 0)
{
    m_nMaxLength = 0;
    CBufferRefT<ELT>::m_pBuffer = 0;
}

template <class ELT> inline ELT & CBufferT <ELT> :: operator [] (index_t nIndex)
{
    return CBufferRefT<ELT>::m_pBuffer[nIndex];
}

template <class ELT> inline const ELT & CBufferT <ELT> :: operator [] (index_t nIndex) const
{
    return CBufferRefT<ELT>::m_pBuffer[nIndex];
}

template <class ELT> void CBufferT <ELT> ::Append(const ELT * pcsz, index_t length, index_t eol)
{
    index_t nNewLength = m_nMaxLength;

    // Check length
    if (nNewLength < 8)
        nNewLength = 8;

    if (CBufferRefT <ELT> ::m_nSize + length + eol > nNewLength)
        nNewLength *= 2;

    if (CBufferRefT <ELT> ::m_nSize + length + eol > nNewLength)
    {
        nNewLength = CBufferRefT <ELT> ::m_nSize + length + eol + 11;
        nNewLength -= nNewLength % 8;
    }

    // Realloc
    if (nNewLength > m_nMaxLength)
    {
        CBufferRefT <ELT> ::m_pBuffer = (ELT *)realloc(CBufferRefT<ELT>::m_pBuffer, sizeof(ELT) * nNewLength);
        m_nMaxLength = nNewLength;
    }

    // Append
    memcpy(CBufferRefT<ELT>::m_pBuffer + CBufferRefT <ELT> ::m_nSize, pcsz, sizeof(ELT) * length);
    CBufferRefT <ELT> ::m_nSize += length;

    if (eol > 0) CBufferRefT<ELT>::m_pBuffer[CBufferRefT <ELT> ::m_nSize] = 0;
}

template <class ELT> inline void CBufferT <ELT> ::Append(ELT el, index_t eol)
{
    Append(&el, 1, eol);
}

template <class ELT> void CBufferT <ELT> ::Push(ELT el)
{
    // Realloc
    if (CBufferRefT <ELT> ::m_nSize >= m_nMaxLength)
    {
        index_t nNewLength = m_nMaxLength * 2;
        if (nNewLength < 8) nNewLength = 8;

        CBufferRefT <ELT> ::m_pBuffer = (ELT *)realloc(CBufferRefT<ELT>::m_pBuffer, sizeof(ELT) * nNewLength);
        m_nMaxLength = nNewLength;
    }

    // Append
    CBufferRefT<ELT>::m_pBuffer[CBufferRefT <ELT> ::m_nSize++] = el;
}

template <class ELT> void CBufferT <ELT> ::Push(const CBufferRefT<ELT> & buf)
{
    for (index_t i = 0; i < buf.GetSize(); i++)
    {
        Push(buf[i]);
    }

    Push((ELT)buf.GetSize());
}

template <class ELT> inline int CBufferT <ELT> ::Pop(ELT & el)
{
    if (CBufferRefT <ELT> ::m_nSize > 0)
    {
        el = CBufferRefT<ELT>::m_pBuffer[--CBufferRefT <ELT> ::m_nSize];
        return 1;
    }
    else
    {
        return 0;
    }
}

template <class ELT> int CBufferT <ELT> ::Pop(CBufferT<ELT> & buf)
{
    index_t size;
    int res = 1;
    res = res && Pop(*(ELT*)&size);
    buf.Restore(size);

    for (index_t i = size - 1; i >= 0; i--)
    {
        res = res && Pop(buf[i]);
    }

    return res;
}

template <class ELT> inline int CBufferT <ELT> ::Peek(ELT & el) const
{
    if (CBufferRefT <ELT> ::m_nSize > 0)
    {
        el = CBufferRefT<ELT>::m_pBuffer[CBufferRefT <ELT> ::m_nSize - 1];
        return 1;
    }
    else
    {
        return 0;
    }
}

template <class ELT> const ELT * CBufferT <ELT> ::GetBuffer() const
{
    static const ELT _def[] = { 0 }; return CBufferRefT<ELT>::m_pBuffer ? CBufferRefT<ELT>::m_pBuffer : _def;
}

template <class ELT> ELT * CBufferT <ELT> ::GetBuffer()
{
    static const ELT _def[] = { 0 }; return CBufferRefT<ELT>::m_pBuffer ? CBufferRefT<ELT>::m_pBuffer : (ELT *)_def;
}

template <class ELT> ELT * CBufferT <ELT> ::Detach()
{
    ELT * pBuffer = CBufferRefT<ELT>::m_pBuffer;

    CBufferRefT <ELT> ::m_pBuffer = 0;
    CBufferRefT <ELT> ::m_nSize = m_nMaxLength = 0;

    return pBuffer;
}

template <class ELT> void CBufferT <ELT> ::Release()
{
    ELT * pBuffer = Detach();

    if (pBuffer != 0) free(pBuffer);
}

template <class ELT> void CBufferT <ELT> ::Prepare(index_t index, int fill)
{
    index_t nNewSize = index + 1;

    // Realloc
    if (nNewSize > m_nMaxLength)
    {
        index_t nNewLength = m_nMaxLength;

        if (nNewLength < 8)
            nNewLength = 8;

        if (nNewSize > nNewLength)
            nNewLength *= 2;

        if (nNewSize > nNewLength)
        {
            nNewLength = nNewSize + 11;
            nNewLength -= nNewLength % 8;
        }

        CBufferRefT <ELT> ::m_pBuffer = (ELT *)realloc(CBufferRefT<ELT>::m_pBuffer, sizeof(ELT) * nNewLength);
        m_nMaxLength = nNewLength;
    }

    // size
    if (CBufferRefT <ELT> ::m_nSize < nNewSize)
    {
        memset(CBufferRefT<ELT>::m_pBuffer + CBufferRefT <ELT> ::m_nSize, fill, sizeof(ELT) * (nNewSize - CBufferRefT <ELT> ::m_nSize));
        CBufferRefT <ELT> ::m_nSize = nNewSize;
    }
}

template <class ELT> inline void CBufferT <ELT> ::Restore(index_t size)
{
    SetMaxLength(size);
    CBufferRefT <ELT> ::m_nSize = size;
}

template <class ELT> CBufferT <ELT> :: ~CBufferT()
{
    if (CBufferRefT<ELT>::m_pBuffer != 0) free(CBufferRefT<ELT>::m_pBuffer);
}

template <class T> class CSortedBufferT : public CBufferT <T>
{
public:
    explicit CSortedBufferT(int reverse = 0);
    explicit CSortedBufferT(int(*)(const void *, const void *));

public:
    void Add(const T & rT);
    void Add(const T * pT, index_t nSize);
    int  Remove(const T & rT);
    static void RemoveAll();

    void SortFreeze() { m_bSortFreezed = 1; }
    void SortUnFreeze();

public:
    int  Find(const T & rT, int(*compare)(const void *, const void *) = nullptr) { return FindAs(*(T*)&rT, compare); }
    int  FindAs(const T & rT, int(*)(const void *, const void *) = nullptr);
    index_t  GetSize() const { return CBufferRefT<T>::m_nSize; }
    T & operator [] (index_t nIndex) { return CBufferT <T> :: operator [] (nIndex); }

protected:
    int(*m_fncompare)(const void *, const void *);
    static int compareT(const void *, const void *);
    static int compareReverseT(const void *, const void *);

    int  m_bSortFreezed;
};

template <class T> CSortedBufferT <T> ::CSortedBufferT(int reverse)
{
    m_fncompare = reverse ? compareReverseT : compareT;
    m_bSortFreezed = 0;
}

template <class T> CSortedBufferT <T> ::CSortedBufferT(int(*compare)(const void *, const void *))
{
    m_fncompare = compare;
    m_bSortFreezed = 0;
}

template <class T> void CSortedBufferT <T> ::Add(const T & rT)
{
    if (m_bSortFreezed != 0)
    {
        CBufferT<T> ::Append(rT);
        return;
    }

    index_t a = 0, b = CBufferRefT<T>::m_nSize - 1, c = CBufferRefT<T>::m_nSize / 2;

    while (a <= b)
    {
        int r = m_fncompare(&rT, &CBufferRefT<T>::m_pBuffer[c]);

        if (r < 0) b = c - 1;
        else if (r > 0) a = c + 1;
        else break;

        c = (a + b + 1) / 2;
    }

    CBufferT<T> ::Insert(c, rT);
}

template <class T> void CSortedBufferT <T> ::Add(const T * pT, index_t nSize)
{
    CBufferT<T> ::Append(pT, nSize);

    if (m_bSortFreezed == 0)
    {
        qsort(CBufferRefT<T>::m_pBuffer, CBufferRefT<T>::m_nSize, sizeof(T), m_fncompare);
    }
}

template <class T> int CSortedBufferT <T> ::FindAs(const T & rT, int(*compare)(const void *, const void *))
{
    const T * pT = (const T *)bsearch(&rT, CBufferRefT<T>::m_pBuffer, CBufferRefT<T>::m_nSize, sizeof(T), compare == nullptr ? m_fncompare : compare);

    if (pT != NULL)
        return static_cast<int>(pT - CBufferRefT<T>::m_pBuffer); //TODO: x64bit > 4GB ?
    else
        return -1;
}

template <class T> int CSortedBufferT <T> ::Remove(const T & rT)
{
    int pos = Find(rT);
    if (pos >= 0) CBufferT <T> ::Remove(pos);
    return pos;
}

template <class T> inline void CSortedBufferT <T> ::RemoveAll()
{
    CBufferT<T>::Restore(0);
}

template <class T> void CSortedBufferT <T> ::SortUnFreeze()
{
    if (m_bSortFreezed != 0)
    {
        m_bSortFreezed = 0;
        qsort(CBufferRefT<T>::m_pBuffer, CBufferRefT<T>::m_nSize, sizeof(T), m_fncompare);
    }
}

template <class T> int CSortedBufferT <T> ::compareT(const void * elem1, const void * elem2)
{
    if (*(const T *)elem1 == *(const T *)elem2)
        return 0;
    else if (*(const T *)elem1 < *(const T *)elem2)
        return -1;
    else
        return 1;
}

template <class T> int CSortedBufferT <T> ::compareReverseT(const void * elem1, const void * elem2)
{
    if (*(const T *)elem1 == *(const T *)elem2)
        return 0;
    else if (*(const T *)elem1 > *(const T *)elem2)
        return -1;
    else
        return 1;
}

//
// Context
//
class CContext
{
public:
    CBufferT <index_t> m_stack;
    CBufferT <index_t> m_capturestack, m_captureindex;

public:
    index_t    m_nCurrentPos;
    index_t    m_nBeginPos;
    index_t    m_nLastBeginPos;
    index_t    m_nParenZindex;
    index_t    m_nCursiveLimit;

    void * m_pMatchString;
    index_t    m_pMatchStringLength;
};

class CContextShot
{
public:
    explicit CContextShot(CContext * pContext)
    {
        m_nCurrentPos = pContext->m_nCurrentPos;
        nsize = pContext->m_stack.GetSize();
        ncsize = pContext->m_capturestack.GetSize();
    }

    void Restore(CContext * pContext)
    {
        pContext->m_stack.Restore(nsize);
        pContext->m_capturestack.Restore(ncsize);
        pContext->m_nCurrentPos = m_nCurrentPos;
    }

public:
    index_t m_nCurrentPos;
    index_t nsize;
    index_t ncsize;
};

//
// Interface
//
class ElxInterface
{
public:
    virtual int Match(CContext * pContext) const = 0;
    virtual int MatchNext(CContext * pContext) const = 0;

public:
    virtual ~ElxInterface() = default;
};

//
// Alternative
//
template <index_t x> class CAlternativeElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CAlternativeElxT();

public:
    CBufferT <ElxInterface *> m_elxlist;
};

using CAlternativeElx = CAlternativeElxT <0>;

//
// Assert
//
template <index_t x> class CAssertElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CAssertElxT(ElxInterface * pelx, int byes = 1);

public:
    ElxInterface * m_pelx;
    int m_byes;
};

using CAssertElx = CAssertElxT <0>;

//
// Back reference elx
//
template <class CHART> class CBackrefElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CBackrefElxT(int nnumber, int brightleft, int bignorecase);

public:
    index_t m_nnumber;
    int m_brightleft;
    int m_bignorecase;

    CBufferT <CHART> m_szNamed;
};

//
// Implementation
//
template <class CHART> CBackrefElxT <CHART> ::CBackrefElxT(int nnumber, int brightleft, int bignorecase)
{
    m_nnumber = nnumber;
    m_brightleft = brightleft;
    m_bignorecase = bignorecase;
}

template <class CHART> int CBackrefElxT <CHART> ::Match(CContext * pContext) const
{
    // check number, for named
    if (m_nnumber < 0 || m_nnumber >= pContext->m_captureindex.GetSize()) return 0;

    index_t index = pContext->m_captureindex[m_nnumber];
    if (index < 0) return 0;

    // check enclosed
    index_t pos1 = pContext->m_capturestack[index + 1];
    index_t pos2 = pContext->m_capturestack[index + 2];

    if (pos2 < 0) pos2 = pContext->m_nCurrentPos;

    // info
    index_t lpos = pos1 < pos2 ? pos1 : pos2;
    index_t rpos = pos1 < pos2 ? pos2 : pos1;
    index_t slen = rpos - lpos;

    const CHART * pcsz = (const CHART *)pContext->m_pMatchString;
    index_t npos = pContext->m_nCurrentPos;
    index_t tlen = pContext->m_pMatchStringLength;

    // compare
    int bsucc;
    CBufferRefT <CHART> refstr(pcsz + lpos, slen);

    if (m_brightleft)
    {
        if (npos < slen)
            return 0;

        if (m_bignorecase)
            bsucc = !refstr.nCompareNoCase(pcsz + (npos - slen));
        else
            bsucc = !refstr.nCompare(pcsz + (npos - slen));

        if (bsucc)
        {
            pContext->m_stack.Push(npos);
            pContext->m_nCurrentPos -= slen;
        }
    }
    else
    {
        if (npos + slen > tlen)
            return 0;

        if (m_bignorecase)
            bsucc = !refstr.nCompareNoCase(pcsz + npos);
        else
            bsucc = !refstr.nCompare(pcsz + npos);

        if (bsucc)
        {
            pContext->m_stack.Push(npos);
            pContext->m_nCurrentPos += slen;
        }
    }

    return bsucc;
}

template <class CHART> int CBackrefElxT <CHART> ::MatchNext(CContext * pContext) const
{
    index_t npos = 0;

    pContext->m_stack.Pop(npos);
    pContext->m_nCurrentPos = npos;

    return 0;
}

// RCHART
#ifndef RCHART
#define RCHART(ch) ((CHART)ch)
#endif

// BOUNDARY_TYPE
enum BOUNDARY_TYPE
{
    BOUNDARY_FILE_BEGIN, // begin of whole text
    BOUNDARY_FILE_END, // end of whole text
    BOUNDARY_FILE_END_N, // end of whole text, or before newline at the end
    BOUNDARY_LINE_BEGIN, // begin of line
    BOUNDARY_LINE_END, // end of line
    BOUNDARY_WORD_BEGIN, // begin of word
    BOUNDARY_WORD_END, // end of word
    BOUNDARY_WORD_EDGE
};

//
// Boundary Elx
//
template <class CHART> class CBoundaryElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CBoundaryElxT(int ntype, int byes = 1);

protected:
    static int IsWordChar(CHART ch);

public:
    int m_ntype;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CBoundaryElxT <CHART> ::CBoundaryElxT(int ntype, int byes)
{
    m_ntype = ntype;
    m_byes = byes;
}

template <class CHART> int CBoundaryElxT <CHART> ::Match(CContext * pContext) const
{
    const CHART * pcsz = (const CHART *)pContext->m_pMatchString;
    index_t npos = pContext->m_nCurrentPos;
    index_t tlen = pContext->m_pMatchStringLength;

    CHART chL = npos > 0 ? pcsz[npos - 1] : 0;
    CHART chR = npos < tlen ? pcsz[npos] : 0;

    int bsucc = 0;

    switch (m_ntype)
    {
    case BOUNDARY_FILE_BEGIN:
        bsucc = (npos <= 0);
        break;

    case BOUNDARY_FILE_END:
        bsucc = (npos >= tlen);
        break;

    case BOUNDARY_FILE_END_N:
        bsucc = (npos >= tlen) || (pcsz[tlen - 1] == RCHART('\n') && (npos == tlen - 1 || (pcsz[tlen - 2] == RCHART('\r') && npos == tlen - 2)));
        break;

    case BOUNDARY_LINE_BEGIN:
        bsucc = (npos <= 0) || (chL == RCHART('\n')) || ((chL == RCHART('\r')) && (chR != RCHART('\n')));
        break;

    case BOUNDARY_LINE_END:
        bsucc = (npos >= tlen) || (chR == RCHART('\r')) || ((chR == RCHART('\n')) && (chL != RCHART('\r')));
        break;

    case BOUNDARY_WORD_BEGIN:
        bsucc = !IsWordChar(chL) && IsWordChar(chR);
        break;

    case BOUNDARY_WORD_END:
        bsucc = IsWordChar(chL) && !IsWordChar(chR);
        break;

    case BOUNDARY_WORD_EDGE:
        bsucc = IsWordChar(chL) ? !IsWordChar(chR) : IsWordChar(chR);
        break;
    }

    return m_byes ? bsucc : !bsucc;
}

template <class CHART> int CBoundaryElxT <CHART> ::MatchNext(CContext *) const
{
    return 0;
}

template <class CHART> inline int CBoundaryElxT <CHART> ::IsWordChar(CHART ch)
{
    return (ch >= RCHART('A') && ch <= RCHART('Z')) || (ch >= RCHART('a') && ch <= RCHART('z')) || (ch >= RCHART('0') && ch <= RCHART('9')) || (ch == RCHART('_'));
}

//
// Bracket
//
template <class CHART> class CBracketElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CBracketElxT(index_t nnumber, int bright);
    static int CheckCaptureIndex(index_t & index, CContext * pContext, index_t number);

public:
    index_t m_nnumber;
    index_t m_balancing;
    int m_bright;

    CBufferT <CHART> m_szNamed;
    CBufferT <CHART> m_szBalancing;
};

template <class CHART> CBracketElxT <CHART> ::CBracketElxT(index_t nnumber, int bright)
{
    m_nnumber = nnumber;
    m_bright = bright;
    m_balancing = -1;
}

template <class CHART> inline int CBracketElxT <CHART> ::CheckCaptureIndex(index_t & index, CContext * pContext, index_t number)
{
    if (index >= pContext->m_capturestack.GetSize())
        index = pContext->m_capturestack.GetSize() - 4;

    while (index >= 0)
    {
        if (pContext->m_capturestack[index] == number)
        {
            return 1;
        }

        index -= 4;
    }
    return 0;
}

//
// capturestack[index+0] => Group number
// capturestack[index+1] => Capture start pos
// capturestack[index+2] => Capture end pos
// capturestack[index+3] => Capture enclose z-index, zindex<0 means inner group with same name
//
template <class CHART> int CBracketElxT <CHART> ::Match(CContext * pContext) const
{
    // check, for named
    if (m_nnumber < 0) return 0;

    if (!m_bright)
    {
        pContext->m_captureindex.Prepare(m_nnumber, -1);
        index_t index = pContext->m_captureindex[m_nnumber];

        // check
        if (CheckCaptureIndex(index, pContext, m_nnumber) && pContext->m_capturestack[index + 2] < 0)
        {
            pContext->m_capturestack[index + 3] --;
            return 1;
        }

        // balancing left
        if (m_balancing >= 0)
        {
            index_t balancing_index = pContext->m_captureindex[m_balancing];
            if (!CheckCaptureIndex(balancing_index, pContext, m_balancing) ||
                pContext->m_capturestack[balancing_index + 2] < 0)
            {
                return 0;
            }
        }

        // save
        pContext->m_captureindex[m_nnumber] = pContext->m_capturestack.GetSize();

        pContext->m_capturestack.Push(m_nnumber);
        pContext->m_capturestack.Push(pContext->m_nCurrentPos);
        pContext->m_capturestack.Push(-1);
        pContext->m_capturestack.Push(0); // z-index
    }
    else
    {
        // check
        index_t index = pContext->m_captureindex[m_nnumber];

        if (CheckCaptureIndex(index, pContext, m_nnumber))
        {
            if (pContext->m_capturestack[index + 3] < 0) // check inner group with same name
            {
                pContext->m_capturestack[index + 3] ++;
                return 1;
            }

            // balancing right
            index_t balancing_index = -1;
            if (m_balancing >= 0)
            {
                balancing_index = pContext->m_captureindex[m_balancing];
                if (!CheckCaptureIndex(balancing_index, pContext, m_balancing))
                {
                    // TODO ERROR
                    return 0;
                }
            }

            // save
            pContext->m_capturestack[index + 2] = pContext->m_nCurrentPos;
            pContext->m_capturestack[index + 3] = pContext->m_nParenZindex++;

            // balancing right
            if (m_balancing >= 0)
            {
                // backup index
                pContext->m_stack.Push(balancing_index);

                if (balancing_index >= 0)
                {
                    pContext->m_capturestack[index + 2] = pContext->m_capturestack[index + 1];
                    pContext->m_capturestack[index + 1] = pContext->m_capturestack[balancing_index + 2];

                    // destopy capture
                    pContext->m_capturestack[balancing_index] = -1;
                    balancing_index -= 4;
                    CheckCaptureIndex(balancing_index, pContext, m_balancing);
                    pContext->m_captureindex[m_balancing] = balancing_index;
                }
            }
        }
    }

    return 1;
}

template <class CHART> int CBracketElxT <CHART> ::MatchNext(CContext * pContext) const
{
    index_t index = pContext->m_captureindex[m_nnumber];
    if (!CheckCaptureIndex(index, pContext, m_nnumber))
    {
        return 0;
    }

    if (!m_bright)
    {
        if (pContext->m_capturestack[index + 3] < 0)
        {
            pContext->m_capturestack[index + 3] ++;
            return 0;
        }

        pContext->m_capturestack.Restore(pContext->m_capturestack.GetSize() - 4);

        // to find
        CheckCaptureIndex(index, pContext, m_nnumber);

        // new index
        pContext->m_captureindex[m_nnumber] = index;
    }
    else
    {
        if (pContext->m_capturestack[index + 2] >= 0)
        {
            // balancing right
            if (m_balancing >= 0)
            {
                index_t balancing_index = -1;
                pContext->m_stack.Pop(balancing_index);

                if (balancing_index >= 0)
                {
                    pContext->m_capturestack[balancing_index] = m_balancing;
                    pContext->m_captureindex[m_balancing] = balancing_index;
                }
            }
            pContext->m_capturestack[index + 2] = -1;
            pContext->m_capturestack[index + 3] = 0;
        }
        else
        {
            pContext->m_capturestack[index + 3] --;
        }
    }

    return 0;
}

//
// Deletage
//
template <class CHART> class CDelegateElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    explicit CDelegateElxT(int ndata = 0);

public:
    ElxInterface * m_pelx;
    index_t m_ndata; // +0 : recursive to
    // -3 : named recursive

    CBufferT <CHART> m_szNamed;
};

template <class CHART> CDelegateElxT <CHART> ::CDelegateElxT(int ndata)
{
    m_pelx = 0;
    m_ndata = ndata;
}

template <class CHART> int CDelegateElxT <CHART> ::Match(CContext * pContext) const
{
    if (m_pelx != 0)
    {
        if (pContext->m_nCursiveLimit > 0)
        {
            pContext->m_nCursiveLimit--;
            int result = m_pelx->Match(pContext);
            pContext->m_nCursiveLimit++;
            return result;
        }
        else
            return 0;
    }
    else
        return 1;
}

template <class CHART> int CDelegateElxT <CHART> ::MatchNext(CContext * pContext) const
{
    if (m_pelx != 0)
        return m_pelx->MatchNext(pContext);
    else
        return 0;
}

//
// Empty
//
template <index_t x> class CEmptyElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CEmptyElxT();
};

using CEmptyElx = CEmptyElxT <0>;

//
// Global
//
template <index_t x> class CGlobalElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CGlobalElxT();
};

using CGlobalElx = CGlobalElxT <0>;

//
// Repeat
//
template <int x> class CRepeatElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CRepeatElxT(ElxInterface * pelx, int ntimes);

protected:
    int MatchFixed(CContext * pContext) const;
    int MatchNextFixed(CContext * pContext) const;
    int MatchForward(CContext * pContext) const
    {
        CContextShot shot(pContext);

        if (!m_pelx->Match(pContext))
            return 0;

        if (pContext->m_nCurrentPos != shot.m_nCurrentPos)
            return 1;

        if (!m_pelx->MatchNext(pContext))
            return 0;

        if (pContext->m_nCurrentPos != shot.m_nCurrentPos)
            return 1;

        shot.Restore(pContext);
        return 0;
    }

public:
    ElxInterface * m_pelx;
    int m_nfixed;
};

using CRepeatElx = CRepeatElxT <0>;

//
// Greedy
//
template <index_t x> class CGreedyElxT : public CRepeatElxT <x>
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CGreedyElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
    int MatchVart(CContext * pContext) const;
    int MatchNextVart(CContext * pContext) const;

public:
    int m_nvart;
};

using CGreedyElx = CGreedyElxT <0>;

//
// Independent
//
template <index_t x> class CIndependentElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    explicit CIndependentElxT(ElxInterface * pelx);

public:
    ElxInterface * m_pelx;
};

using CIndependentElx = CIndependentElxT <0>;

//
// List
//
template <index_t x> class CListElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    explicit CListElxT(int brightleft);

public:
    CBufferT <ElxInterface *> m_elxlist;
    int m_brightleft;
};

using CListElx = CListElxT <0>;

//
// Posix Elx
//
template <class CHART> class CPosixElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CPosixElxT(const char * posix, int brightleft);

public:
    POSIX_FUNC m_posixfun;
    int m_brightleft;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CPosixElxT <CHART> ::CPosixElxT(const char * posix, int brightleft)
{
    m_brightleft = brightleft;

    if (posix[1] == '^')
    {
        m_byes = 0;
        posix += 2;
    }
    else
    {
        m_byes = 1;
        posix += 1;
    }

    if (!strncmp(posix, "alnum:", 6)) m_posixfun = ::isalnum;
    else if (!strncmp(posix, "alpha:", 6)) m_posixfun = ::isalpha;
    else if (!strncmp(posix, "ascii:", 6)) m_posixfun = ::isascii;
    else if (!strncmp(posix, "cntrl:", 6)) m_posixfun = ::iscntrl;
    else if (!strncmp(posix, "digit:", 6)) m_posixfun = ::isdigit;
    else if (!strncmp(posix, "graph:", 6)) m_posixfun = ::isgraph;
    else if (!strncmp(posix, "lower:", 6)) m_posixfun = ::islower;
    else if (!strncmp(posix, "print:", 6)) m_posixfun = ::isprint;
    else if (!strncmp(posix, "punct:", 6)) m_posixfun = ::ispunct;
    else if (!strncmp(posix, "space:", 6)) m_posixfun = ::isspace;
    else if (!strncmp(posix, "upper:", 6)) m_posixfun = ::isupper;
    else if (!strncmp(posix, "xdigit:", 7)) m_posixfun = ::isxdigit;
    else if (!strncmp(posix, "blank:", 6)) m_posixfun = isblank;
    else                                    m_posixfun = 0;
}

inline int isblank(int c)
{
    return c == 0x20 || c == '\t';
}

template <class CHART> int CPosixElxT <CHART> ::Match(CContext * pContext) const
{
    if (m_posixfun == 0) return 0;

    index_t tlen = pContext->m_pMatchStringLength;
    index_t npos = pContext->m_nCurrentPos;

    // check
    index_t at = m_brightleft ? npos - 1 : npos;
    if (at < 0 || at >= tlen)
        return 0;

    CHART ch = ((const CHART *)pContext->m_pMatchString)[at];

    int bsucc = (*m_posixfun)(ch);

    if (!m_byes)
        bsucc = !bsucc;

    if (bsucc)
        pContext->m_nCurrentPos += m_brightleft ? -1 : 1;

    return bsucc;
}

template <class CHART> int CPosixElxT <CHART> ::MatchNext(CContext * pContext) const
{
    pContext->m_nCurrentPos -= m_brightleft ? -1 : 1;
    return 0;
}

//
// Possessive
//
template <int x> class CPossessiveElxT : public CGreedyElxT <x>
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CPossessiveElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);
};

using CPossessiveElx = CPossessiveElxT <0>;

//
// Range Elx
//
template <class CHART> class CRangeElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CRangeElxT(int brightleft, int byes);

public:
    int IsContainChar(CHART ch) const;

public:
    CBufferT <CHART> m_ranges;
    CBufferT <CHART> m_chars;
    CBufferT <ElxInterface *> m_embeds;

public:
    int m_brightleft;
    int m_byes;
};

//
// Implementation
//
template <class CHART> CRangeElxT <CHART> ::CRangeElxT(int brightleft, int byes)
{
    m_brightleft = brightleft;
    m_byes = byes;
}

template <class CHART> int CRangeElxT <CHART> ::Match(CContext * pContext) const
{
    index_t tlen = pContext->m_pMatchStringLength;
    index_t npos = pContext->m_nCurrentPos;

    // check
    index_t at = m_brightleft ? npos - 1 : npos;
    if (at < 0 || at >= tlen)
        return 0;

    CHART ch = ((const CHART *)pContext->m_pMatchString)[at];
    int bsucc = 0, i;

    // compare
    for (i = 0; !bsucc && i < m_ranges.GetSize(); i += 2)
    {
        if (m_ranges[i] <= ch && ch <= m_ranges[i + 1]) bsucc = 1;
    }

    for (i = 0; !bsucc && i < m_chars.GetSize(); i++)
    {
        if (m_chars[i] == ch) bsucc = 1;
    }

    for (i = 0; !bsucc && i < m_embeds.GetSize(); i++)
    {
        if (m_embeds[i]->Match(pContext))
        {
            pContext->m_nCurrentPos = npos;
            bsucc = 1;
        }
    }

    if (!m_byes)
        bsucc = !bsucc;

    if (bsucc)
        pContext->m_nCurrentPos += m_brightleft ? -1 : 1;

    return bsucc;
}

template <class CHART> int CRangeElxT <CHART> ::IsContainChar(CHART ch) const
{
    int bsucc = 0, i;

    // compare
    for (i = 0; !bsucc && i < m_ranges.GetSize(); i += 2)
    {
        if (m_ranges[i] <= ch && ch <= m_ranges[i + 1]) bsucc = 1;
    }

    for (i = 0; !bsucc && i < m_chars.GetSize(); i++)
    {
        if (m_chars[i] == ch) bsucc = 1;
    }

    return bsucc;
}

template <class CHART> int CRangeElxT <CHART> ::MatchNext(CContext * pContext) const
{
    pContext->m_nCurrentPos -= m_brightleft ? -1 : 1;
    return 0;
}

//
// Reluctant
//
template <int x> class CReluctantElxT : public CRepeatElxT <x>
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CReluctantElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
    static int MatchVart(CContext * pContext);
    int MatchNextVart(CContext * pContext) const;

public:
    int m_nvart;
};

using CReluctantElx = CReluctantElxT <0>;

//
// String Elx
//
template <class CHART> class CStringElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CStringElxT(const CHART * fixed, index_t nlength, int brightleft, int bignorecase);

public:
    CBufferT <CHART> m_szPattern;
    int m_brightleft;
    int m_bignorecase;
};

//
// Implementation
//
template <class CHART> CStringElxT <CHART> ::CStringElxT(const CHART * fixed, index_t nlength, int brightleft, int bignorecase) : m_szPattern(fixed, nlength)
{
    m_brightleft = brightleft;
    m_bignorecase = bignorecase;
}

template <class CHART> int CStringElxT <CHART> ::Match(CContext * pContext) const
{
    const CHART * pcsz = (const CHART *)pContext->m_pMatchString;
    index_t npos = pContext->m_nCurrentPos;
    index_t tlen = pContext->m_pMatchStringLength;
    index_t slen = m_szPattern.GetSize();

    int bsucc;

    if (m_brightleft)
    {
        if (npos < slen)
            return 0;

        if (m_bignorecase)
            bsucc = !m_szPattern.nCompareNoCase(pcsz + (npos - slen));
        else
            bsucc = !m_szPattern.nCompare(pcsz + (npos - slen));

        if (bsucc)
            pContext->m_nCurrentPos -= slen;
    }
    else
    {
        if (npos + slen > tlen)
            return 0;

        if (m_bignorecase)
            bsucc = !m_szPattern.nCompareNoCase(pcsz + npos);
        else
            bsucc = !m_szPattern.nCompare(pcsz + npos);

        if (bsucc)
            pContext->m_nCurrentPos += slen;
    }

    return bsucc;
}

template <class CHART> int CStringElxT <CHART> ::MatchNext(CContext * pContext) const
{
    index_t slen = m_szPattern.GetSize();

    if (m_brightleft)
        pContext->m_nCurrentPos += slen;
    else
        pContext->m_nCurrentPos -= slen;

    return 0;
}

//
// CConditionElx
//
template <class CHART> class CConditionElxT : public ElxInterface
{
public:
    int Match(CContext * pContext) const override;
    int MatchNext(CContext * pContext) const override;

public:
    CConditionElxT();

public:
    // backref condition
    index_t m_nnumber;
    CBufferT <CHART> m_szNamed;

    // elx condition
    ElxInterface * m_pelxask;

    // selection
    ElxInterface * m_pelxyes, *m_pelxno;
};

template <class CHART> CConditionElxT <CHART> ::CConditionElxT()
    : m_nnumber(-1)
    , m_szNamed()
    , m_pelxask(nullptr)
    , m_pelxyes(nullptr)
    , m_pelxno(nullptr)
{
}

template <class CHART> int CConditionElxT <CHART> ::Match(CContext * pContext) const
{
    // status
    index_t nbegin = pContext->m_nCurrentPos;
    index_t nsize = pContext->m_stack.GetSize();
    index_t ncsize = pContext->m_capturestack.GetSize();

    // condition result
    int condition_yes = 0;

    // backref type
    if (m_nnumber >= 0)
    {
        do
        {
            if (m_nnumber >= pContext->m_captureindex.GetSize()) break;

            index_t index = pContext->m_captureindex[m_nnumber];
            if (index < 0) break;

            // else valid
            condition_yes = 1;
        } while (false);
    }
    else
    {
        if (m_pelxask == 0)
            condition_yes = 1;
        else
            condition_yes = m_pelxask->Match(pContext);

        pContext->m_stack.Restore(nsize);
        pContext->m_nCurrentPos = nbegin;
    }

    // elx result
    int bsucc;
    if (condition_yes)
        bsucc = m_pelxyes == 0 ? 1 : m_pelxyes->Match(pContext);
    else
        bsucc = m_pelxno == 0 ? 1 : m_pelxno->Match(pContext);

    if (bsucc)
    {
        pContext->m_stack.Push(ncsize);
        pContext->m_stack.Push(condition_yes);
    }
    else
    {
        pContext->m_capturestack.Restore(ncsize);
    }

    return bsucc;
}

template <class CHART> int CConditionElxT <CHART> ::MatchNext(CContext * pContext) const
{
    // pop
    index_t ncsize, condition_yes;

    pContext->m_stack.Pop(condition_yes);
    pContext->m_stack.Pop(ncsize);

    // elx result
    int bsucc;
    if (condition_yes)
        bsucc = m_pelxyes == 0 ? 0 : m_pelxyes->MatchNext(pContext);
    else
        bsucc = m_pelxno == 0 ? 0 : m_pelxno->MatchNext(pContext);

    if (bsucc)
    {
        pContext->m_stack.Push(ncsize);
        pContext->m_stack.Push(condition_yes);
    }
    else
    {
        pContext->m_capturestack.Restore(ncsize);
    }

    return bsucc;
}

//
// MatchResult
//
template <index_t x> class MatchResultT
{
public:
    int IsMatched() const;

public:
    index_t GetStart() const;
    index_t GetEnd() const;

public:
    index_t MaxGroupNumber() const;
    index_t GetGroupStart(index_t nGroupNumber) const;
    index_t GetGroupEnd(index_t nGroupNumber) const;

public:
    MatchResultT(const MatchResultT <x> & from) { *this = from; }
    MatchResultT(CContext * pContext = nullptr, index_t nMaxNumber = -1);
    MatchResultT <x> & operator = (const MatchResultT <x> &);
    inline operator int() const { return IsMatched(); }

public:
    CBufferT <index_t> m_result;
};

using MatchResult = MatchResultT <0>;

// Stocked Elx IDs
enum STOCKELX_ID_DEFINES
{
    STOCKELX_EMPTY = 0,

    ///////////////////////

    STOCKELX_DOT_ALL,
    STOCKELX_DOT_NOT_ALL,

    STOCKELX_WORD,
    STOCKELX_WORD_NOT,

    STOCKELX_SPACE,
    STOCKELX_SPACE_NOT,

    STOCKELX_DIGITAL,
    STOCKELX_DIGITAL_NOT,

    //////////////////////

    STOCKELX_DOT_ALL_RIGHTLEFT,
    STOCKELX_DOT_NOT_ALL_RIGHTLEFT,

    STOCKELX_WORD_RIGHTLEFT,
    STOCKELX_WORD_RIGHTLEFT_NOT,

    STOCKELX_SPACE_RIGHTLEFT,
    STOCKELX_SPACE_RIGHTLEFT_NOT,

    STOCKELX_DIGITAL_RIGHTLEFT,
    STOCKELX_DIGITAL_RIGHTLEFT_NOT,

    /////////////////////

    STOCKELX_COUNT
};

// REGEX_FLAGS
#ifndef _REGEX_FLAGS_DEFINED
enum REGEX_FLAGS
{
    NO_FLAG = 0,
    SINGLELINE = 0x01,
    MULTILINE = 0x02,
    GLOBAL = 0x04,
    IGNORECASE = 0x08,
    RIGHTTOLEFT = 0x10,
    EXTENDED = 0x20
};
#define _REGEX_FLAGS_DEFINED
#endif

//
// Builder T
//
template <class CHART> class CBuilderT
{
public:
    using CDelegateElx = CDelegateElxT  <CHART>;
    using CBracketElx = CBracketElxT   <CHART>;
    using CBackrefElx = CBackrefElxT   <CHART>;
    using CConditionElx = CConditionElxT <CHART>;

    // Methods
public:
    ElxInterface * Build(const CBufferRefT <CHART> & pattern, int flags);
    index_t GetNamedNumber(const CBufferRefT <CHART> & named) const;
    void Clear();

public:
    CBuilderT();
    virtual ~CBuilderT();

    // Public Attributes
public:
    ElxInterface * m_pTopElx;
    int            m_nFlags;
    index_t        m_nMaxNumber;
    index_t        m_nNextNamed;
    index_t        m_nGroupCount;
    int            m_nNextBalancing;

    CBufferT <ElxInterface  *> m_objlist;
    CBufferT <ElxInterface  *> m_grouplist;
    CBufferT <CDelegateElx  *> m_recursivelist;
    CBufferT <CListElx      *> m_namedlist;
    CBufferT <CBackrefElx   *> m_namedbackreflist;
    CBufferT <CConditionElx *> m_namedconditionlist;
    CBufferT <CListElx      *> m_purebalancinglist;

    // CHART_INFO
protected:
    struct CHART_INFO
    {
    public:
        CHART ch;
        int   type;
        int   pos;
        int   len;

    public:
        CHART_INFO(CHART c, int t, int p = 0, int l = 0) { ch = c; type = t; pos = p; len = l; }
        inline int operator == (const CHART_INFO & ci) const { return ch == ci.ch && type == ci.type; }
        inline int operator != (const CHART_INFO & ci) const { return !operator == (ci); }
    };

protected:
    static unsigned int Hex2Int(const CHART * pcsz, int length, int & used);
    static int ReadDec(char * & str, unsigned int & dec);
    void MoveNext();
    int  GetNext2();

    ElxInterface * BuildAlternative(int vaflags);
    ElxInterface * BuildList(int & flags);
    ElxInterface * BuildRepeat(int & flags);
    ElxInterface * BuildSimple(int & flags);
    ElxInterface * BuildCharset(int & flags);
    ElxInterface * BuildRecursive(int & flags);
    ElxInterface * BuildBoundary(int & flags);
    ElxInterface * BuildBackref(int & flags);

    ElxInterface * GetStockElx(int nStockId);
    ElxInterface * Keep(ElxInterface * pElx);

    // Private Attributes
protected:
    CBufferRefT <CHART> m_pattern;
    CHART_INFO prev, curr, next, nex2;
    int m_nNextPos;
    int m_nCharsetDepth;
    int m_bQuoted;
    POSIX_FUNC m_quote_fun;

    // Backup current pos
    struct Snapshot
    {
        CHART_INFO prev, curr, next, nex2;
        int m_nNextPos;
        int m_nCharsetDepth;
        int m_bQuoted;
        POSIX_FUNC m_quote_fun;
        Snapshot() : prev(0, 0), curr(0, 0), next(0, 0), nex2(0, 0)
            , m_nNextPos(0), m_nCharsetDepth(0), m_bQuoted(0), m_quote_fun()
        {}
    };
    void Backup(Snapshot * pdata) { memcpy(pdata, &prev, sizeof(Snapshot)); }
    void Restore(Snapshot * pdata) { memcpy(&prev, pdata, sizeof(Snapshot)); }

    ElxInterface * m_pStockElxs[STOCKELX_COUNT];
};

//
// Implementation
//
template <class CHART> CBuilderT <CHART> ::CBuilderT() 
    : m_nFlags(0)
    , m_nNextBalancing(0)
// protected
    , m_pattern(0, 0)
    , prev(0, 0)
    , curr(0, 0)
    , next(0, 0)
    , nex2(0, 0)
    , m_nNextPos(0)
    , m_nCharsetDepth(0)
    , m_bQuoted(0)
    , m_quote_fun()
{
    Clear();
}

template <class CHART> CBuilderT <CHART> :: ~CBuilderT()
{
    Clear();
}

template <class CHART> index_t CBuilderT <CHART> ::GetNamedNumber(const CBufferRefT <CHART> & named) const
{
    for (int i = 0; i < m_namedlist.GetSize(); i++)
    {
        if (!((CBracketElx *)m_namedlist[i]->m_elxlist[0])->m_szNamed.CompareNoCase(named))
            return ((CBracketElx *)m_namedlist[i]->m_elxlist[0])->m_nnumber;
    }

    return -3;
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::Build(const CBufferRefT <CHART> & pattern, int flags)
{
    // init
    m_pattern = pattern;
    m_nNextPos = 0;
    m_nCharsetDepth = 0;
    m_nMaxNumber = 0;
    m_nNextNamed = 0;
    m_nNextBalancing = 0;
    m_nFlags = flags;
    m_bQuoted = 0;
    m_quote_fun = 0;

    m_grouplist.Restore(0);
    m_recursivelist.Restore(0);
    m_namedlist.Restore(0);
    m_namedbackreflist.Restore(0);
    m_namedconditionlist.Restore(0);
    m_purebalancinglist.Restore(0);

    int i;
    for (i = 0; i < 3; i++) MoveNext();

    // build
    m_pTopElx = BuildAlternative(flags);

    // group 0
    m_grouplist.Prepare(0);
    m_grouplist[0] = m_pTopElx;

    // append named to unnamed
    m_nGroupCount = m_grouplist.GetSize();

    m_grouplist.Prepare(m_nMaxNumber + m_namedlist.GetSize());

    for (i = 0; i < m_namedlist.GetSize(); i++)
    {
        CBracketElx * pleft = (CBracketElx *)m_namedlist[i]->m_elxlist[0];
        CBracketElx * pright = (CBracketElx *)m_namedlist[i]->m_elxlist[2];

        // append
        m_grouplist[m_nGroupCount++] = m_namedlist[i];

        if (pleft->m_nnumber > 0)
            continue;

        // same name
        index_t find_same_name = GetNamedNumber(pleft->m_szNamed);
        if (find_same_name >= 0)
        {
            pleft->m_nnumber = find_same_name;
            pright->m_nnumber = find_same_name;
        }
        else
        {
            m_nMaxNumber++;

            pleft->m_nnumber = m_nMaxNumber;
            pright->m_nnumber = m_nMaxNumber;
        }
    }

    for (i = 0; i < m_namedlist.GetSize(); i++)
    {
        CBracketElx * pleft = (CBracketElx *)m_namedlist[i]->m_elxlist[0];
        CBracketElx * pright = (CBracketElx *)m_namedlist[i]->m_elxlist[2];

        // balancing
        if (pleft->m_szBalancing.GetSize() > 0)
        {
            index_t balancing_to = GetNamedNumber(pleft->m_szBalancing);
            if (balancing_to >= 0)
            {
                pleft->m_balancing = balancing_to;
                pright->m_balancing = balancing_to;
            }
            else
            {
                //TODO: ERROR
                throw regexExcp;
            }
        }
    }

    for (i = 1; i < m_nGroupCount; i++)
    {
        CBracketElx * pleft = (CBracketElx *)((CListElx*)m_grouplist[i])->m_elxlist[0];

        if (pleft->m_nnumber > m_nMaxNumber)
            m_nMaxNumber = pleft->m_nnumber;
    }

    // pure balancing group
    index_t nMaxNumber = m_nMaxNumber;
    for (i = 0; i < m_purebalancinglist.GetSize(); i++)
    {
        CBracketElx * pleft = (CBracketElx *)m_purebalancinglist[i]->m_elxlist[0];
        CBracketElx * pright = (CBracketElx *)m_purebalancinglist[i]->m_elxlist[2];

        nMaxNumber++;

        pleft->m_nnumber = nMaxNumber;
        pright->m_nnumber = nMaxNumber;

        // balancing
        if (pleft->m_szBalancing.GetSize() > 0)
        {
            index_t balancing_to = GetNamedNumber(pleft->m_szBalancing);
            if (balancing_to >= 0)
            {
                pleft->m_balancing = balancing_to;
                pright->m_balancing = balancing_to;
            }
            else
            {
                //TODO: ERROR
                throw regexExcp;
            }
        }
    }

    // connect recursive
    for (i = 0; i < m_recursivelist.GetSize(); i++)
    {
        if (m_recursivelist[i]->m_ndata == -3)
            m_recursivelist[i]->m_ndata = GetNamedNumber(m_recursivelist[i]->m_szNamed);

        if (m_recursivelist[i]->m_ndata >= 0 && m_recursivelist[i]->m_ndata <= m_nMaxNumber)
        {
            if (m_recursivelist[i]->m_ndata == 0)
                m_recursivelist[i]->m_pelx = m_pTopElx;
            else for (int j = 1; j < m_grouplist.GetSize(); j++)
            {
                if (m_recursivelist[i]->m_ndata == ((CBracketElx *)((CListElx*)m_grouplist[j])->m_elxlist[0])->m_nnumber)
                {
                    m_recursivelist[i]->m_pelx = m_grouplist[j];
                    break;
                }
            }
        }
    }

    // named backref
    for (i = 0; i < m_namedbackreflist.GetSize(); i++)
    {
        m_namedbackreflist[i]->m_nnumber = GetNamedNumber(m_namedbackreflist[i]->m_szNamed);
    }

    // named condition
    for (i = 0; i < m_namedconditionlist.GetSize(); i++)
    {
        index_t nn = GetNamedNumber(m_namedconditionlist[i]->m_szNamed);
        if (nn >= 0)
        {
            m_namedconditionlist[i]->m_nnumber = nn;
            m_namedconditionlist[i]->m_pelxask = 0;
        }
    }

    return m_pTopElx;
}

template <class CHART> void CBuilderT <CHART> ::Clear()
{
    for (int i = 0; i < m_objlist.GetSize(); i++)
    {
        delete m_objlist[i];
    }

    m_objlist.Restore(0);
    m_pTopElx = 0;
    m_nMaxNumber = 0;

    memset(m_pStockElxs, 0, sizeof(m_pStockElxs));
}

//
// hex to int
//
template <class CHART> unsigned int CBuilderT <CHART> ::Hex2Int(const CHART * pcsz, int length, int & used)
{
    unsigned int result = 0;
    int & i = used;

    for (i = 0; i < length; i++)
    {
        if (pcsz[i] >= RCHART('0') && pcsz[i] <= RCHART('9'))
            result = (result << 4) + (pcsz[i] - RCHART('0'));
        else if (pcsz[i] >= RCHART('A') && pcsz[i] <= RCHART('F'))
            result = (result << 4) + (0x0A + (pcsz[i] - RCHART('A')));
        else if (pcsz[i] >= RCHART('a') && pcsz[i] <= RCHART('f'))
            result = (result << 4) + (0x0A + (pcsz[i] - RCHART('a')));
        else
            break;
    }

    return result;
}

template <class CHART> inline ElxInterface * CBuilderT <CHART> ::Keep(ElxInterface * pelx)
{
    m_objlist.Push(pelx);
    return pelx;
}

template <class CHART> void CBuilderT <CHART> ::MoveNext()
{
    // forwards
    prev = curr;
    curr = next;
    next = nex2;

    // get nex2
    while (!GetNext2()) {};
}

template <class CHART> int CBuilderT <CHART> ::GetNext2()
{
    // check length
    if (m_nNextPos >= m_pattern.GetSize())
    {
        nex2 = CHART_INFO(0, 1, m_nNextPos, 0);
        return 1;
    }

    int   delta = 1;
    CHART ch = m_pattern[m_nNextPos];

    // if quoted
    if (m_bQuoted)
    {
        if (ch == RCHART('\\'))
        {
            if (m_pattern[m_nNextPos + 1] == RCHART('E'))
            {
                m_quote_fun = 0;
                m_bQuoted = 0;
                m_nNextPos += 2;
                return 0;
            }
        }

        if (m_quote_fun != 0)
            nex2 = CHART_INFO((CHART)(*m_quote_fun)((int)ch), 0, m_nNextPos, delta);
        else
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);

        m_nNextPos += delta;

        return 1;
    }

    // common
    switch (ch)
    {
    case RCHART('\\'):
        {
            CHART ch1 = m_pattern[m_nNextPos + 1];

            // backref
            if (ch1 >= RCHART('0') && ch1 <= RCHART('9'))
            {
                nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
                break;
            }

            // escape
            delta = 2;

            switch (ch1)
            {
            case RCHART('A'):
            case RCHART('Z'):
            case RCHART('z'):
            case RCHART('w'):
            case RCHART('W'):
            case RCHART('s'):
            case RCHART('S'):
            case RCHART('B'):
            case RCHART('d'):
            case RCHART('D'):
            case RCHART('k'):
            case RCHART('g'):
                nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
                break;

            case RCHART('b'):
                if (m_nCharsetDepth > 0)
                    nex2 = CHART_INFO('\b', 0, m_nNextPos, delta);
                else
                    nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
                break;

                /*
                case RCHART('<'):
                case RCHART('>'):
                if(m_nCharsetDepth > 0)
                nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
                else
                nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
                break;
                */

            case RCHART('x'):
                if (m_pattern[m_nNextPos + 2] != '{')
                {
                    int red = 0;
                    unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 2, red);

                    delta += red;

                    if (red > 0)
                        nex2 = CHART_INFO(RCHART(ch2), 0, m_nNextPos, delta);
                    else
                        nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);

                    break;
                }

            case RCHART('u'):
                if (m_pattern[m_nNextPos + 2] != '{')
                {
                    int red = 0;
                    unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 4, red);

                    delta += red;

                    if (red > 0)
                        nex2 = CHART_INFO(RCHART(ch2), 0, m_nNextPos, delta);
                    else
                        nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
                }
                else
                {
                    int red = 0;
                    unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 3, sizeof(int) * 2, red);

                    delta += red;

                    while (m_nNextPos + delta < m_pattern.GetSize() && m_pattern.At(m_nNextPos + delta) != RCHART('}'))
                        delta++;

                    delta++; // skip '}'

                    nex2 = CHART_INFO(RCHART(ch2), 0, m_nNextPos, delta);
                }
                break;

            case RCHART('a'): nex2 = CHART_INFO(RCHART('\a'), 0, m_nNextPos, delta); break;
            case RCHART('f'): nex2 = CHART_INFO(RCHART('\f'), 0, m_nNextPos, delta); break;
            case RCHART('n'): nex2 = CHART_INFO(RCHART('\n'), 0, m_nNextPos, delta); break;
            case RCHART('r'): nex2 = CHART_INFO(RCHART('\r'), 0, m_nNextPos, delta); break;
            case RCHART('t'): nex2 = CHART_INFO(RCHART('\t'), 0, m_nNextPos, delta); break;
            case RCHART('v'): nex2 = CHART_INFO(RCHART('\v'), 0, m_nNextPos, delta); break;
            case RCHART('e'): nex2 = CHART_INFO(RCHART(27), 0, m_nNextPos, delta); break;

            case RCHART('G'):  // skip '\G'
                if (m_nCharsetDepth > 0)
                {
                    m_nNextPos += 2;
                    return 0;
                }
                else
                {
                    nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
                    break;
                }

            case RCHART('L'):
                if (!m_quote_fun) m_quote_fun = ::tolower;

            case RCHART('U'):
                if (!m_quote_fun) m_quote_fun = ::toupper;

            case RCHART('Q'):
                {
                    m_bQuoted = 1;
                    m_nNextPos += 2;
                    return 0;
                }

            case RCHART('E'):
                {
                    m_quote_fun = 0;
                    m_bQuoted = 0;
                    m_nNextPos += 2;
                    return 0;
                }

            case 0:
                if (m_nNextPos + 1 >= m_pattern.GetSize())
                {
                    delta = 1;
                    nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
                }
                else
                    nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta); // common '\0' char
                break;

            default:
                nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
                break;
            }
        }
        break;

    case RCHART('*'):
    case RCHART('+'):
    case RCHART('?'):
    case RCHART('.'):
    case RCHART('{'):
    case RCHART('}'):
    case RCHART(')'):
    case RCHART('|'):
    case RCHART('$'):
        if (m_nCharsetDepth > 0)
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        else
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        break;

    case RCHART('-'):
        if (m_nCharsetDepth > 0)
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        else
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        break;

    case RCHART('('):
        {
            CHART ch1 = m_pattern[m_nNextPos + 1];
            CHART ch2 = m_pattern[m_nNextPos + 2];

            // skip remark
            if (ch1 == RCHART('?') && ch2 == RCHART('#'))
            {
                m_nNextPos += 2;
                while (m_nNextPos < m_pattern.GetSize())
                {
                    if (m_pattern[m_nNextPos] == RCHART(')'))
                        break;

                    m_nNextPos++;
                }

                if (m_pattern[m_nNextPos] == RCHART(')'))
                {
                    m_nNextPos++;

                    // get next nex2
                    return 0;
                }
            }
            else
            {
                if (m_nCharsetDepth > 0)
                    nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
                else
                    nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
            }
        }
        break;

    case RCHART('#'):
        if (m_nFlags & EXTENDED)
        {
            // skip remark
            m_nNextPos++;

            while (m_nNextPos < m_pattern.GetSize())
            {
                if (m_pattern[m_nNextPos] == RCHART('\n') || m_pattern[m_nNextPos] == RCHART('\r'))
                    break;

                m_nNextPos++;
            }

            // get next nex2
            return 0;
        }
        else
        {
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        }
        break;

    case RCHART(' '):
    case RCHART('\f'):
    case RCHART('\n'):
    case RCHART('\r'):
    case RCHART('\t'):
    case RCHART('\v'):
        if (m_nFlags & EXTENDED)
        {
            m_nNextPos++;

            // get next nex2
            return 0;
        }
        else
        {
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        }
        break;

    case RCHART('['):
        if (m_nCharsetDepth == 0 || m_pattern.At(m_nNextPos + 1, 0) == RCHART(':'))
        {
            m_nCharsetDepth++;
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        }
        else
        {
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        }
        break;

    case RCHART(']'):
        if (m_nCharsetDepth > 0)
        {
            m_nCharsetDepth--;
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        }
        else
        {
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        }
        break;

    case RCHART(':'):
        if (next == CHART_INFO(RCHART('['), 1))
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        else
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        break;

    case RCHART('^'):
        if (m_nCharsetDepth == 0 || next == CHART_INFO(RCHART('['), 1) || (curr == CHART_INFO(RCHART('['), 1) && next == CHART_INFO(RCHART(':'), 1)))
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
        else
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        break;

    case 0:
        if (m_nNextPos >= m_pattern.GetSize())
            nex2 = CHART_INFO(ch, 1, m_nNextPos, delta); // end of string
        else
            nex2 = CHART_INFO(ch, 0, m_nNextPos, delta); // common '\0' char
        break;

    default:
        nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
        break;
    }

    m_nNextPos += delta;

    return 1;
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::GetStockElx(int nStockId)
{
    ElxInterface ** pStockElxs = m_pStockElxs;

    // check
    if (nStockId < 0 || nStockId >= STOCKELX_COUNT)
        return GetStockElx(0);

    // create if no
    if (pStockElxs[nStockId] == nullptr)
    {
        switch (nStockId)
        {
        case STOCKELX_EMPTY:
            pStockElxs[nStockId] = Keep(new CEmptyElx());
            break;

        case STOCKELX_WORD:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 1));

                pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
                pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
                pRange->m_chars.Push(RCHART('_'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_WORD_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 0));

                pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
                pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
                pRange->m_chars.Push(RCHART('_'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DOT_ALL:
            pStockElxs[nStockId] = Keep(new CRangeElxT <CHART>(0, 0));
            break;

        case STOCKELX_DOT_NOT_ALL:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 0));

                pRange->m_chars.Push(RCHART('\n'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_SPACE:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 1));

                pRange->m_chars.Push(RCHART(' '));
                pRange->m_chars.Push(RCHART('\t'));
                pRange->m_chars.Push(RCHART('\r'));
                pRange->m_chars.Push(RCHART('\n'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_SPACE_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 0));

                pRange->m_chars.Push(RCHART(' '));
                pRange->m_chars.Push(RCHART('\t'));
                pRange->m_chars.Push(RCHART('\r'));
                pRange->m_chars.Push(RCHART('\n'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DIGITAL:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 1));

                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DIGITAL_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(0, 0));

                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_WORD_RIGHTLEFT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 1));

                pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
                pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
                pRange->m_chars.Push(RCHART('_'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_WORD_RIGHTLEFT_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 0));

                pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
                pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
                pRange->m_chars.Push(RCHART('_'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DOT_ALL_RIGHTLEFT:
            pStockElxs[nStockId] = Keep(new CRangeElxT <CHART>(1, 0));
            break;

        case STOCKELX_DOT_NOT_ALL_RIGHTLEFT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 0));

                pRange->m_chars.Push(RCHART('\n'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_SPACE_RIGHTLEFT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 1));

                pRange->m_chars.Push(RCHART(' '));
                pRange->m_chars.Push(RCHART('\t'));
                pRange->m_chars.Push(RCHART('\r'));
                pRange->m_chars.Push(RCHART('\n'));
                pRange->m_chars.Push(RCHART('\f'));
                pRange->m_chars.Push(RCHART('\v'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_SPACE_RIGHTLEFT_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 0));

                pRange->m_chars.Push(RCHART(' '));
                pRange->m_chars.Push(RCHART('\t'));
                pRange->m_chars.Push(RCHART('\r'));
                pRange->m_chars.Push(RCHART('\n'));
                pRange->m_chars.Push(RCHART('\f'));
                pRange->m_chars.Push(RCHART('\v'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DIGITAL_RIGHTLEFT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 1));

                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

                pStockElxs[nStockId] = pRange;
            }
            break;

        case STOCKELX_DIGITAL_RIGHTLEFT_NOT:
            {
                CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(1, 0));

                pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

                pStockElxs[nStockId] = pRange;
            }
            break;
        }
    }

    // return
    return pStockElxs[nStockId];
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildAlternative(int vaflags)
{
    if (curr == CHART_INFO(0, 1))
        return GetStockElx(STOCKELX_EMPTY);

    // flag instance
    int flags = vaflags;

    // first part
    ElxInterface * pAlternativeOne = BuildList(flags);

    // check alternative
    if (curr == CHART_INFO(RCHART('|'), 1))
    {
        CAlternativeElx * pAlternative = (CAlternativeElx *)Keep(new CAlternativeElx());
        pAlternative->m_elxlist.Push(pAlternativeOne);

        // loop
        while (curr == CHART_INFO(RCHART('|'), 1))
        {
            // skip '|' itself
            MoveNext();

            pAlternativeOne = BuildList(flags);
            pAlternative->m_elxlist.Push(pAlternativeOne);
        }

        return pAlternative;
    }

    return pAlternativeOne;
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildList(int & flags)
{
    if (curr == CHART_INFO(0, 1) || curr == CHART_INFO(RCHART('|'), 1) || curr == CHART_INFO(RCHART(')'), 1))
        return GetStockElx(STOCKELX_EMPTY);

    // first
    ElxInterface * pListOne = BuildRepeat(flags);

    if (curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('|'), 1) && curr != CHART_INFO(RCHART(')'), 1))
    {
        CListElx * pList = (CListElx *)Keep(new CListElx(flags & RIGHTTOLEFT));
        pList->m_elxlist.Push(pListOne);

        while (curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('|'), 1) && curr != CHART_INFO(RCHART(')'), 1))
        {
            pListOne = BuildRepeat(flags);

            // add
            pList->m_elxlist.Push(pListOne);
        }

        return pList;
    }

    return pListOne;
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildRepeat(int & flags)
{
    // simple
    ElxInterface * pSimple = BuildSimple(flags);

    if (curr.type == 0) return pSimple;

    // is quantifier or not
    int bIsQuantifier = 1;

    // quantifier range
    unsigned int nMin = 0, nMax = 0;

    switch (curr.ch)
    {
    case RCHART('{'):
        {
            CBufferT <char> re;

            // skip '{'
            MoveNext();

            // copy
            while (curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('}'), 1))
            {
                re.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                MoveNext();
            }

            // skip '}'
            MoveNext();

            // read
            int red;
            char * str = re.GetBuffer();

            if (!ReadDec(str, nMin))
                red = 0;
            else if (*str != ',')
                red = 1;
            else
            {
                str++;

                if (!ReadDec(str, nMax))
                    red = 2;
                else
                    red = 3;
            }

            // check
            if (red <= 1) nMax = nMin;
            if (red == 2) nMax = INT_MAX;
            if (nMax < nMin) nMax = nMin;
        }
        break;

    case RCHART('?'):
        nMin = 0;
        nMax = 1;

        // skip '?'
        MoveNext();
        break;

    case RCHART('*'):
        nMin = 0;
        nMax = INT_MAX;

        // skip '*'
        MoveNext();
        break;

    case RCHART('+'):
        nMin = 1;
        nMax = INT_MAX;

        // skip '+'
        MoveNext();
        break;

    default:
        bIsQuantifier = 0;
        break;
    }

    // do quantify
    if (bIsQuantifier)
    {
        // 0 times
        if (nMax == 0)
            return GetStockElx(STOCKELX_EMPTY);

        // fixed times
        if (nMin == nMax)
        {
            if (curr == CHART_INFO(RCHART('?'), 1) || curr == CHART_INFO(RCHART('+'), 1))
                MoveNext();

            return Keep(new CRepeatElx(pSimple, nMin));
        }

        // range times
        if (curr == CHART_INFO(RCHART('?'), 1))
        {
            MoveNext();
            return Keep(new CReluctantElx(pSimple, nMin, nMax));
        }
        else if (curr == CHART_INFO(RCHART('+'), 1))
        {
            MoveNext();
            return Keep(new CPossessiveElx(pSimple, nMin, nMax));
        }
        else
        {
            return Keep(new CGreedyElx(pSimple, nMin, nMax));
        }
    }

    return pSimple;
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildSimple(int & flags)
{
    CBufferT <CHART> fixed;

    while (curr != CHART_INFO(0, 1))
    {
        if (curr.type == 0)
        {
            if (next == CHART_INFO(RCHART('{'), 1) || next == CHART_INFO(RCHART('?'), 1) || next == CHART_INFO(RCHART('*'), 1) || next == CHART_INFO(RCHART('+'), 1))
            {
                if (fixed.GetSize() == 0)
                {
                    fixed.Append(curr.ch, 1);
                    MoveNext();
                }

                break;
            }
            else
            {
                fixed.Append(curr.ch, 1);
                MoveNext();
            }
        }
        else if (curr.type == 1)
        {
            CHART vch = curr.ch;

            // end of simple
            if (vch == RCHART(')') || vch == RCHART('|'))
                break;

            // has fixed already
            if (fixed.GetSize() > 0)
                break;

            // left parentheses
            if (vch == RCHART('('))
            {
                return BuildRecursive(flags);
            }

            // char set
            if (vch == RCHART('[') || vch == RCHART('.') || vch == RCHART('w') || vch == RCHART('W') ||
                vch == RCHART('s') || vch == RCHART('S') || vch == RCHART('d') || vch == RCHART('D')
                )
            {
                return BuildCharset(flags);
            }

            // boundary
            if (vch == RCHART('^') || vch == RCHART('$') || vch == RCHART('A') || vch == RCHART('Z') || vch == RCHART('z') ||
                vch == RCHART('b') || vch == RCHART('B') || vch == RCHART('G') // vch == RCHART('<') || vch == RCHART('>')
                )
            {
                return BuildBoundary(flags);
            }

            // backref
            if (vch == RCHART('\\') || vch == RCHART('k') || vch == RCHART('g'))
            {
                return BuildBackref(flags);
            }

            // treat vchar as char
            fixed.Append(curr.ch, 1);
            MoveNext();
        }
    }

    if (fixed.GetSize() > 0)
        return Keep(new CStringElxT <CHART>(fixed.GetBuffer(), fixed.GetSize(), flags & RIGHTTOLEFT, flags & IGNORECASE));
    else
        return GetStockElx(STOCKELX_EMPTY);
}

#define deelx_max(a, b)  (((a) > (b)) ? (a) : (b))
#define deelx_min(a, b)  (((a) < (b)) ? (a) : (b))

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildCharset(int & flags)
{
    // char
    CHART ch = curr.ch;

    // skip
    MoveNext();

    switch (ch)
    {
    case RCHART('.'):
        return GetStockElx(
            (flags & RIGHTTOLEFT) ?
            ((flags & SINGLELINE) ? STOCKELX_DOT_ALL_RIGHTLEFT : STOCKELX_DOT_NOT_ALL_RIGHTLEFT) :
            ((flags & SINGLELINE) ? STOCKELX_DOT_ALL : STOCKELX_DOT_NOT_ALL)
            );

    case RCHART('w'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_WORD_RIGHTLEFT : STOCKELX_WORD);

    case RCHART('W'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_WORD_RIGHTLEFT_NOT : STOCKELX_WORD_NOT);

    case RCHART('s'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_SPACE_RIGHTLEFT : STOCKELX_SPACE);

    case RCHART('S'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_SPACE_RIGHTLEFT_NOT : STOCKELX_SPACE_NOT);

    case RCHART('d'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_DIGITAL_RIGHTLEFT : STOCKELX_DIGITAL);

    case RCHART('D'):
        return GetStockElx((flags & RIGHTTOLEFT) ? STOCKELX_DIGITAL_RIGHTLEFT_NOT : STOCKELX_DIGITAL_NOT);

    case RCHART('['):
        {
            CRangeElxT <CHART> * pRange;

            // create
            if (curr == CHART_INFO(RCHART(':'), 1))
            {
                // Backup before posix
                Snapshot shot;
                Backup(&shot);

                CBufferT <char> posix;

                do
                {
                    posix.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                    MoveNext();
                } while (curr.ch != RCHART(0) && curr != CHART_INFO(RCHART(']'), 1));

                MoveNext(); // skip ']'

                // posix
                CPosixElxT<CHART> * pposix = (CPosixElxT<CHART> *) Keep(new CPosixElxT <CHART>(posix.GetBuffer(), flags & RIGHTTOLEFT));
                if (pposix->m_posixfun != 0)
                {
                    return pposix;
                }

                // restore if not posix
                Restore(&shot);
            }

            if (curr == CHART_INFO(RCHART('^'), 1))
            {
                MoveNext(); // skip '^'
                pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(flags & RIGHTTOLEFT, 0));
            }
            else
            {
                pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART>(flags & RIGHTTOLEFT, 1));
            }

            // parse
            while (curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART(']'), 1))
            {
                ch = curr.ch;

                if (curr.type == 1 && (
                    ch == RCHART('.') || ch == RCHART('w') || ch == RCHART('W') || ch == RCHART('s') || ch == RCHART('S') || ch == RCHART('d') || ch == RCHART('D') ||
                    (ch == RCHART('[') && next == CHART_INFO(RCHART(':'), 1))
                    ))
                {
                    pRange->m_embeds.Push(BuildCharset(flags));
                }
                else if (next == CHART_INFO(RCHART('-'), 1) && nex2.type == 0)
                {
                    pRange->m_ranges.Push(ch); pRange->m_ranges.Push(nex2.ch);

                    // next
                    MoveNext();
                    MoveNext();
                    MoveNext();
                }
                else
                {
                    pRange->m_chars.Push(ch);

                    // next
                    MoveNext();
                }
            }

            // skip ']'
            MoveNext();

            if (flags & IGNORECASE)
            {
                CBufferT <CHART> & ranges = pRange->m_ranges;
                index_t i, oldcount = ranges.GetSize() / 2;

                for (i = 0; i < oldcount; i++)
                {
                    CHART newmin, newmax;

                    if (ranges[i * 2] <= RCHART('Z') && ranges[i * 2 + 1] >= RCHART('A'))
                    {
                        newmin = tolower(deelx_max(RCHART('A'), ranges[i * 2]));
                        newmax = tolower(deelx_min(RCHART('Z'), ranges[i * 2 + 1]));

                        if (newmin < ranges[i * 2] || newmax > ranges[i * 2 + 1])
                        {
                            ranges.Push(newmin);
                            ranges.Push(newmax);
                        }
                    }

                    if (ranges[i * 2] <= RCHART('z') && ranges[i * 2 + 1] >= RCHART('a'))
                    {
                        newmin = toupper(deelx_max(RCHART('a'), ranges[i * 2]));
                        newmax = toupper(deelx_min(RCHART('z'), ranges[i * 2 + 1]));

                        if (newmin < ranges[i * 2] || newmax > ranges[i * 2 + 1])
                        {
                            ranges.Push(newmin);
                            ranges.Push(newmax);
                        }
                    }
                }

                CBufferT <CHART> & chars = pRange->m_chars;
                oldcount = chars.GetSize();
                for (i = 0; i < oldcount; i++)
                {
                    if (isupper(chars[i]) && !pRange->IsContainChar(tolower(chars[i])))
                        chars.Push(tolower(chars[i]));

                    if (islower(chars[i]) && !pRange->IsContainChar(toupper(chars[i])))
                        chars.Push(toupper(chars[i]));
                }
            }

            return pRange;
        }
    }

    return GetStockElx(STOCKELX_EMPTY);
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildRecursive(int & flags)
{
    // skip '('
    MoveNext();

    if (curr == CHART_INFO(RCHART('?'), 1))
    {
        ElxInterface * pElx = nullptr;

        // skip '?'
        MoveNext();

        int bNegative = 0;
        CHART named_end = RCHART('>');

        switch (curr.ch)
        {
        case RCHART('!'):
            bNegative = 1;

        case RCHART('='):
            {
                MoveNext(); // skip '!' or '='
                pElx = Keep(new CAssertElx(BuildAlternative(flags & ~RIGHTTOLEFT), !bNegative));
            }
            break;

        case RCHART('<'):
            switch (next.ch)
            {
            case RCHART('!'):
                bNegative = 1;

            case RCHART('='):
                MoveNext(); // skip '<'
                MoveNext(); // skip '!' or '='
                {
                    pElx = Keep(new CAssertElx(BuildAlternative(flags | RIGHTTOLEFT), !bNegative));
                }
                break;

            default: // named group
                break;
            }
            // break if assertion // else named
            if (pElx != nullptr) break;

        case RCHART('P'):
            if (curr.ch == RCHART('P')) MoveNext(); // skip 'P'

        case RCHART('\''):
            if (curr.ch == RCHART('<')) named_end = RCHART('>');
            else if (curr.ch == RCHART('\'')) named_end = RCHART('\'');
            MoveNext(); // skip '<' or '\''
            {
                CListElx    * pList = (CListElx    *)Keep(new CListElx(flags & RIGHTTOLEFT));
                CBracketElx * pleft = (CBracketElx *)Keep(new CBracketElx(-1, (flags & RIGHTTOLEFT) ? 1 : 0));
                CBracketElx * pright = (CBracketElx *)Keep(new CBracketElx(-1, (flags & RIGHTTOLEFT) ? 0 : 1));

                // save name
                CBufferT <CHART> & name = pleft->m_szNamed, &balancing_name = pleft->m_szBalancing, *pname = &name;
                CBufferT <char> num, balancing_num, *pnum = &num;

                while (curr.ch != RCHART(0) && curr.ch != named_end)
                {
                    if (curr.ch == RCHART('-'))
                    {
                        pname = &balancing_name;
                        pnum = &balancing_num;
                        MoveNext();
                        continue;
                    }

                    pname->Append(curr.ch, 1);
                    pnum->Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                    MoveNext();
                }
                MoveNext(); // skip '>' or '\''

                // check <num>
                unsigned int number;
                char * str = num.GetBuffer();

                if (ReadDec(str, number) ? (*str == '\0') : false)
                {
                    pleft->m_nnumber = number;
                    pright->m_nnumber = number;

                    name.Release();
                }

                str = balancing_num.GetBuffer();
                if (ReadDec(str, number) ? (*str == '\0') : false)
                {
                    pleft->m_balancing = number;
                    pright->m_balancing = number;

                    balancing_name.Release();
                }

                // left, center, right
                pList->m_elxlist.Push(pleft);
                pList->m_elxlist.Push(BuildAlternative(flags));
                pList->m_elxlist.Push(pright);

                // named number
                if (pleft->m_nnumber >= 0 || name.GetSize() > 0)
                {
                    index_t nThisBackref = m_nNextNamed++;
                    m_namedlist.Prepare(nThisBackref);
                    m_namedlist[nThisBackref] = pList;
                }
                else if (pleft->m_balancing >= 0 || balancing_name.GetSize() > 0)
                {
                    int nThisBalancing = m_nNextBalancing++;
                    m_purebalancinglist.Prepare(nThisBalancing, 0);
                    m_purebalancinglist[nThisBalancing] = pList;
                }
                else
                {
                    // TODO ERROR
                }

                pElx = pList;
            }
            break;

        case RCHART('>'):
            {
                MoveNext(); // skip '>'
                pElx = Keep(new CIndependentElx(BuildAlternative(flags)));
            }
            break;

        case RCHART('R'):
            MoveNext(); // skip 'R'
            while (curr.ch != RCHART(0) && isspace(curr.ch)) MoveNext(); // skip space

            if (curr.ch == RCHART('<') || curr.ch == RCHART('\''))
            {
                named_end = curr.ch == RCHART('<') ? RCHART('>') : RCHART('\'');
                CDelegateElx * pDelegate = (CDelegateElx *)Keep(new CDelegateElx(-3));

                MoveNext(); // skip '<' or '\\'

                // save name
                CBufferT <CHART> & name = pDelegate->m_szNamed;
                CBufferT <char> num;

                while (curr.ch != RCHART(0) && curr.ch != named_end)
                {
                    name.Append(curr.ch, 1);
                    num.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                    MoveNext();
                }
                MoveNext(); // skip '>' or '\''

                // check <num>
                unsigned int number;
                char * str = num.GetBuffer();

                if (ReadDec(str, number) ? (*str == '\0') : false)
                {
                    pDelegate->m_ndata = number;
                    name.Release();
                }

                m_recursivelist.Push(pDelegate);
                pElx = pDelegate;
            }
            else
            {
                CBufferT <char> rto;
                while (curr.ch != RCHART(0) && curr.ch != RCHART(')'))
                {
                    rto.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                    MoveNext();
                }

                unsigned int rtono = 0;
                char * str = rto.GetBuffer();
                ReadDec(str, rtono);

                CDelegateElx * pDelegate = (CDelegateElx *)Keep(new CDelegateElx(rtono));

                m_recursivelist.Push(pDelegate);
                pElx = pDelegate;
            }
            break;

        case RCHART('('):
            {
                CConditionElx * pConditionElx = (CConditionElx *)Keep(new CConditionElx());

                // condition
                ElxInterface * & pCondition = pConditionElx->m_pelxask;

                if (next == CHART_INFO(RCHART('?'), 1))
                {
                    pCondition = BuildRecursive(flags);
                }
                else // named, assert or number
                {
                    MoveNext(); // skip '('
                    int pos0 = curr.pos;

                    // save elx condition
                    pCondition = Keep(new CAssertElx(BuildAlternative(flags), 1));

                    // save name
                    pConditionElx->m_szNamed.Append(m_pattern.GetBuffer() + pos0, curr.pos - pos0, 1);

                    // save number
                    CBufferT <char> numstr;
                    while (pos0 < curr.pos)
                    {
                        CHART ch = m_pattern[pos0];
                        numstr.Append(((ch & (CHART)0xff) == ch) ? (char)ch : 0, 1);
                        pos0++;
                    }

                    unsigned int number;
                    char * str = numstr.GetBuffer();

                    // valid group number
                    if (ReadDec(str, number) ? (*str == '\0') : false)
                    {
                        pConditionElx->m_nnumber = number;
                        pCondition = nullptr;
                    }
                    else // maybe elx, maybe named
                    {
                        pConditionElx->m_nnumber = -1;
                        m_namedconditionlist.Push(pConditionElx);
                    }

                    MoveNext(); // skip ')'
                }

                // alternative
        {
            int newflags = flags;

            pConditionElx->m_pelxyes = BuildList(newflags);
        }

        if (curr.ch == RCHART('|'))
        {
            MoveNext(); // skip '|'

            pConditionElx->m_pelxno = BuildAlternative(flags);
        }
        else
        {
            pConditionElx->m_pelxno = 0;
        }

        pElx = pConditionElx;
            }
            break;

        default:
            while (curr.ch != RCHART(0) && isspace(curr.ch)) MoveNext(); // skip space

            if (curr.ch >= RCHART('0') && curr.ch <= RCHART('9')) // recursive (?1) => (?R1)
            {
                CBufferT <char> rto;
                while (curr.ch != RCHART(0) && curr.ch != RCHART(')'))
                {
                    rto.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
                    MoveNext();
                }

                unsigned int rtono = 0;
                char * str = rto.GetBuffer();
                ReadDec(str, rtono);

                CDelegateElx * pDelegate = (CDelegateElx *)Keep(new CDelegateElx(rtono));

                m_recursivelist.Push(pDelegate);
                pElx = pDelegate;
            }
            else
            {
                // flag
                int newflags = flags;
                while (curr != CHART_INFO(0, 1) && curr.ch != RCHART(':') && curr.ch != RCHART(')') && curr != CHART_INFO(RCHART('('), 1))
                {
                    int tochange = 0;

                    switch (curr.ch)
                    {
                    case RCHART('i'):
                    case RCHART('I'):
                        tochange = IGNORECASE;
                        break;

                    case RCHART('s'):
                    case RCHART('S'):
                        tochange = SINGLELINE;
                        break;

                    case RCHART('m'):
                    case RCHART('M'):
                        tochange = MULTILINE;
                        break;

                    case RCHART('g'):
                    case RCHART('G'):
                        tochange = GLOBAL;
                        break;

                    case RCHART('-'):
                        bNegative = 1;
                        break;
                    }

                    if (bNegative)
                        newflags &= ~tochange;
                    else
                        newflags |= tochange;

                    // move to next char
                    MoveNext();
                }

                if (curr.ch == RCHART(':') || curr == CHART_INFO(RCHART('('), 1))
                {
                    // skip ':'
                    if (curr.ch == RCHART(':')) MoveNext();

                    pElx = BuildAlternative(newflags);
                }
                else
                {
                    // change parent flags
                    flags = newflags;

                    pElx = GetStockElx(STOCKELX_EMPTY);
                }
            }
            break;
        }

        MoveNext(); // skip ')'

        return pElx;
    }
    else
    {
        // group and number
        CListElx * pList = (CListElx *)Keep(new CListElx(flags & RIGHTTOLEFT));
        index_t nThisBackref = ++m_nMaxNumber;

        // left, center, right
        pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, (flags & RIGHTTOLEFT) ? 1 : 0)));
        pList->m_elxlist.Push(BuildAlternative(flags));
        pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, (flags & RIGHTTOLEFT) ? 0 : 1)));

        // for recursive
        m_grouplist.Prepare(nThisBackref);
        m_grouplist[nThisBackref] = pList;

        // right
        MoveNext(); // skip ')' 

        return pList;
    }
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildBoundary(int & flags)
{
    // char
    CHART ch = curr.ch;

    // skip
    MoveNext();

    switch (ch)
    {
    case RCHART('^'):
        return Keep(new CBoundaryElxT <CHART>((flags & MULTILINE) ? BOUNDARY_LINE_BEGIN : BOUNDARY_FILE_BEGIN));

    case RCHART('$'):
        return Keep(new CBoundaryElxT <CHART>((flags & MULTILINE) ? BOUNDARY_LINE_END : BOUNDARY_FILE_END));

    case RCHART('b'):
        return Keep(new CBoundaryElxT <CHART>(BOUNDARY_WORD_EDGE));

    case RCHART('B'):
        return Keep(new CBoundaryElxT <CHART>(BOUNDARY_WORD_EDGE, 0));

    case RCHART('A'):
        return Keep(new CBoundaryElxT <CHART>(BOUNDARY_FILE_BEGIN));

    case RCHART('Z'):
        return Keep(new CBoundaryElxT <CHART>(BOUNDARY_FILE_END_N));

    case RCHART('z'):
        return Keep(new CBoundaryElxT <CHART>(BOUNDARY_FILE_END));

    case RCHART('G'):
        if (flags & GLOBAL)
            return Keep(new CGlobalElx());
        else
            return GetStockElx(STOCKELX_EMPTY);

    default:
        return GetStockElx(STOCKELX_EMPTY);
    }
}

template <class CHART> ElxInterface * CBuilderT <CHART> ::BuildBackref(int & flags)
{
    // skip '\\' or '\k' or '\g'
    MoveNext();

    if (curr.ch == RCHART('<') || curr.ch == RCHART('\''))
    {
        CHART named_end = curr.ch == RCHART('<') ? RCHART('>') : RCHART('\'');
        CBackrefElxT <CHART> * pbackref = (CBackrefElxT <CHART> *)Keep(new CBackrefElxT <CHART>(-1, flags & RIGHTTOLEFT, flags & IGNORECASE));

        MoveNext(); // skip '<' or '\''

        // save name
        CBufferT <CHART> & name = pbackref->m_szNamed;
        CBufferT <char> num;

        while (curr.ch != RCHART(0) && curr.ch != named_end)
        {
            name.Append(curr.ch, 1);
            num.Append(((curr.ch & (CHART)0xff) == curr.ch) ? (char)curr.ch : 0, 1);
            MoveNext();
        }
        MoveNext(); // skip '>' or '\''

        // check <num>
        unsigned int number;
        char * str = num.GetBuffer();

        if (ReadDec(str, number) ? (*str == '\0') : false)
        {
            pbackref->m_nnumber = number;
            name.Release();
        }
        else
        {
            m_namedbackreflist.Push(pbackref);
        }

        return pbackref;
    }
    else
    {
        unsigned int nbackref = 0;

        for (int i = 0; i < 3; i++)
        {
            if (curr.ch >= RCHART('0') && curr.ch <= RCHART('9'))
                nbackref = nbackref * 10 + (curr.ch - RCHART('0'));
            else
                break;

            MoveNext();
        }

        return Keep(new CBackrefElxT <CHART>(nbackref, flags & RIGHTTOLEFT, flags & IGNORECASE));
    }
}

template <class CHART> int CBuilderT <CHART> ::ReadDec(char * & str, unsigned int & dec)
{
    int s = 0;
    while (str[s] != 0 && isspace(str[s])) s++;

    if (str[s] < '0' || str[s] > '9') return 0;

    dec = 0;
    unsigned int i;

    for (i = s; i < sizeof(CHART) * 3 + s; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
            dec = dec * 10 + (str[i] - '0');
        else
            break;
    }

    while (str[i] != 0 && isspace(str[i])) i++;
    str += i;

    return 1;
}

//
// Regexp
//
template <class CHART> class CRegexpT
{
public:
    CRegexpT(const CHART * pattern = 0, int flags = 0);
    CRegexpT(const CHART * pattern, index_t length, int flags);
    void Compile(const CHART * pattern, int flags = 0);
    void Compile(const CHART * pattern, index_t length, int flags);

public:
    MatchResult MatchExact(const CHART * tstring, CContext * pContext = nullptr) const;
    MatchResult MatchExact(const CHART * tstring, index_t length, CContext * pContext = nullptr) const;
    MatchResult Match(const CHART * tstring, index_t start = -1, CContext * pContext = nullptr) const;
    MatchResult Match(const CHART * tstring, index_t length, index_t start, CContext * pContext = nullptr) const;
    MatchResult Match(CContext * pContext) const;
    CContext * PrepareMatch(const CHART * tstring, index_t start = -1, CContext * pContext = nullptr) const;
    CContext * PrepareMatch(const CHART * tstring, index_t length, index_t start, CContext * pContext = nullptr) const;
    CHART * Replace(const CHART * tstring, const CHART * replaceto, index_t start = -1, int ntimes = -1, MatchResult * result = nullptr, CContext * pContext = nullptr) const;
    CHART * Replace(const CHART * tstring, index_t string_length, const CHART * replaceto, index_t to_length, index_t & result_length, index_t start = -1, int ntimes = -1, MatchResult * result = nullptr, CContext * pContext = nullptr) const;
    int GetNamedGroupNumber(const CHART * group_name) const;

public:
    static void ReleaseString(CHART    * tstring);
    static void ReleaseContext(CContext * pContext);

public:
    CBuilderT <CHART> m_builder;
};

//
// Implementation
//
template <class CHART> CRegexpT <CHART> ::CRegexpT(const CHART * pattern, int flags)
{
    Compile(pattern, CBufferRefT<CHART>(pattern).GetSize(), flags);
}

template <class CHART> CRegexpT <CHART> ::CRegexpT(const CHART * pattern, index_t length, int flags)
{
    Compile(pattern, length, flags);
}

template <class CHART> inline void CRegexpT <CHART> ::Compile(const CHART * pattern, int flags)
{
    Compile(pattern, CBufferRefT<CHART>(pattern).GetSize(), flags);
}

template <class CHART> void CRegexpT <CHART> ::Compile(const CHART * pattern, index_t length, int flags)
{
    m_builder.Clear();
    if (pattern != 0) m_builder.Build(CBufferRefT<CHART>(pattern, length), flags);
}

template <class CHART> inline MatchResult CRegexpT <CHART> ::MatchExact(const CHART * tstring, CContext * pContext) const
{
    return MatchExact(tstring, CBufferRefT<CHART>(tstring).GetSize(), pContext);
}

template <class CHART> MatchResult CRegexpT <CHART> ::MatchExact(const CHART * tstring, index_t length, CContext * pContext) const
{
    if (m_builder.m_pTopElx == 0)
        return nullptr;

    // info
    int endpos = 0;

    CContext context;
    if (pContext == nullptr) pContext = &context;

    pContext->m_stack.Restore(0);
    pContext->m_capturestack.Restore(0);
    pContext->m_captureindex.Restore(0);

    pContext->m_nParenZindex = 0;
    pContext->m_nLastBeginPos = -1;
    pContext->m_pMatchString = (void*)tstring;
    pContext->m_pMatchStringLength = length;
    pContext->m_nCursiveLimit = 100;

    if (m_builder.m_nFlags & RIGHTTOLEFT)
    {
        pContext->m_nBeginPos = length;
        pContext->m_nCurrentPos = length;
        endpos = 0;
    }
    else
    {
        pContext->m_nBeginPos = 0;
        pContext->m_nCurrentPos = 0;
        endpos = length;
    }

    pContext->m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
    pContext->m_captureindex[0] = 0;
    pContext->m_capturestack.Push(0);
    pContext->m_capturestack.Push(pContext->m_nCurrentPos);
    pContext->m_capturestack.Push(-1);
    pContext->m_capturestack.Push(-1);

    // match
    if (!m_builder.m_pTopElx->Match(pContext))
        return nullptr;
    else
    {
        while (pContext->m_nCurrentPos != endpos)
        {
            if (!m_builder.m_pTopElx->MatchNext(pContext))
                return nullptr;
            else
            {
                if (pContext->m_nLastBeginPos == pContext->m_nBeginPos && pContext->m_nBeginPos == pContext->m_nCurrentPos)
                    return nullptr;
                else
                    pContext->m_nLastBeginPos = pContext->m_nCurrentPos;
            }
        }

        // end pos
        pContext->m_capturestack[2] = pContext->m_nCurrentPos;

        return MatchResult(pContext, m_builder.m_nMaxNumber);
    }
}

template <class CHART> MatchResult CRegexpT <CHART> ::Match(const CHART * tstring, index_t start, CContext * pContext) const
{
    return Match(tstring, CBufferRefT<CHART>(tstring).GetSize(), start, pContext);
}

template <class CHART> MatchResult CRegexpT <CHART> ::Match(const CHART * tstring, index_t length, index_t start, CContext * pContext) const
{
    if (m_builder.m_pTopElx == 0)
        return nullptr;

    CContext context;
    if (pContext == nullptr) pContext = &context;

    PrepareMatch(tstring, length, start, pContext);

    return Match(pContext);
}

template <class CHART> MatchResult CRegexpT <CHART> ::Match(CContext * pContext) const
{
    if (m_builder.m_pTopElx == 0)
        return nullptr;

    index_t endpos;
    int delta;

    if (m_builder.m_nFlags & RIGHTTOLEFT)
    {
        endpos = -1;
        delta = -1;
    }
    else
    {
        endpos = pContext->m_pMatchStringLength + 1;
        delta = 1;
    }

    while (pContext->m_nCurrentPos != endpos)
    {
        pContext->m_captureindex.Restore(0);
        pContext->m_stack.Restore(0);
        pContext->m_capturestack.Restore(0);

        pContext->m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
        pContext->m_captureindex[0] = 0;
        pContext->m_capturestack.Push(0);
        pContext->m_capturestack.Push(pContext->m_nCurrentPos);
        pContext->m_capturestack.Push(-1);
        pContext->m_capturestack.Push(-1);

        if (m_builder.m_pTopElx->Match(pContext))
        {
            pContext->m_capturestack[2] = pContext->m_nCurrentPos;

            // zero width
            if (pContext->m_capturestack[1] == pContext->m_nCurrentPos)
            {
                pContext->m_nCurrentPos += delta;
            }

            // save pos
            pContext->m_nLastBeginPos = pContext->m_nBeginPos;
            pContext->m_nBeginPos = pContext->m_nCurrentPos;

            // return
            return MatchResult(pContext, m_builder.m_nMaxNumber);
        }
        else
        {
            pContext->m_nCurrentPos += delta;
        }
    }

    return nullptr;
}

template <class CHART> inline CContext * CRegexpT <CHART> ::PrepareMatch(const CHART * tstring, index_t start, CContext * pContext) const
{
    return PrepareMatch(tstring, CBufferRefT<CHART>(tstring).GetSize(), start, pContext);
}

template <class CHART> CContext * CRegexpT <CHART> ::PrepareMatch(const CHART * tstring, index_t length, index_t start, CContext * pContext) const
{
    if (m_builder.m_pTopElx == 0)
        return nullptr;

    if (pContext == nullptr) pContext = new CContext();

    pContext->m_nParenZindex = 0;
    pContext->m_nLastBeginPos = -1;
    pContext->m_pMatchString = (void*)tstring;
    pContext->m_pMatchStringLength = length;
    pContext->m_nCursiveLimit = 100;

    if (start < 0)
    {
        if (m_builder.m_nFlags & RIGHTTOLEFT)
        {
            pContext->m_nBeginPos = length;
            pContext->m_nCurrentPos = length;
        }
        else
        {
            pContext->m_nBeginPos = 0;
            pContext->m_nCurrentPos = 0;
        }
    }
    else
    {
        if (start > length) start = length + ((m_builder.m_nFlags & RIGHTTOLEFT) ? 0 : 1);

        pContext->m_nBeginPos = start;
        pContext->m_nCurrentPos = start;
    }

    return pContext;
}

template <class CHART> inline int CRegexpT <CHART> ::GetNamedGroupNumber(const CHART * group_name) const
{
    return m_builder.GetNamedNumber(group_name);
}

template <class CHART> CHART * CRegexpT <CHART> ::Replace(const CHART * tstring, const CHART * replaceto, index_t start, int ntimes, MatchResult * result, CContext * pContext) const
{
    index_t result_length = 0;
    return Replace(tstring, CBufferRefT<CHART>(tstring).GetSize(), replaceto, CBufferRefT<CHART>(replaceto).GetSize(), result_length, start, ntimes, result, pContext);
}

template <class CHART> CHART * CRegexpT <CHART> ::Replace(const CHART * tstring, index_t string_length, const CHART * replaceto, index_t to_length, index_t & result_length, index_t start, int ntimes, MatchResult * remote_result, CContext * oContext) const
{
    if (m_builder.m_pTopElx == 0) return 0;

    // --- compile replace to ---

    CBufferT <index_t> compiledto;

    static const CHART rtoptn[] = { RCHART('\\'), RCHART('$'), RCHART('('), RCHART('?'), RCHART(':'), RCHART('['), RCHART('$'), RCHART('&'), RCHART('`'), RCHART('\''), RCHART('+'), RCHART('_'), RCHART('\\'), RCHART('d'), RCHART(']'), RCHART('|'), RCHART('\\'), RCHART('{'), RCHART('.'), RCHART('*'), RCHART('?'), RCHART('\\'), RCHART('}'), RCHART(')'), RCHART('\0') };
    static CRegexpT <CHART> rtoreg(rtoptn);

    MatchResult local_result(nullptr), *result = remote_result ? remote_result : &local_result;

    // prepare
    CContext * pContext = rtoreg.PrepareMatch(replaceto, to_length, -1, oContext);
    index_t lastIndex = 0;
    index_t nmatch = 0;

    while (((*result) = rtoreg.Match(pContext)).IsMatched())
    {
        index_t delta = result->GetStart() - lastIndex;
        if (delta > 0)
        {
            compiledto.Push(lastIndex);
            compiledto.Push(delta);
        }

        lastIndex = result->GetStart();
        delta = 2;

        switch (replaceto[lastIndex + 1])
        {
        case RCHART('$'):
            compiledto.Push(lastIndex);
            compiledto.Push(1);
            break;

        case RCHART('&'):
        case RCHART('`'):
        case RCHART('\''):
        case RCHART('+'):
        case RCHART('_'):
            compiledto.Push(-1);
            compiledto.Push((int)replaceto[lastIndex + 1]);
            break;

        case RCHART('{'):
            delta = result->GetEnd() - result->GetStart();
            nmatch = m_builder.GetNamedNumber(CBufferRefT <CHART>(replaceto + (lastIndex + 2), delta - 3));

            if (nmatch > 0 && nmatch <= m_builder.m_nMaxNumber)
            {
                compiledto.Push(-2);
                compiledto.Push(nmatch);
            }
            else
            {
                compiledto.Push(lastIndex);
                compiledto.Push(delta);
            }
            break;

        default:
            nmatch = 0;
            for (delta = 1; delta <= 3; delta++)
            {
                CHART ch = replaceto[lastIndex + delta];

                if (ch < RCHART('0') || ch > RCHART('9'))
                    break;

                nmatch = nmatch * 10 + (ch - RCHART('0'));
            }

            if (nmatch > m_builder.m_nMaxNumber)
            {
                while (nmatch > m_builder.m_nMaxNumber)
                {
                    nmatch /= 10;
                    delta--;
                }

                if (nmatch == 0)
                {
                    delta = 1;
                }
            }

            if (delta == 1)
            {
                compiledto.Push(lastIndex);
                compiledto.Push(1);
            }
            else
            {
                compiledto.Push(-2);
                compiledto.Push(nmatch);
            }
            break;
        }

        lastIndex += delta;
    }

    if (lastIndex < to_length)
    {
        compiledto.Push(lastIndex);
        compiledto.Push(to_length - lastIndex);
    }

    int rightleft = m_builder.m_nFlags & RIGHTTOLEFT;

    index_t cmplSize = compiledto.GetSize();
    index_t tb = rightleft ? cmplSize - 2 : 0;
    index_t te = rightleft ? -2 : cmplSize;
    index_t ts = rightleft ? -2 : 2;

    // --- compile complete ---

    index_t beginpos = rightleft ? string_length : 0;
    index_t endpos = rightleft ? 0 : string_length;

    index_t toIndex0 = 0;
    index_t toIndex1 = 0;
    index_t i;
    int ntime;

    CBufferT <const CHART *> buffer;

    // prepare
    pContext = PrepareMatch(tstring, string_length, start, pContext);
    lastIndex = beginpos;

    // Match
    for (ntime = 0; ntimes < 0 || ntime < ntimes; ntime++)
    {
        (*result) = Match(pContext);

        if (!result->IsMatched())
            break;

        // before
        if (rightleft)
        {
            index_t distance = lastIndex - result->GetEnd();
            if (distance)
            {
                buffer.Push(tstring + result->GetEnd());
                buffer.Push((const CHART *)distance);

                toIndex1 -= distance;
            }
            lastIndex = result->GetStart();
        }
        else
        {
            index_t distance = result->GetStart() - lastIndex;
            if (distance)
            {
                buffer.Push(tstring + lastIndex);
                buffer.Push((const CHART *)distance);

                toIndex1 += distance;
            }
            lastIndex = result->GetEnd();
        }

        toIndex0 = toIndex1;

        // middle
        for (i = tb; i != te; i += ts)
        {
            index_t off = compiledto[i];
            index_t len = compiledto[i + 1];

            const CHART * sub = replaceto + off;

            if (off == -1)
            {
                switch (RCHART(len))
                {
                case RCHART('&'):
                    sub = tstring + result->GetStart();
                    len = result->GetEnd() - result->GetStart();
                    break;

                case RCHART('`'):
                    sub = tstring;
                    len = result->GetStart();
                    break;

                case RCHART('\''):
                    sub = tstring + result->GetEnd();
                    len = string_length - result->GetEnd();
                    break;

                case RCHART('+'):
                    for (nmatch = result->MaxGroupNumber(); nmatch >= 0; nmatch--)
                    {
                        if (result->GetGroupStart(nmatch) >= 0) break;
                    }
                    sub = tstring + result->GetGroupStart(nmatch);
                    len = result->GetGroupEnd(nmatch) - result->GetGroupStart(nmatch);
                    break;

                case RCHART('_'):
                    sub = tstring;
                    len = string_length;
                    break;
                }
            }
            else if (off == -2)
            {
                //TODO: @@@ check to use nmatch instead of len here ???
                int l = static_cast<int>(len);
                sub = tstring + result->GetGroupStart(l);
                len = result->GetGroupEnd(l) - result->GetGroupStart(l);
            }

            buffer.Push(sub);
            buffer.Push((const CHART *)len);

            toIndex1 += rightleft ? (-len) : len;
        }
    }

    // after
    if (rightleft)
    {
        if (endpos < lastIndex)
        {
            buffer.Push(tstring + endpos);
            buffer.Push((const CHART *)(lastIndex - endpos));
        }
    }
    else
    {
        if (lastIndex < endpos)
        {
            buffer.Push(tstring + lastIndex);
            buffer.Push((const CHART *)(endpos - lastIndex));
        }
    }

    if (oContext == nullptr) ReleaseContext(pContext);

    // join string
    result_length = 0;
    for (i = 0; i < buffer.GetSize(); i += 2)
    {
        result_length += (index_t)buffer[i + 1];
    }

    CBufferT <CHART> result_string;
    result_string.Prepare(result_length);
    result_string.Restore(0);

    if (rightleft)
    {
        for (i = buffer.GetSize() - 2; i >= 0; i -= 2)
        {
            result_string.Append(buffer[i], (index_t)buffer[i + 1]);
        }
    }
    else
    {
        for (i = 0; i < buffer.GetSize(); i += 2)
        {
            result_string.Append(buffer[i], (index_t)buffer[i + 1]);
        }
    }

    result_string.Append(0);

    result->m_result.Append(result_length, 3);
    result->m_result.Append(ntime);

    if (rightleft)
    {
        result->m_result.Append(result_length - toIndex1);
        result->m_result.Append(result_length - toIndex0);
    }
    else
    {
        result->m_result.Append(toIndex0);
        result->m_result.Append(toIndex1);
    }

    return result_string.Detach();
}

template <class CHART> inline void CRegexpT <CHART> ::ReleaseString(CHART * tstring)
{
    if (tstring != 0) free(tstring);
}

template <class CHART> inline void CRegexpT <CHART> ::ReleaseContext(CContext * pContext)
{
    if (pContext != nullptr) delete pContext;
}

//
// All implementations
//
template <index_t x> CAlternativeElxT <x> ::CAlternativeElxT() = default;

template <index_t x> int CAlternativeElxT <x> ::Match(CContext * pContext) const
{
    if (m_elxlist.GetSize() == 0)
        return 1;

    // try all
    for (int n = 0; n < m_elxlist.GetSize(); n++)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            pContext->m_stack.Push(n);
            return 1;
        }
    }

    return 0;
}

template <index_t x> int CAlternativeElxT <x> ::MatchNext(CContext * pContext) const
{
    if (m_elxlist.GetSize() == 0)
        return 0;

    index_t n = 0;

    // recall prev
    pContext->m_stack.Pop(n);

    // prev
    if (m_elxlist[n]->MatchNext(pContext))
    {
        pContext->m_stack.Push(n);
        return 1;
    }
    else
    {
        // try rest
        for (n++; n < m_elxlist.GetSize(); n++)
        {
            if (m_elxlist[n]->Match(pContext))
            {
                pContext->m_stack.Push(n);
                return 1;
            }
        }

        return 0;
    }
}

// assertx.cpp: implementation of the CAssertElx class.
//
template <index_t x> CAssertElxT <x> ::CAssertElxT(ElxInterface * pelx, int byes)
{
    m_pelx = pelx;
    m_byes = byes;
}

template <index_t x> int CAssertElxT <x> ::Match(CContext * pContext) const
{
    index_t nbegin = pContext->m_nCurrentPos;
    index_t nsize = pContext->m_stack.GetSize();
    index_t ncsize = pContext->m_capturestack.GetSize();
    int bsucc;

    // match
    if (m_byes)
        bsucc = m_pelx->Match(pContext);
    else
        bsucc = !m_pelx->Match(pContext);

    // status
    pContext->m_stack.Restore(nsize);
    pContext->m_nCurrentPos = nbegin;

    if (bsucc)
        pContext->m_stack.Push(ncsize);
    else
        pContext->m_capturestack.Restore(ncsize);

    return bsucc;
}

template <index_t x> int CAssertElxT <x> ::MatchNext(CContext * pContext) const
{
    index_t ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_capturestack.Restore(ncsize);

    return 0;
}

// emptyelx.cpp: implementation of the CEmptyElx class.
//
template <index_t x> CEmptyElxT <x> ::CEmptyElxT() = default;

template <index_t x> int CEmptyElxT <x> ::Match(CContext *) const
{
    return 1;
}

template <index_t x> int CEmptyElxT <x> ::MatchNext(CContext *) const
{
    return 0;
}

// globalx.cpp: implementation of the CGlobalElx class.
//
template <index_t x> CGlobalElxT <x> ::CGlobalElxT() = default;

template <index_t x> int CGlobalElxT <x> ::Match(CContext * pContext) const
{
    return pContext->m_nCurrentPos == pContext->m_nBeginPos;
}

template <index_t x> int CGlobalElxT <x> ::MatchNext(CContext *) const
{
    return 0;
}

// greedelx.cpp: implementation of the CGreedyElx class.
//
template <index_t x> CGreedyElxT <x> ::CGreedyElxT(ElxInterface * pelx, int nmin, int nmax) : CRepeatElxT <x>(pelx, nmin)
{
    m_nvart = nmax - nmin;
}

template <index_t x> int CGreedyElxT <x> ::Match(CContext * pContext) const
{
    if (!CRepeatElxT <x> ::MatchFixed(pContext))
        return 0;

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
            return 0;
    }

    return 1;
}

template <index_t x> int CGreedyElxT <x> ::MatchNext(CContext * pContext) const
{
    if (MatchNextVart(pContext))
        return 1;

    if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
        return 0;

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
            return 0;
    }

    return 1;
}

template <index_t x> int CGreedyElxT <x> ::MatchVart(CContext * pContext) const
{
    int n = 0;
    index_t nbegin00 = pContext->m_nCurrentPos;
    index_t nsize = pContext->m_stack.GetSize();
    index_t ncsize = pContext->m_capturestack.GetSize();

    while (n < m_nvart && CRepeatElx::MatchForward(pContext))
    {
        n++;
    }

    pContext->m_stack.Push(ncsize);
    pContext->m_stack.Push(nsize);
    pContext->m_stack.Push(pContext->m_nCurrentPos);
    pContext->m_stack.Push(1);
    pContext->m_stack.Push(nbegin00);
    pContext->m_stack.Push(n);

    return 1;
}

template <index_t x> int CGreedyElxT <x> ::MatchNextVart(CContext * pContext) const
{
    index_t n, nbegin00, nsize, ncsize;
    CSortedBufferT <index_t> nbegin99;
    pContext->m_stack.Pop(n);
    pContext->m_stack.Pop(nbegin00);
    pContext->m_stack.Pop(nbegin99);
    pContext->m_stack.Pop(nsize);
    pContext->m_stack.Pop(ncsize);

    if (n == 0) return 0;

    index_t n0 = n;

    if (!CRepeatElxT<x>::m_pelx->MatchNext(pContext))
    {
        n--;
    }

    // not to re-match
    else if (pContext->m_nCurrentPos == nbegin00)
    {
        pContext->m_stack.Restore(nsize);
        pContext->m_capturestack.Restore(ncsize);
        pContext->m_nCurrentPos = nbegin00;

        return 0;
    }

    // fix 2012-10-26, thanks to chenlx01@sohu.com
    else
    {
        CContextShot shot(pContext);

        while (n < m_nvart && CRepeatElx::MatchForward(pContext))
        {
            n++;
        }

        if (nbegin99.Find(pContext->m_nCurrentPos) >= 0)
        {
            shot.Restore(pContext);
            n = n0;
        }
        else
        {
            nbegin99.Add(pContext->m_nCurrentPos);
        }
    }

    pContext->m_stack.Push(ncsize);
    pContext->m_stack.Push(nsize);
    pContext->m_stack.Push(nbegin99);
    pContext->m_stack.Push(nbegin00);
    pContext->m_stack.Push(n);

    return 1;
}

// indepelx.cpp: implementation of the CIndependentElx class.
//
template <index_t x> CIndependentElxT <x> ::CIndependentElxT(ElxInterface * pelx)
{
    m_pelx = pelx;
}

template <index_t x> int CIndependentElxT <x> ::Match(CContext * pContext) const
{
    index_t nbegin = pContext->m_nCurrentPos;
    index_t nsize = pContext->m_stack.GetSize();
    index_t ncsize = pContext->m_capturestack.GetSize();

    // match
    int bsucc = m_pelx->Match(pContext);

    // status
    pContext->m_stack.Restore(nsize);

    if (bsucc)
    {
        pContext->m_stack.Push(nbegin);
        pContext->m_stack.Push(ncsize);
    }

    return bsucc;
}

template <index_t x> int CIndependentElxT <x> ::MatchNext(CContext * pContext) const
{
    index_t nbegin = 0, ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_stack.Pop(nbegin);

    pContext->m_capturestack.Restore(ncsize);
    pContext->m_nCurrentPos = nbegin;

    return 0;
}

// listelx.cpp: implementation of the CListElx class.
//
template <index_t x> CListElxT <x> ::CListElxT(int brightleft)
{
    m_brightleft = brightleft;
}

template <index_t x> int CListElxT <x> ::Match(CContext * pContext) const
{
    if (m_elxlist.GetSize() == 0)
        return 1;

    // prepare
    index_t bol = m_brightleft ? m_elxlist.GetSize() : -1;
    index_t stp = m_brightleft ? -1 : 1;
    index_t eol = m_brightleft ? -1 : m_elxlist.GetSize();

    // from first
    index_t n = bol + stp;

    // match all
    while (n != eol)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            n += stp;
        }
        else
        {
            n -= stp;

            while (n != bol && !m_elxlist[n]->MatchNext(pContext))
                n -= stp;

            if (n != bol)
                n += stp;
            else
                return 0;
        }
    }

    return 1;
}

template <index_t x> int CListElxT <x> ::MatchNext(CContext * pContext) const
{
    if (m_elxlist.GetSize() == 0)
        return 0;

    // prepare
    index_t bol = m_brightleft ? m_elxlist.GetSize() : -1;
    index_t stp = m_brightleft ? -1 : 1;
    index_t eol = m_brightleft ? -1 : m_elxlist.GetSize();

    // from last
    index_t n = eol - stp;

    while (n != bol && !m_elxlist[n]->MatchNext(pContext))
        n -= stp;

    if (n != bol)
        n += stp;
    else
        return 0;

    // match rest
    while (n != eol)
    {
        if (m_elxlist[n]->Match(pContext))
        {
            n += stp;
        }
        else
        {
            n -= stp;

            while (n != bol && !m_elxlist[n]->MatchNext(pContext))
                n -= stp;

            if (n != bol)
                n += stp;
            else
                return 0;
        }
    }

    return 1;
}

// mresult.cpp: implementation of the MatchResult class.
//
template <index_t x> MatchResultT <x> ::MatchResultT(CContext * pContext, index_t nMaxNumber)
{
    if (pContext != nullptr)
    {
        m_result.Prepare(nMaxNumber * 2 + 3, -1);

        // matched
        m_result[0] = 1;
        m_result[1] = nMaxNumber;

        for (int n = 0; n <= nMaxNumber; n++)
        {
            index_t index = pContext->m_captureindex[n];
            //if( index < 0 ) continue;
            if (!CBracketElxT<char>::CheckCaptureIndex(index, pContext, n)) continue;
            // check enclosed
            index_t pos1 = pContext->m_capturestack[index + 1];
            index_t pos2 = pContext->m_capturestack[index + 2];

            // info
            m_result[n * 2 + 2] = pos1 < pos2 ? pos1 : pos2;
            m_result[n * 2 + 3] = pos1 < pos2 ? pos2 : pos1;
        }
    }
}

template <index_t x> inline int MatchResultT <x> ::IsMatched() const
{
    return static_cast<int>(m_result.At(0, 0));
}

template <index_t x> inline index_t MatchResultT <x> ::MaxGroupNumber() const
{
    return m_result.At(1, 0);
}

template <index_t x> inline index_t MatchResultT <x> ::GetStart() const
{
    return m_result.At(2, -1);
}

template <index_t x> inline index_t MatchResultT <x> ::GetEnd() const
{
    return m_result.At(3, -1);
}

template <index_t x> inline index_t MatchResultT <x> ::GetGroupStart(index_t nGroupNumber) const
{
    return m_result.At(2 + nGroupNumber * 2, -1);
}

template <index_t x> inline index_t MatchResultT <x> ::GetGroupEnd(index_t nGroupNumber) const
{
    return m_result.At(2 + nGroupNumber * 2 + 1, -1);
}

template <index_t x> MatchResultT <x> & MatchResultT <x> :: operator = (const MatchResultT <x> & result)
{
    m_result.Restore(0);
    if (result.m_result.GetSize() > 0) m_result.Append(result.m_result.GetBuffer(), result.m_result.GetSize());

    return *this;
}

// posselx.cpp: implementation of the CPossessiveElx class.
//
template <int x> CPossessiveElxT <x> ::CPossessiveElxT(ElxInterface * pelx, int nmin, int nmax) : CGreedyElxT <x>(pelx, nmin, nmax)
{}

template <int x> int CPossessiveElxT <x> ::Match(CContext * pContext) const
{
    index_t nbegin = pContext->m_nCurrentPos;
    index_t nsize = pContext->m_stack.GetSize();
    index_t ncsize = pContext->m_capturestack.GetSize();
    int bsucc = 1;

    // match
    if (!CRepeatElxT <x> ::MatchFixed(pContext))
    {
        bsucc = 0;
    }
    else
    {
        while (!CGreedyElxT <x> ::MatchVart(pContext))
        {
            if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
            {
                bsucc = 0;
                break;
            }
        }
    }

    // status
    pContext->m_stack.Restore(nsize);

    if (bsucc)
    {
        pContext->m_stack.Push(nbegin);
        pContext->m_stack.Push(ncsize);
    }

    return bsucc;
}

template <int x> int CPossessiveElxT <x> ::MatchNext(CContext * pContext) const
{
    index_t nbegin = 0, ncsize = 0;

    pContext->m_stack.Pop(ncsize);
    pContext->m_stack.Pop(nbegin);

    pContext->m_capturestack.Restore(ncsize);
    pContext->m_nCurrentPos = nbegin;

    return 0;
}

// reluctx.cpp: implementation of the CReluctantElx class.
//
template <int x> CReluctantElxT <x> ::CReluctantElxT(ElxInterface * pelx, int nmin, int nmax) : CRepeatElxT <x>(pelx, nmin)
{
    m_nvart = nmax - nmin;
}

template <int x> int CReluctantElxT <x> ::Match(CContext * pContext) const
{
    if (!CRepeatElxT <x> ::MatchFixed(pContext))
        return 0;

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
            return 0;
    }

    return 1;
}

template <int x> int CReluctantElxT <x> ::MatchNext(CContext * pContext) const
{
    if (MatchNextVart(pContext))
        return 1;

    if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
        return 0;

    while (!MatchVart(pContext))
    {
        if (!CRepeatElxT <x> ::MatchNextFixed(pContext))
            return 0;
    }

    return 1;
}

template <int x> int CReluctantElxT <x> ::MatchVart(CContext * pContext)
{
    pContext->m_stack.Push(0);

    return 1;
}

template <int x> int CReluctantElxT <x> ::MatchNextVart(CContext * pContext) const
{
    index_t n = 0, nbegin = pContext->m_nCurrentPos;

    pContext->m_stack.Pop(n);

    if (n < m_nvart && CRepeatElxT <x> ::m_pelx->Match(pContext))
    {
        while (pContext->m_nCurrentPos == nbegin)
        {
            if (!CRepeatElxT <x> ::m_pelx->MatchNext(pContext)) break;
        }

        if (pContext->m_nCurrentPos != nbegin)
        {
            n++;

            pContext->m_stack.Push(nbegin);
            pContext->m_stack.Push(n);

            return 1;
        }
    }

    while (n > 0)
    {
        pContext->m_stack.Pop(nbegin);

        while (CRepeatElxT <x> ::m_pelx->MatchNext(pContext))
        {
            if (pContext->m_nCurrentPos != nbegin)
            {
                pContext->m_stack.Push(nbegin);
                pContext->m_stack.Push(n);

                return 1;
            }
        }

        n--;
    }

    return 0;
}

// repeatx.cpp: implementation of the CRepeatElx class.
//
template <int x> CRepeatElxT <x> ::CRepeatElxT(ElxInterface * pelx, int ntimes)
{
    m_pelx = pelx;
    m_nfixed = ntimes;
}

template <int x> int CRepeatElxT <x> ::Match(CContext * pContext) const
{
    return MatchFixed(pContext);
}

template <int x> int CRepeatElxT <x> ::MatchNext(CContext * pContext) const
{
    return MatchNextFixed(pContext);
}

template <int x> int CRepeatElxT <x> ::MatchFixed(CContext * pContext) const
{
    if (m_nfixed == 0)
        return 1;

    int n = 0;

    while (n < m_nfixed)
    {
        if (m_pelx->Match(pContext))
        {
            n++;
        }
        else
        {
            n--;

            while (n >= 0 && !m_pelx->MatchNext(pContext))
                n--;

            if (n >= 0)
                n++;
            else
                return 0;
        }
    }

    return 1;
}

template <int x> int CRepeatElxT <x> ::MatchNextFixed(CContext * pContext) const
{
    if (m_nfixed == 0)
        return 0;

    // from last
    int n = m_nfixed - 1;

    while (n >= 0 && !m_pelx->MatchNext(pContext))
        n--;

    if (n >= 0)
        n++;
    else
        return 0;

    // match rest
    while (n < m_nfixed)
    {
        if (m_pelx->Match(pContext))
        {
            n++;
        }
        else
        {
            n--;

            while (n >= 0 && !m_pelx->MatchNext(pContext))
                n--;

            if (n >= 0)
                n++;
            else
                return 0;
        }
    }

    return 1;
}

// Regexp
using CRegexpA = CRegexpT <char>;
using CRegexpW = CRegexpT <unsigned short>;

#if defined(_UNICODE) || defined(UNICODE)
using CRegexp = CRegexpW;
#else
using CRegexp = CRegexpA;
#endif

}

#endif//__DEELX_REGEXP64__H__
