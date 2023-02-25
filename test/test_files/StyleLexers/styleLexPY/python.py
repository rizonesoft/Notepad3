# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------
"""
Utilities and Helpers for General Tasks
"""
import sys
import os
import stat
import getpass
import shutil
import tempfile
import logging

from datetime import datetime

# =============================================================================

X = 'blah'
LOG_TEST = rf"test{X}test"

WRITE_TO_LOGFILE = False
DEFAULT_LOG_LEVEL = logging.INFO
LOG_DIR_CONFIG_FILENAME = "_DBMIG_TMP_DIR_CFG.ini"

# =============================================================================

# TODO: unique keys should not be empty
_NOT_NULL_EXCEPTION_LIST = [('deductions', 'imrestype_ref'),
                            ('queue lengths', 'imrestype_ref'),
                            ('neighbour networkpoint distances', 'traffic network'),
                            ('occupations', 'day'),
                            ('tb_loc_prop', 'internal_id')]
                            #('flighttypes', 'bgcolor_ref'),
                            #('managedstations', 'routings_ref'),
                            #('routings', 'classification_ref'),
                            #('routings', 'timezone_ref')]

# force FK references to be not null
_NOT_NULL_FK_REF_LIST = ['managedstations']

# =============================================================================

def eprint(strg):
    """ print to stderr """
    print(strg, file=sys.stderr)
    sys.stderr.flush()

# =============================================================================

def is_ascii(strg):
    """ is_ascii() """
    #~return all(ord(c) < 128 for c in strg)
    try:
        strg.encode('ascii')
    except UnicodeEncodeError:
        return False
    return True


def is_mbcs(strg):
    """ is_ascii() """
    try:
        strg.encode('mbcs')
    except UnicodeEncodeError:
        return False
    return True

# =============================================================================

def check_read_access(input_file):
    """ check_read_access """
    if not os.path.isfile(input_file):
        eprint("Input file ({}) not found!".format(input_file))
        return False

    if not os.access(input_file, os.R_OK):
        eprint("Input file ({}) is not readable!".format(input_file))
        eprint(f"Input file ({input_file}) is not readable!")
        return False

    return True

# =============================================================================


def check_write_access(output_path):
    """ check_write_access """
    from tempfile import NamedTemporaryFile

    # check if file exists
    if os.path.isfile(output_path):

        if not os.access(output_path, os.W_OK):
            eprint("Output file ({}) is not writable!".format(output_path))
            return False
        try:
            os.rename(output_path, output_path)   # hack alert
        except OSError as err:
            eprint("Error on write access: {}".format(err))
            return False

    else:   # check if directory is writable
        try:
            out_dir = os.path.dirname(output_path)
            with NamedTemporaryFile(dir=out_dir, delete=True) as tmpfile:
                tmpfile.write(b'blahblub')
                tmpfile.flush()
        except OSError as err:
            eprint("Error on write access: {}".format(err))
            return False

    return True

# =============================================================================


def compose_log_file_path(db_path=None,aprefix="PY"):
    """ compose_log_file_path() """

    global WRITE_TO_LOGFILE
    log_file_dir = None
    log_file_path = None

    try:
        # assuming this is deterministic
        cfg_log_path = os.path.join(tempfile.gettempdir(), LOG_DIR_CONFIG_FILENAME)
        if os.path.isfile(cfg_log_path):
            with open(cfg_log_path, 'r') as file:
                log_file_dir = file.readline()
    except OSError as err:
        eprint("Error on reading file '{file}' : {err}".format(file=cfg_log_path, err=err))
        log_file_dir = None
    except:  # pylint: disable=bare-except
        eprint("Error on composing logfile path!")
        log_file_dir = None
    finally:
        if log_file_dir:
             WRITE_TO_LOGFILE = True
        if db_path and not log_file_dir:
            log_file_dir = os.path.dirname(db_path)

    if not (log_file_dir and os.path.isdir(log_file_dir)):
        return None

    if not check_write_access(log_file_dir):
        eprint("No write access to: '{dir}!'".format(dir=log_file_dir))
        return None

    if db_path:
        base_name = "_".join([aprefix, os.path.basename(db_path)])
    else:
        # no logfile specified, but log_file_dir given: make temp name
        tmp_path = tempfile.NamedTemporaryFile(prefix=aprefix, suffix="")
        base_name = os.path.basename(tmp_path.name)

    time_stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S.%f")[:-3]
    pid_str = "{}".format(os.getpid())
    log_file_name = ".".join([base_name[:16], getpass.getuser(), pid_str, time_stamp, "log"])

    return os.path.join(log_file_dir, log_file_name)


# =============================================================================

MAIN_LOGGER = "thePLPythonMainLogger"

def create_logger(log_file_path=None):
    """ create_logger() """
    if WRITE_TO_LOGFILE and log_file_path:
        have_write_access = check_write_access(log_file_path)
    else:
        have_write_access = False
    # create logger
    logger = logging.getLogger(MAIN_LOGGER)
    # create file handler which logs even debug messages
    if have_write_access:
        try:
            if os.path.isfile(log_file_path):
                os.remove(log_file_path)
            file_handler = logging.FileHandler(log_file_path, 'w', encoding='UTF-8')
            file_handler.setLevel(logging.NOTSET)
        except OSError as err:
            # don't use eprint() cause this will trigger error meassage in Planning
            print("Exception: Can not remove old logfile {}!".format(log_file_path))
            have_write_access = false
    # create console handler with a higher log level
    con_stdout_handler = logging.StreamHandler(stream=sys.stdout)
    con_stdout_handler.setLevel(logging.NOTSET)
    con_stderr_handler = logging.StreamHandler(stream=sys.stderr)
    con_stderr_handler.setLevel(logging.ERROR)
    # create formatter and add it to the handlers
    #formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
    if have_write_access:
        file_handler.setFormatter(formatter)
    con_stdout_handler.setFormatter(formatter)
    con_stderr_handler.setFormatter(formatter)
    # add the handlers to the logger
    if have_write_access:
        logger.addHandler(file_handler)
    logger.addHandler(con_stdout_handler)
    logger.addHandler(con_stderr_handler)
    # set default loglevel
    logger.setLevel(DEFAULT_LOG_LEVEL)
    return logger

# -----------------------------------------------------------------------------

def get_main_logger():
    """ get_main_logger() """
    return logging.getLogger(MAIN_LOGGER)

# -----------------------------------------------------------------------------

def flush_logger():
    """ flush_logger() """
    logger = get_main_logger()
    for handler in logger.handlers:
        handler.flush()

# =============================================================================


def add_read_permission(pathname, who='u'):
    """Add "read" permission to specified path.
    """
    mode = os.stat(pathname).st_mode
    mode_map = {
        'u': stat.S_IRUSR,
        'g': stat.S_IRGRP,
        'o': stat.S_IROTH,
        'a': stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH,
    }
    for w in who:
        mode |= mode_map[w]
    os.chmod(pathname, mode)

# -----------------------------------------------------------------------------

def add_write_permission(pathname, who='u'):
    """Add "write" permission to specified path.
    """
    mode = os.stat(pathname).st_mode
    mode_map = {
        'u': stat.S_IWUSR,
        'g': stat.S_IWGRP,
        'o': stat.S_IWOTH,
        'a': stat.S_IWUSR | stat.S_IWGRP | stat.S_IWOTH,
    }
    for w in who:
        mode |= mode_map[w]
    os.chmod(pathname, mode)

# -----------------------------------------------------------------------------

def add_execute_permission(pathname, who='u'):
    """Add "write" permission to specified path.
    """
    mode = os.stat(pathname).st_mode
    mode_map = {
        'u': stat.S_IXUSR,
        'g': stat.S_IXGRP,
        'o': stat.S_IXOTH,
        'a': stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH,
    }
    for w in who:
        mode |= mode_map[w]
    os.chmod(pathname, mode)

# =============================================================================


def get_ordered_unique_list(seq, rev=False):
    """ get_ordered_unique_list() """
    return sorted(set(seq), reverse=rev)

# -----------------------------------------------------------------------------


def get_missing_elements(sorted_list_of_int, start=None, end=None):
    """ get_missing_elements()
        call:  list(migutils.get_missing_elements(migutils.get_ordered_unique_list(seq)))
    """
    if not start:
        start = 0
    if not end:
        end = len(sorted_list_of_int) - 1

    if (end - start) <= 1:
        if (sorted_list_of_int[end] - sorted_list_of_int[start]) > 1:
            yield from range(sorted_list_of_int[start] + 1, sorted_list_of_int[end])
        return

    index = start + (end - start) // 2

    # is the lower half consecutive?
    consecutive_low = sorted_list_of_int[index] == (sorted_list_of_int[start] + (index - start))
    if not consecutive_low:
        yield from get_missing_elements(sorted_list_of_int, start, index)

    # is the upper part consecutive?
    consecutive_high = sorted_list_of_int[index] == (sorted_list_of_int[end] - (end - index))
    if not consecutive_high:
        yield from get_missing_elements(sorted_list_of_int, index, end)

# -----------------------------------------------------------------------------

def create_temp_filecopy(logger, fpath, fprefix="py_", fsuffix=".tmp"):
    """ create_temp_filecopy
    """
    #~tmp_file_name = db_path + ".tmp_{0}.mdb".format(os.getlogin())
    tmp_file = tempfile.NamedTemporaryFile(prefix=fprefix, suffix=fsuffix)
    tmp_file_name = tmp_file.name
    tmp_file.close()  # deletes tmp file
    logger.info("Creating temporary copy %r.", tmp_file_name)
    copy_path = shutil.copy2(fpath, tmp_file_name)
    if not os.path.isfile(copy_path):
        logger.error("Can not create a temp. copy of '%r'.", fpath)
        return None
    add_read_permission(copy_path, 'a')
    add_write_permission(copy_path, 'a')
    return copy_path

# -----------------------------------------------------------------------------

# =============================================================================

class CiDict(dict):
    """ Case-Insensitive dictionary
    """

    class Key(str):
        """ Key """
        def __init__(self, key):
            """ __init__"""
            str.__init__(key)

        def __hash__(self):
            """ __hash__"""
            return hash(self.lower())

        def __eq__(self, other):
            """ __eq__"""
            return self.lower() == other.lower()

    # ------------------------------------------------

    def __init__(self, data=None):
        """ C'tor of CiDict """
        super(CiDict, self).__init__()
        if data is None:
            data = {}
        for (key, val) in data.items():
            self[key] = val

    def __contains__(self, key):
        """ __contains__ """
        key = self.Key(key)
        return super(CiDict, self).__contains__(key)

    def __delitem__(self, key):
        """ __delitem__ """
        key = self.Key(key)
        return super(CiDict, self).__delitem__(key)

    def __setitem__(self, key, value):
        """ __setitem__ """
        key = self.Key(key)
        super(CiDict, self).__setitem__(key, value)

    def __getitem__(self, key):
        """ __getitem__ """
        key = self.Key(key)
        return super(CiDict, self).__getitem__(key)

# =========================================================================


def indent_xml(elem, level=0, last=False):
    """ indent_xml():  <a> text </a> tail   """
    _tab = "\t"
    _tail = "\n" + (level * _tab)
    _text = _tail + _tab
    
    if elem:
        if not elem.text or not elem.text.strip(): elem.text = _text

        # recursion
        for el in elem[:-1]: indent_xml(el, level+1)

        # last item
        if level:
            for el in elem[-1:]: indent_xml(el, level, True)
        else:
            for el in elem[-1:]: indent_xml(el, 1, True)

        if not elem.tail or not elem.tail.strip(): elem.tail = "\n" if last else _tail

    else:
        if not elem.tail or not elem.tail.strip(): elem.tail = _tail

# =============================================================================


# =============================================================================
