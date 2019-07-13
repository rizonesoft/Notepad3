' IsMember2.vbs
' VBScript program demonstrating the use of Function IsMember.
'
' ----------------------------------------------------------------------
' Copyright (c) 2002-2010 Richard L. Mueller
' Hilltop Lab web site - http://www.rlmueller.net
' Version 1.0 - November 10, 2002
' Version 1.1 - February 19, 2003 - Standardize Hungarian notation.
' Version 1.2 - July 31, 2007 - Escape any "/" characters in group DN's.
' Version 1.3 - November 6, 2010 - No need to set objects to Nothing.
' An efficient IsMember function to test group membership for a single
' user or computer. The function reveals membership in nested groups. It
' requires that the user or computer object be bound with the LDAP
' provider. It will not reveal membership in the primary group.
'
' You have a royalty-free right to use, modify, reproduce, and
' distribute this script file in any way you find useful, provided that
' you agree that the copyright owner above has no warranty, obligations,
' or liability for such use.

Option Explicit

Rem -- If Execute per error, then exit immediatly !!!
wscript.quit

Dim objGroupList, objADObject, strGroup

' Bind to the user object with the LDAP provider.
Set objADObject = _
    GetObject("LDAP://cn=Testuser,ou=Sales,dc=MyDomain,dc=com")

strGroup = "Sales"
If (IsMember(strGroup) = True) Then
    Wscript.Echo "User " & objADObject.name _
        & " is a member of group " & strGroup
Else
    Wscript.Echo "User " & objADObject.name _
        & " is NOT a member of group " & strGroup
End If

strGroup = "Engineering"
If (IsMember(strGroup) = True) Then
    Wscript.Echo "User " & objADObject.name _
        & " is a member of group " & strGroup
Else
    Wscript.Echo "User " & objADObject.name _
        & " is NOT a member of group " & strGroup
End If

Function IsMember(ByVal strGroup)
    ' Function to test for group membership.
    ' strGroup is the NT name (sAMAccountName) of the group to test.
    ' objGroupList is a dictionary object, with global scope.
    ' Returns True if the user or computer is a member of the group.

    If (IsEmpty(objGroupList) = True) Then
        Set objGroupList = CreateObject("Scripting.Dictionary")
        Call LoadGroups(objADObject)
    End If
    IsMember = objGroupList.Exists(strGroup)
End Function

Sub LoadGroups(ByVal objADObject)
    ' Recursive subroutine to populate dictionary object with group
    ' memberships. When this subroutine is first called by Function
    ' IsMember, objADObject is the user or computer object. On recursive calls
    ' objADObject will be a group object. For each group in the MemberOf
    ' collection, first check to see if the group is already in the
    ' dictionary object. If it is not, add the group to the dictionary
    ' object and recursively call this subroutine again to enumerate any
    ' groups the group might be a member of (nested groups). It is necessary
    ' to first check if the group is already in the dictionary object to
    ' prevent an infinite loop if the group nesting is "circular".

    Dim colstrGroups, objGroup, j
    objGroupList.CompareMode = vbTextCompare
    colstrGroups = objADObject.memberOf
    If (IsEmpty(colstrGroups) = True) Then
        Exit Sub
    End If
    If (TypeName(colstrGroups) = "String") Then
        ' Escape any forward slash characters, "/", with the backslash
        ' escape character. All other characters that should be escaped are.
        colstrGroups = Replace(colstrGroups, "/", "\/")
        Set objGroup = GetObject("LDAP://" & colstrGroups)
        If (objGroupList.Exists(objGroup.sAMAccountName) = False) Then
            objGroupList.Add objGroup.sAMAccountName, True
            Call LoadGroups(objGroup)
        End If
        Exit Sub
    End If
    For j = 0 To UBound(colstrGroups)
        ' Escape any forward slash characters, "/", with the backslash
        ' escape character. All other characters that should be escaped are.
        colstrGroups(j) = Replace(colstrGroups(j), "/", "\/")
        Set objGroup = GetObject("LDAP://" & colstrGroups(j))
        If (objGroupList.Exists(objGroup.sAMAccountName) = False) Then
            objGroupList.Add objGroup.sAMAccountName, True
            Call LoadGroups(objGroup)
        End If
    Next
End Sub
