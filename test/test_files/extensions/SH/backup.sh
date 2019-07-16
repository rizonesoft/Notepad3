#! /bin/sh
# Make backups.

# Copyright 2004-2006, 2013-2014, 2016-2017 Free Software Foundation,
# Inc.

# This file is part of GNU tar.

# GNU tar is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# GNU tar is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PROGNAME=`basename $0`
CONFIGPATH="$SYSCONFDIR/backup"
REMOTEBACKUPDIR="$SYSCONFDIR/tar-backup"
CONFIGFILE=${CONFIGPATH}/backup-specs
DIRLIST=${CONFIGPATH}/dirs
FILELIST=${CONFIGPATH}/files
LOGPATH=${CONFIGPATH}/log

# Default functions for running various magnetic tape commands
mt_begin() {
    $MT -f "$1" retension
}

mt_rewind() {
    $MT -f "$1" rewind
}

mt_offline() {
    $MT -f "$1" offl
}

mt_status() {
    $MT -f "$1" status
}

# The main configuration file may override any of these variables
MT_BEGIN=mt_begin
MT_REWIND=mt_rewind
MT_OFFLINE=mt_offline
MT_STATUS=mt_status

# Insure 'mail' is in PATH.
PATH="/usr/ucb:${PATH}"
export PATH
# Put startdate in the subject line of mailed report, since if it happens
# to run longer than 24 hours (as may be the case if someone forgets to put
# in the next volume of the tape in adequate time), the backup date won't
# appear too misleading.
startdate="`date`"
here="`pwd`"
# Save local hostname
localhost="`hostname | sed -e 's/\..*//' | tr A-Z a-z`"

# Produce a diagnostic output
message() {
    if [ "$VERBOSE" != "" ]; then
	if [ $VERBOSE -ge $1 ]; then
	    shift
	    echo "$@" >&2
	fi
    fi
}

# Bail out and exit.
bailout() {
    echo "$PROGNAME: $*" >&2
    exit 1
}

# Return current date
now() {
	date +%Y-%m-%d
}

# Bail out if we don't have root privileges.
test_root() {
    if [ ! -w ${ROOT_FS-/} ]; then
	bailout "The backup must be run as root or else some files will fail to be dumped."
    fi
}

root_fs() {
    echo "${ROOT_FS}$1" | tr -s /
}

advice() {
    echo "Directory $1 is not found." >&2
    cat >&2 <<EOF
The following directories and files are needed for the backup to function:

1. Directory with configuration files and file lists:
$CONFIGPATH
2. Directory for backup log files
$LOGPATH
3. Main configuration file
$CONFIGFILE

Please, create these and invoke the script again.
EOF
}

init_common() {
    # Check if the necessary directories exist
    if [ ! -d $CONFIGPATH ]; then
	advice $CONFIGPATH
	exit 1
    fi
    if [ ! -d $LOGPATH ]; then
	if mkdir $LOGPATH; then
	    :
	else
	    advice $LOGPATH
	    exit 1
	fi
    fi
    # Get the values of BACKUP_DIRS, BACKUP_FILES, and other variables.
    if [ ! -r $CONFIGFILE ]; then
	echo "$PROGNAME: cannot read $CONFIGFILE. Stop." >&2
	exit 1
    fi
    . $CONFIGFILE

    # Environment sanity check

    test_root

    if [ x"${ADMINISTRATOR}" = x ]; then
	bailout "ADMINISTRATOR not defined"
    fi

    [ x"$TAR" = x ] && TAR=tar
    [ x"$SLEEP_TIME" = x ] && SLEEP_TIME=60

    if [ x$VOLNO_FILE = x ]; then
	bailout "VOLNO_FILE not specified"
    fi

    if [ -r $DIRLIST ]; then
	BACKUP_DIRS="$BACKUP_DIRS `cat $DIRLIST`"
    fi
    if [ -r $FILELIST ]; then
	BACKUP_FILES="$BACKUP_FILES `cat $FILELIST`"
    fi

    if [ \( x"$BACKUP_DIRS" = x \) -a \( x"$BACKUP_FILES" = x \) ]; then
	bailout "Neither BACKUP_DIRS nor BACKUP_FILES specified"
    fi
    if [ -z "$RSH" ]; then
	RSH=rsh
	MT_RSH_OPTION=
    else
	MT_RSH_OPTION="--rsh-command=$RSH"
    fi
    if [ -z "$TAPE_FILE" ]; then
	TAPE_FILE=/dev/tape
    fi

    # If TAPE_FILE is a remote device, update mt invocation accordingly
    : ${MT:=mt}
    case $TAPE_FILE in
    *:*) MT="$MT $MT_RSH_OPTION";;
    *)   ;;
    esac

    POSIXLY_CORRECT=1
    export POSIXLY_CORRECT
}

init_backup() {
    init_common
    TAR_PART1="${TAR} -c --format=gnu --multi-volume --one-file-system --sparse --volno-file=${VOLNO_FILE}"
    if [ "x$XLIST" != x ]; then
	TAR_PART1="${TAR_PART1} \`test -r $REMOTEBACKUPDIR/$XLIST && echo \"--exclude-from $REMOTEBACKUPDIR/$XLIST\"\`"
    fi
    if [ "$RSH_COMMAND" != "" ]; then
	TAR_PART1="${TAR_PART1} --rsh-command=$RSH_COMMAND"
    fi
    if [ x$BLOCKING != x ]; then
	TAR_PART1="${TAR_PART1} --blocking=${BLOCKING}"
    fi

    # Only use --info-script if DUMP_REMIND_SCRIPT was defined in backup-specs
    if [ "x${DUMP_REMIND_SCRIPT}" != "x" ]; then
	TAR_PART1="${TAR_PART1} --info-script='${DUMP_REMIND_SCRIPT}'"
    fi
    # Set logfile name
    # Logfile name should be in the form 'log-1993-03-18-level-0'
    # They go in the directory '/usr/etc/log'.
    # i.e. year-month-date.  This format is useful for sorting by name, since
    # logfiles are intentionally kept online for future reference.
    LOGFILE="${LOGPATH}/log-`now`-level-${DUMP_LEVEL}"
}

init_restore() {
    init_common
    # FIXME: Replace --list with --extract
    TAR_PART1="${TAR} --extract --multi-volume"
    if [ "$RSH_COMMAND" != "" ]; then
	TAR_PART1="${TAR_PART1} --rsh-command=$RSH_COMMAND"
    fi
    if [ x$BLOCKING != x ]; then
	TAR_PART1="${TAR_PART1} --blocking=${BLOCKING}"
    fi

    # Only use --info-script if DUMP_REMIND_SCRIPT was defined in backup-specs
    if [ "x${DUMP_REMIND_SCRIPT}" != "x" ]; then
	TAR_PART1="${TAR_PART1} --info-script='${DUMP_REMIND_SCRIPT}'"
    fi
    LOGFILE="${LOGPATH}/restore-`now`"
}

wait_time() {
    if [ "${1}" != "now" ]; then
	if [ "${1}x" != "x" ]; then
	    spec="${1}"
	else
	    spec="${BACKUP_HOUR}"
	fi

	pausetime="`date | awk -v spec=\"${spec}\" '
		BEGIN {
		    split(spec, time, ":")
		}
		{
		    split($4, now, ":")
		    diff = 3600 * (time[1] - now[1]) + 60 * (time[2] - now[2]);
		    if (diff < 0)
			diff += 3600 * 24
		    print diff
		}'`"
	clear
	echo "${SLEEP_MESSAGE}"
	sleep "${pausetime}"
    fi
}

level_log_name() {
    echo "$REMOTEBACKUPDIR/${1}.level-${2-$DUMP_LEVEL}"
}

# Prepare a temporary level logfile
# usage: make_level_log HOSTNAME
make_level_log() {
    if [ "z${localhost}" != "z$1" ] ; then
	$RSH "$1" mkdir $REMOTEBACKUPDIR > /dev/null 2>&1
        $RSH "$1" rm -f `level_log_name temp`
    else
        mkdir $REMOTEBACKUPDIR > /dev/null 2>&1
        rm -f `level_log_name temp`
    fi
}

# Rename temporary log
# usage: flush_level_log HOSTNAME FSNAME
flush_level_log() {
    message 10 "RENAME: `level_log_name temp` --> `level_log_name $2`"
    if [ "z${localhost}" != "z$1" ] ; then
	$RSH "$1" mv -f `level_log_name temp` "`level_log_name $2`"
    else
        mv -f `level_log_name temp` "`level_log_name $2`"
    fi
}

# Return the timestamp of the last backup.
# usage: get_dump_time LEVEL
get_dump_time() {
    ls -r ${LOGPATH}/log-*-level-$1 \
        | head -n 1 \
	| sed "s,.*log-\(.*\)-level-$1,\1,"
}

# Do actual backup on a host
# usage: backup_host HOSTNAME [TAR_ARGUMENTS]
backup_host() {
    message 10 "ARGS: $@"
    rhost=$1
    shift
    if [ "z${localhost}" != "z$rhost" ] ; then
	$RSH "$rhost" ${TAR_PART1} -f "${localhost}:${TAPE_FILE}" $@
    else
	# Using 'sh -c exec' causes nested quoting and shell substitution
        # to be handled here in the same way rsh handles it.
        CMD="exec ${TAR_PART1} -f \"${TAPE_FILE}\" $@"
        message 10 "CMD: $CMD"
        sh -c "$CMD"
        RC=$?
        message 10 "RC: $RC"
        return $RC
    fi
}

print_level() {
    if [ ${1-$DUMP_LEVEL} -eq 0 ]; then
	echo "Full"
    else
    	echo "Level ${1-$DUMP_LEVEL}"
    fi
}

prev_level() {
    print_level `expr $DUMP_LEVEL - 1` | tr A-Z a-z
}

remote_run() {
    rhost=$1
    shift
    message 10 "REMOTE $rhost: $@"
    if [ "x$rhost" != "x${localhost}" ] ; then
	$RSH "${rhost}" "$@"
    else
        $*
    fi
}

license() {
    cat - <<EOF
Copyright (C) 2013 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
EOF
}
