#ifndef _STRHEAP_H_
#define _STRHEAP_H_

/*
 * Copyright (c) 2023, University of Oregon
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the University of Oregon nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * functions for a ref-counted string heap
 *
 * for use in programs that store multiple copies of the same string on the
 * heap
 *
 * a unique string will only be stored once on the heap, and a reference
 * count is kept of the number of times that string has been str_malloc()ed.
 *
 * each str_free() causes the associated reference count to be decremented.
 *
 * when the reference count reaches 0, the string is purged from the heap
 */

#include <stdbool.h>

/*
 * "duplicate" `string' on the string heap
 *
 * returns the address of the heap allocated string or NULL if malloc() errors
 *
 * increments the reference count for the string
 */
char *str_malloc(char *string);

/*
 * "free" `string' on the string heap
 *
 * returns true if free'd, or false if the string was not present on the string
 * heap
 *
 * if the decremented reference count has reached 0, the string is purged from
 * the heap
 */
bool str_free(char *string);

#endif /* _STRHEAP_H_ */
