# Null pointer dereference in libjpeg-turbo 3.1.0 `start_input_pass`

## Description

A denial-of-service vulnerability in libjpeg-turbo 3.1.0. When processing a crafted malformed JPEG file, the library only prints a warning `Premature end of JPEG file` but does not abort the decoding process. It continues to execute the full decompression pipeline initialization.

The malformed image contains a partial DQT table, an SOF0 header with 12-bit precision, and a truncated SOS segment missing its spectral selection parameters (Ss, Se, Ah/Al). As a result, the decoder fails to initialize the `start_input_pass` function pointer, leaving it as NULL. When `jpeg_start_decompress()` is subsequently called, it invokes `start_input_pass` through this null pointer, reading from address 0x000000000000 and causing a segfault.

This vulnerability can only cause a program crash. There is no risk of remote code execution, information disclosure, or privilege escalation.

## Affected version

libjpeg-turbo 3.1.0

## Reproduction

This repository includes `jpeg_fuzzer` (compiled with AddressSanitizer, calls `jpeg_read_header()` then `jpeg_start_decompress()`) and `libjpeg_turbo_null_deref_minimized.jpg` (91-byte truncated malformed JPEG).

```bash
ASAN_OPTIONS=detect_leaks=0 \
/home/jpeg_fuzzer \
/home/libjpeg_turbo_null_deref_minimized.jpg
```

### Expected output

```
Premature end of JPEG file
AddressSanitizer:DEADLYSIGNAL
=================================================================
==...==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000
The signal is caused by a READ memory access.
    #0 start_input_pass
    #1 jinit_master_decompress
    #2 jpeg_start_decompress
SUMMARY: AddressSanitizer: SEGV ... in start_input_pass
==...==ABORTING
```
