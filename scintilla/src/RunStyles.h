/** @file RunStyles.h
 ** Data structure used to store sparse styles.
 **/
// Copyright 1998-2007 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/// Styling buffer using one element for each run rather than using
/// a filled buffer.

#ifndef RUNSTYLES_H
#define RUNSTYLES_H

namespace Scintilla {

template <typename DISTANCE, typename STYLE>
class RunStyles {
private:
	std::unique_ptr<Partitioning<DISTANCE>> starts;
	std::unique_ptr<SplitVector<STYLE>> styles;
	DISTANCE RunFromPosition(DISTANCE position) const;
	DISTANCE SplitRun(DISTANCE position);
	void RemoveRun(DISTANCE run);
	void RemoveRunIfEmpty(DISTANCE run);
	void RemoveRunIfSameAsPrevious(DISTANCE run);
public:
	RunStyles();
	// Deleted so RunStyles objects can not be copied.
	RunStyles(const RunStyles &) = delete;
	void operator=(const RunStyles &) = delete;
	~RunStyles();
	DISTANCE Length() const;
	STYLE ValueAt(DISTANCE position) const;
	DISTANCE FindNextChange(DISTANCE position, DISTANCE end) const;
	DISTANCE StartRun(DISTANCE position) const;
	DISTANCE EndRun(DISTANCE position) const;
	// Returns true if some values may have changed
	bool FillRange(DISTANCE &position, STYLE value, DISTANCE &fillLength);
	void SetValueAt(DISTANCE position, STYLE value);
	void InsertSpace(DISTANCE position, DISTANCE insertLength);
	void DeleteAll();
	void DeleteRange(DISTANCE position, DISTANCE deleteLength);
	DISTANCE Runs() const;
	bool AllSame() const;
	bool AllSameAs(STYLE value) const;
	DISTANCE Find(STYLE value, DISTANCE start) const;

	void Check() const;
};

}

#endif
