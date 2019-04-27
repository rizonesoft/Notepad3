-- https://sqlite.org/lang.html
-- keywords
-- https://sqlite.org/lang_keywords.html
ABORT
ACTION
ADD
AFTER
ALL
ALTER
ANALYZE
AND
AS
ASC
ATTACH
AUTOINCREMENT
BEFORE
BEGIN
BETWEEN
BY
CASCADE
CASE
CAST
CHECK
COLLATE
COLUMN
COMMIT
CONFLICT
CONSTRAINT
CREATE
CROSS
CURRENT_DATE
CURRENT_TIME
CURRENT_TIMESTAMP
DATABASE
DEFAULT
DEFERRABLE
DEFERRED
DELETE
DESC
DETACH
DISTINCT
DROP
EACH
ELSE
END
ESCAPE
EXCEPT
EXCLUSIVE
EXISTS
EXPLAIN
FAIL
FOR
FOREIGN
FROM
FULL
GLOB
GROUP
HAVING
IF
IGNORE
IMMEDIATE
IN
INDEX
INDEXED
INITIALLY
INNER
INSERT
INSTEAD
INTERSECT
INTO
IS
ISNULL
JOIN
KEY
LEFT
LIKE
LIMIT
MATCH
NATURAL
NO
NOT
NOTNULL
NULL
OF
OFFSET
ON
OR
ORDER
OUTER
PLAN
PRAGMA
PRIMARY
QUERY
RAISE
RECURSIVE
REFERENCES
REGEXP
REINDEX
RELEASE
RENAME
REPLACE
RESTRICT
RIGHT
ROLLBACK
ROW
SAVEPOINT
SELECT
SET
TABLE
TEMP
TEMPORARY
THEN
TO
TRANSACTION
TRIGGER
UNION
UNIQUE
UPDATE
USING
VACUUM
VALUES
VIEW
VIRTUAL
WHEN
WHERE
WITH
WITHOUT

-- Datatypes
-- https://sqlite.org/datatype3.html
TEXT
NUMERIC
INTEGER
REAL
BLOB
-- Type Affinity
BIGINT
BOOLEAN
CHARACTER
CLOB
DATE
DATETIME
DECIMAL
DOUBLE
DOUBLE PRECISION
FLOAT
INT
INT2
INT8
INTEGER
MEDIUMINT
NATIVE CHARACTER
NCHAR
NUMERIC
NVARCHAR
REAL
SMALLINT
TEXT
TINYINT
UNSIGNED BIGINT
VARCHAR
VARYING CHARACTER
-- Collating Sequences
BINARY
NOCASE
RTRIM

-- aggregate functions
-- https://sqlite.org/lang_aggfunc.html
avg(X)
count(*)
count(X)
group_concat(X)
group_concat(X,Y)
max(X)
min(X)
sum(X)
total(X)
-- date and time functions
-- https://sqlite.org/lang_datefunc.html
date(timestring, modifier, modifier, ...)
datetime(timestring, modifier, modifier, ...)
julianday(timestring, modifier, modifier, ...)
strftime(format, timestring, modifier, modifier, ...)
time(timestring, modifier, modifier, ...)
-- core functions
-- https://sqlite.org/lang_corefunc.html
abs(X)
changes()
char(X1,X2,...,XN)
coalesce(X,Y,...)
glob(X,Y)
hex(X)
ifnull(X,Y)
instr(X,Y)
last_insert_rowid()
length(X)
like(X,Y)
like(X,Y,Z)
likelihood(X,Y)
likely(X)
load_extension(X)
load_extension(X,Y)
lower(X)
ltrim(X)
ltrim(X,Y)
max(X,Y,...)
min(X,Y,...)
nullif(X,Y)
printf(FORMAT,...)
quote(X)
random()
randomblob(N)
replace(X,Y,Z)
round(X)
round(X,Y)
rtrim(X)
rtrim(X,Y)
soundex(X)
sqlite_compileoption_get(N)
sqlite_compileoption_used(X)
sqlite_source_id()
sqlite_version()
substr(X,Y)
substr(X,Y,Z)
total_changes()
trim(X)
trim(X,Y)
typeof(X)
unicode(X)
unlikely(X)
upper(X)
zeroblob(N)
-- JSON1
-- https://www.sqlite.org/json1.html
json(json)
json_array(value1,value2,...)
json_array_length(json)
json_array_length(json,path)
json_extract(json,path,...)
json_insert(json,path,value,...)
json_object(label1,value1,...)
json_patch(json1,json2)
json_remove(json,path,...)
json_replace(json,path,value,...)
json_set(json,path,value,...)
json_type(json)
json_type(json,path)
json_valid(json)
json_quote(value)
json_group_array(value)
json_group_object(name,value)
json_each(json)
json_each(json,path)
json_tree(json)
json_tree(json,path)
