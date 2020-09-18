// encoding: UTF-8
// ced.cpp : CompactEncodingDetection - Defines the entry point for the console application.
//

#include "targetver.h"

#include <stdio.h>          // fopen/fclose/fprintf/printf/puts
#include <stdlib.h>         // exit
#include <tchar.h>

#include "compact_enc_det/compact_enc_det.h"

// max size of needed bytes to read in for reliable file encoding detection
#define CED_BUFFER_SIZE 200000

bool verbose = false;

// ============================================================================

Encoding Encoding_Analyze(const char* const text, const size_t len, bool* pIsReliable)
{
  int bytes_consumed;

  Encoding encoding = CompactEncDet::DetectEncoding(
    text, static_cast<int>(len),
    nullptr, nullptr, nullptr,
    UNKNOWN_ENCODING,
    UNKNOWN_LANGUAGE,
    CompactEncDet::WEB_CORPUS,
    false,
    &bytes_consumed,
    pIsReliable);

  return encoding;
}
// ============================================================================


// ============================================================================
//
// Main
//
static void usage()
{
  //fprintf(stderr, "Usage: ced [-v] <filename> \n");
  fprintf(stderr, "Usage: ced <filename> \n");
}


// ============================================================================
//
// Main
//
int __cdecl main(int argc, char* argv[])
{
  const char* filename;
  if (argc == 3 && strcmp(argv[1], "-v") == 0) {
    verbose = true;
  }
  if (argc != 2 && !verbose) {
    usage();
    exit(EXIT_FAILURE);
  }
  if (verbose) {
    filename = argv[2];
  }
  else {
    filename = argv[1];
  }

  FILE* fp = nullptr;
  errno_t _errno = fopen_s(&fp, filename, "rb");
  if (fp == nullptr) {
    char err_buf[512];
    strerror_s(err_buf, _errno);
    fprintf(stderr, "\nCannot open file '%s': %s !\n", filename, err_buf);
    exit(EXIT_FAILURE);
  }

  char buffer[CED_BUFFER_SIZE];
  
  size_t len = fread(buffer, 1, sizeof buffer, fp);
  fclose(fp);

  bool is_reliable = false;
  Encoding enc = Encoding_Analyze(buffer, len, &is_reliable);

  const char* rel = (is_reliable ? "reliable" : "NOT reliable");

  printf("\nEncoding of '%s' is: %s (%s) [%s].\n\n", filename, 
         EncodingName(enc), MimeEncodingName(enc), rel);

  return 0;
}
// ============================================================================
