# PTRDIFF_DOESNT_ALIAS_INT Preprocessor Option
# TYPE: Code patch (applied)
#
# Scintilla 5.5.8 adds support for this preprocessor define for platforms
# where ptrdiff_t doesn't alias int (affects Scintilla message handling).
#
# CHANGE: Adds `|| defined(PTRDIFF_DOESNT_ALIAS_INT)` to the condition
#         in RunStyles.cxx that enables ptrdiff_t template instantiations.
#
# FILE: scintilla/src/RunStyles.cxx (line 324)
#
# STATUS: ✅ APPLIED
