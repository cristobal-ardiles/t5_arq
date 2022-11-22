/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include "newlib.h"

#define _LITE_EXIT 1

#=========================================================================
# crt0.S : Entry point for RISC-V user programs
#=========================================================================

  .text
  .global _start
  .type   _start, @function
_start:
  # Initialize global pointer
.option push
.option norelax
1:auipc gp, %pcrel_hi(__global_pointer$)
  addi  gp, gp, %pcrel_lo(1b)
.option pop

  # Init stack pointer
  li      sp, 2
  slli    sp, sp, 26  # 0x2000000

  # Clear the bss segment
  la      a0, _edata
  la      a2, _end
  sub     a2, a2, a0
  li      a1, 0
  call    memset
#ifdef _LITE_EXIT
  # Make reference to atexit weak to avoid unconditionally pulling in
  # support code.  Refer to comments in __atexit.c for more details.
  .weak   atexit
  la      a0, atexit
  beqz    a0, .Lweak_atexit
  .weak   __libc_fini_array
#endif

  la      a0, __libc_fini_array   # Register global termination functions
  call    atexit                  #  to be called upon exit
#ifdef _LITE_EXIT
.Lweak_atexit:
#endif
  call    __libc_init_array       # Run global initialization functions

# call    _ini_intr

  li      a0, 0                    # a0 = argc
  li      a1, 0                    # a1 = argv
  li      a2, 0                    # a2 = envp = NULL
  call    main
  call    stop
  tail    exit
  .size  _start, .-_start
