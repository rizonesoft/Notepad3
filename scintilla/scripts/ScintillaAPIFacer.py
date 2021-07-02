#!/usr/bin/env python3
# ScintillaAPIFacer.py - regenerate the ScintillaTypes.h, and ScintillaMessages.h
# from the Scintilla.iface interface definition file.
# Implemented 2019 by Neil Hodgson neilh@scintilla.org
# Requires Python 3.6 or later

import Face
import FileGenerator

def HMessages(f):
	out = ["enum class Message {"]
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			if v["FeatureType"] in ["fun", "get", "set"]:
				out.append("\t" + name + " = " + v["Value"] + ",")
	out.append("};")
	return out

deadValues = [
	"INDIC_CONTAINER",
	"INDIC_IME",
	"INDIC_IME_MAX",
	"INDIC_MAX",
]

def HEnumerations(f):
	out = []
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			# Only want non-deprecated enumerations and lexers are not part of Scintilla API
			if v["FeatureType"] in ["enu"] and name != "Lexer":
				out.append("")
				prefixes = v["Value"].split()
				#out.append("enum class " + name + " {" + " // " + ",".join(prefixes))
				out.append("enum class " + name + " {")
				for valueName in f.order:
					prefixMatched = ""
					for p in prefixes:
						if valueName.startswith(p) and valueName not in deadValues:
							prefixMatched = p
					if prefixMatched:
						vEnum = f.features[valueName]
						valueNameNoPrefix = ""
						if valueName in f.aliases:
							valueNameNoPrefix = f.aliases[valueName]
						else:
							valueNameNoPrefix = valueName[len(prefixMatched):]
							if not valueNameNoPrefix:	# Removed whole name
								valueNameNoPrefix = valueName
							if valueNameNoPrefix.startswith("SC_"):
								valueNameNoPrefix = valueNameNoPrefix[len("SC_"):]
						pascalName = Face.PascalCase(valueNameNoPrefix)
						out.append("\t" + pascalName + " = " + vEnum["Value"] + ",")
				out.append("};")

	out.append("")
	out.append("enum class Notification {")
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			if v["FeatureType"] in ["evt"]:
				out.append("\t" + name + " = " + v["Value"] + ",")
	out.append("};")

	return out

def HConstants(f):
	# Constants not in an eumeration
	out = []
	allEnumPrefixes = [
		"SCE_", # Lexical styles
		"SCI_", # Message number allocation
		"SCEN_", # Notifications sent with WM_COMMAND
	]
	for _n, v in f.features.items():
		if v["Category"] != "Deprecated":
			# Only want non-deprecated enumerations and lexers are not part of Scintilla API
			if v["FeatureType"] in ["enu"]:
				allEnumPrefixes.extend(v["Value"].split())
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			# Only want non-deprecated enumerations and lexers are not part of Scintilla API
			if v["FeatureType"] in ["val"]:
				hasPrefix = False
				for prefix in allEnumPrefixes:
					if name.startswith(prefix):
						hasPrefix = True
				if not hasPrefix:
					if name.startswith("SC_"):
						name = name[3:]
					type = "int"
					if name == "INVALID_POSITION":
						type = "Position"
					out.append("constexpr " + type + " " + Face.PascalCase(name) + " = " + v["Value"] + ";")
	return out

def RegenerateAll(root):
	f = Face.Face()
	f.ReadFromFile(root + "include/Scintilla.iface")
	FileGenerator.Regenerate(root + "include/ScintillaMessages.h", "//", HMessages(f))
	FileGenerator.Regenerate(root + "include/ScintillaTypes.h", "//", HEnumerations(f), HConstants(f))

if __name__ == "__main__":
	RegenerateAll("../")
