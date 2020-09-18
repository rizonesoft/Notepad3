#!/bin/tcsh
#set echo
#
# this is a test script for notepadcrypt.  It encrypts every file in cwd and compares
# the decrypted result with the original. If all goes well it cleans up after itself,
# leaving the original files untouched.
#
foreach file (*)

if(-f "$file") then

echo $file

# plain file
notepadcrypt ef "$file" "$file.enc" "$file"
notepadcrypt df "$file.enc" "$file.out" "$file"
diff "$file" "$file.out"
if($status) then
 echo "decode file"
 break; 
 endif
rm "$file.out" "$file.enc"

# master file decrypt with file key
notepadcrypt em "$file" "$file.enc" "$file key" "master $file"
notepadcrypt df "$file.enc" "$file.out" "$file key"
diff "$file" "$file.out"
if($status) then
 echo "decode master using file"
 break; 
 endif
rm "$file.out" "$file.enc"

# master file decrypt with master key
notepadcrypt em "$file" "$file.enc" "$file key" "master $file"
notepadcrypt dm "$file.enc" "$file.out" "master $file"
diff "$file" "$file.out"
if($status) then
 echo "decode master using master"
 break; 
 endif
 
rm "$file.out" "$file.enc"

endif

end
