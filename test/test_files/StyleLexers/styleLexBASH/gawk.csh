alias gawkpath_default 'unsetenv AWKPATH; setenv AWKPATH `gawk -v x=AWKPATH "BEGIN {print ENVIRON[x]}"`'

alias gawkpath_prepend 'if (! $?AWKPATH) setenv AWKPATH ""; if ($AWKPATH == "") then; unsetenv AWKPATH; setenv AWKPATH `gawk -v x=AWKPATH "BEGIN {print ENVIRON[x]}"`; endif; setenv AWKPATH "\!*"":$AWKPATH"'

alias gawkpath_append 'if (! $?AWKPATH) setenv AWKPATH ""; if ($AWKPATH == "") then; unsetenv AWKPATH; setenv AWKPATH `gawk -v x=AWKPATH "BEGIN {print ENVIRON[x]}"`; endif; setenv AWKPATH "$AWKPATH"":\!*"'

alias gawklibpath_default 'unsetenv AWKLIBPATH; setenv AWKLIBPATH `gawk -v x=AWKLIBPATH "BEGIN {print ENVIRON[x]}"`'

alias gawklibpath_prepend 'if (! $?AWKLIBPATH) setenv AWKLIBPATH ""; if ($AWKLIBPATH == "") then; unsetenv AWKLIBPATH; setenv AWKLIBPATH `gawk -v x=AWKLIBPATH "BEGIN {print ENVIRON[x]}"`; endif; setenv AWKLIBPATH "\!*"":$AWKLIBPATH"'

alias gawklibpath_append 'if (! $?AWKLIBPATH) setenv AWKLIBPATH ""; if ($AWKLIBPATH == "") then; unsetenv AWKLIBPATH; setenv AWKLIBPATH `gawk -v x=AWKLIBPATH "BEGIN {print ENVIRON[x]}"`; endif; setenv AWKLIBPATH "$AWKLIBPATH"":\!*"'
