#!/bin/tcsh
#set echo
#
# this is a test script for np3encrypt.exe.  It encrypts every file in cwd and compares
# the decrypted result with the original. If all goes well it cleans up after itself,
# leaving the original files untouched.
#
foreach file (*)

if(-f "$file") then

echo $file

# plain file
np3encrypt ef "$file" "$file.enc" "$file"
np3encrypt df "$file.enc" "$file.out" "$file"
diff "$file" "$file.out"
if($status) then
 echo "decode file"
 break; 
 endif
rm "$file.out" "$file.enc"

# master file decrypt with file key
np3encrypt em "$file" "$file.enc" "$file key" "master $file"
np3encrypt df "$file.enc" "$file.out" "$file key"
diff "$file" "$file.out"
if($status) then
 echo "decode master using file"
 break; 
 endif
rm "$file.out" "$file.enc"

# master file decrypt with master key
np3encrypt em "$file" "$file.enc" "$file key" "master $file"
np3encrypt dm "$file.enc" "$file.out" "master $file"
diff "$file" "$file.out"
if($status) then
 echo "decode master using master"
 break; 
 endif
 
rm "$file.out" "$file.enc"

endif

end
