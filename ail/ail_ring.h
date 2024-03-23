// Simple Buffer
//
// LICENSE
/*
Copyright (c) 2024 Val Richter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef AIL_RING_H_
#define AIL_RING_H_

#ifndef AIL_TYPES_IMPL
#define AIL_TYPES_IMPL
#endif // AIL_TYPES_IMPL
#include "ail.h"

#ifndef AIL_RING_ASSERT
#define AIL_RING_ASSERT AIL_ASSERT
#endif

#ifndef AIL_RING_DEF
#ifdef  AIL_DEF
#define AIL_RING_DEF AIL_DEF
#else
#define AIL_RING_DEF
#endif // AIL_DEF
#endif // AIL_RING_DEF
#ifndef AIL_RING_DEF_INLINE
#ifdef  AIL_DEF_INLINE
#define AIL_RING_DEF_INLINE AIL_DEF_INLINE
#else
#define AIL_RING_DEF_INLINE static inline // AIL_DEF_INLINE
#endif // AIL_DEF_INLINE
#endif // AIL_RING_DEF_INLINE

// @Study whether we should use memcpy
// #ifndef AIL_RING_MEMCPY
// #ifdef AIL_MEMCPY
// #define AIL_RING_MEMCPY AIL_MEMCPY
// #elif
// #include <string.h>
// #define AIL_RING_MEMCPY(dst, src, len) memcpy(dst, src, len)
// #endif
// #endif

#ifndef AIL_RING_SIZE
#define AIL_RING_SIZE 128
#endif // AIL_RING_SIZE

AIL_STATIC_ASSERT(AIL_RING_SIZE <= UINT8_MAX); // @Note: Makes sure that using 'u8' for indexes works
typedef struct AIL_RingBuffer {
    u8 start;
    u8 end;
    u8 data[AIL_RING_SIZE];
} AIL_RingBuffer;

AIL_RING_DEF u8   ail_ring_len     (AIL_RingBuffer rb);
AIL_RING_DEF void ail_ring_pop     (AIL_RingBuffer *rb);
AIL_RING_DEF void ail_ring_popn    (AIL_RingBuffer *rb, u8 n);
AIL_RING_DEF u8   ail_ring_peek    (AIL_RingBuffer rb);
AIL_RING_DEF u8   ail_ring_peek_at (AIL_RingBuffer rb, u8 offset);
AIL_RING_DEF u16  ail_ring_peek2msb(AIL_RingBuffer rb);
AIL_RING_DEF u16  ail_ring_peek2lsb(AIL_RingBuffer rb);
AIL_RING_DEF u32  ail_ring_peek4msb(AIL_RingBuffer rb);
AIL_RING_DEF u32  ail_ring_peek4lsb(AIL_RingBuffer rb);
AIL_RING_DEF u64  ail_ring_peek8msb(AIL_RingBuffer rb);
AIL_RING_DEF u64  ail_ring_peek8lsb(AIL_RingBuffer rb);
AIL_RING_DEF void ail_ring_peekn   (AIL_RingBuffer rb, u8 n, u8 *buf);
AIL_RING_DEF u16  ail_ring_read2msb(AIL_RingBuffer *rb);
AIL_RING_DEF u16  ail_ring_read2lsb(AIL_RingBuffer *rb);
AIL_RING_DEF u32  ail_ring_read4msb(AIL_RingBuffer *rb);
AIL_RING_DEF u32  ail_ring_read4lsb(AIL_RingBuffer *rb);
AIL_RING_DEF u64  ail_ring_read8msb(AIL_RingBuffer *rb);
AIL_RING_DEF u64  ail_ring_read8lsb(AIL_RingBuffer *rb);
AIL_RING_DEF void ail_ring_write_at (AIL_RingBuffer *rb, u8 offset, u8 x); // @Note: Does not modify rb->end
AIL_RING_DEF void ail_ring_write1   (AIL_RingBuffer *rb, u8 x);
AIL_RING_DEF void ail_ring_readn    (AIL_RingBuffer *rb, u8 n, u8 *buf);
AIL_RING_DEF void ail_ring_write2msb(AIL_RingBuffer *rb, u16 x);
AIL_RING_DEF void ail_ring_write2lsb(AIL_RingBuffer *rb, u16 x);
AIL_RING_DEF void ail_ring_write4msb(AIL_RingBuffer *rb, u32 x);
AIL_RING_DEF void ail_ring_write4lsb(AIL_RingBuffer *rb, u32 x);
AIL_RING_DEF void ail_ring_write8msb(AIL_RingBuffer *rb, u64 x);
AIL_RING_DEF void ail_ring_write8lsb(AIL_RingBuffer *rb, u64 x);
AIL_RING_DEF void ail_ring_writen   (AIL_RingBuffer *rb, u8 n, u8 *buf);

#endif // AIL_RING_H_


#ifdef  AIL_RING_IMPL
#ifndef _AIL_RING_IMPL_GUARD_
#define _AIL_RING_IMPL_GUARD_

u8 ail_ring_len(AIL_RingBuffer rb)
{
    bool wrapped = rb.end < rb.start;
    return (!wrapped)*(rb.end - rb.start) + (wrapped)*(rb.end + AIL_RING_SIZE - rb.start);
}

// Pops first element or does nothing if buffer is empty
void ail_ring_pop(AIL_RingBuffer *rb)
{
    if (rb->end != rb->start) rb->start = (rb->start + 1) % AIL_RING_SIZE;
}

void ail_ring_popn(AIL_RingBuffer *rb, u8 n)
{
    if (ail_ring_len(*rb) < n) rb->start = rb->end = 0;
    else rb->start = (rb->start + n) % AIL_RING_SIZE;
}

// Returns the next byte or 0 if the buffer's length is 0
u8 ail_ring_peek(AIL_RingBuffer rb)
{
    return (rb.end != rb.start)*rb.data[rb.start];
}

u8 ail_ring_peek_at(AIL_RingBuffer rb, u8 offset)
{
    return (ail_ring_len(rb) > offset)*rb.data[(rb.start + offset) % AIL_RING_SIZE];
}

u16 ail_ring_peek2msb(AIL_RingBuffer rb)
{
	return ((u16)ail_ring_peek_at(rb, 0) << 8) | ((u16)ail_ring_peek_at(rb, 1));
}

u16 ail_ring_peek2lsb(AIL_RingBuffer rb)
{
	return ((u16)ail_ring_peek_at(rb, 1) << 8) | ((u16)ail_ring_peek_at(rb, 0));
}

u32 ail_ring_peek4msb(AIL_RingBuffer rb)
{
    return ((u32)ail_ring_peek_at(rb, 0) << 24) | ((u32)ail_ring_peek_at(rb, 1) << 16) | ((u32)ail_ring_peek_at(rb, 2) << 8) | ((u32)ail_ring_peek_at(rb, 3));
}

u32 ail_ring_peek4lsb(AIL_RingBuffer rb)
{
    return ((u32)ail_ring_peek_at(rb, 3) << 24) | ((u32)ail_ring_peek_at(rb, 2) << 16) | ((u32)ail_ring_peek_at(rb, 1) << 8) | ((u32)ail_ring_peek_at(rb, 0));
}

u64 ail_ring_peek8msb(AIL_RingBuffer rb)
{
    return ((u64)ail_ring_peek_at(rb, 0) << 7*8) | ((u64)ail_ring_peek_at(rb, 1) << 6*8) | ((u64)ail_ring_peek_at(rb, 2) << 5*8) | ((u64)ail_ring_peek_at(rb, 3) << 4*8) |
           ((u64)ail_ring_peek_at(rb, 4) << 3*8) | ((u64)ail_ring_peek_at(rb, 5) << 2*8) | ((u64)ail_ring_peek_at(rb, 6) << 1*8) | ((u64)ail_ring_peek_at(rb, 7) << 0*8);
}

u64 ail_ring_peek8lsb(AIL_RingBuffer rb)
{
    return ((u64)ail_ring_peek_at(rb, 7) << 7*8) | ((u64)ail_ring_peek_at(rb, 6) << 6*8) | ((u64)ail_ring_peek_at(rb, 5) << 5*8) | ((u64)ail_ring_peek_at(rb, 4) << 4*8) |
           ((u64)ail_ring_peek_at(rb, 3) << 3*8) | ((u64)ail_ring_peek_at(rb, 2) << 2*8) | ((u64)ail_ring_peek_at(rb, 1) << 1*8) | ((u64)ail_ring_peek_at(rb, 0) << 0*8);
}

void ail_ring_peekn(AIL_RingBuffer rb, u8 n, u8 *buf)
{
    // @TODO: Use AIL_RING_MEMCPY instead maybe?
    for (u8 i = 0, j = rb.start; i < n; i++, j = (j+1)%AIL_RING_SIZE) {
        buf[i] = rb.data[j];
		// @Safety @Performance
        // @Note: The more safe version would set the value to 0 if it's out of bounds
        // buf[i] = rb.data[j]*(j < rb.end || (rb.end < rb.start && rb.start <= j));
    }
}

u16 ail_ring_read2msb(AIL_RingBuffer *rb)
{
    u16 res = ail_ring_peek2msb(*rb);
    ail_ring_popn(rb, 2);
    return res;
}

u16 ail_ring_read2lsb(AIL_RingBuffer *rb)
{
    u16 res = ail_ring_peek2lsb(*rb);
    ail_ring_popn(rb, 2);
    return res;
}

u32 ail_ring_read4msb(AIL_RingBuffer *rb)
{
    u32 res = ail_ring_peek4msb(*rb);
    ail_ring_popn(rb, 4);
    return res;
}

u32 ail_ring_read4lsb(AIL_RingBuffer *rb)
{
    u32 res = ail_ring_peek4lsb(*rb);
    ail_ring_popn(rb, 4);
    return res;
}

u64 ail_ring_read8msb(AIL_RingBuffer *rb)
{
    u64 res = ail_ring_peek8msb(*rb);
    ail_ring_popn(rb, 8);
    return res;
}

u64 ail_ring_read8lsb(AIL_RingBuffer *rb)
{
    u64 res = ail_ring_peek8lsb(*rb);
    ail_ring_popn(rb, 8);
    return res;
}

void ail_ring_readn(AIL_RingBuffer *rb, u8 n, u8 *buf)
{
    ail_ring_peekn(*rb, n, buf);
    ail_ring_popn(rb, n);
}

void ail_ring_write_at(AIL_RingBuffer *rb, u8 offset, u8 x)
{
    rb->data[(rb->end + offset)%AIL_RING_SIZE] = x;
    AIL_RING_ASSERT((rb->end < rb->start) == (rb->end + offset < rb->start));
}

void ail_ring_write1(AIL_RingBuffer *rb, u8 x)
{
    ail_ring_write_at(rb, 0, x);
    rb->end = (rb->end + 1)%AIL_RING_SIZE;
}

void ail_ring_write2msb(AIL_RingBuffer *rb, u16 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 1*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 0*8));
    rb->end = (rb->end + 2)%AIL_RING_SIZE;
}

void ail_ring_write2lsb(AIL_RingBuffer *rb, u16 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 0*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 1*8));
    rb->end = (rb->end + 2)%AIL_RING_SIZE;
}

void ail_ring_write4msb(AIL_RingBuffer *rb, u32 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 3*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 2*8));
    ail_ring_write_at(rb, 2, (u8)(x >> 1*8));
    ail_ring_write_at(rb, 3, (u8)(x >> 0*8));
    rb->end = (rb->end + 4)%AIL_RING_SIZE;
}

void ail_ring_write4lsb(AIL_RingBuffer *rb, u32 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 0*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 1*8));
    ail_ring_write_at(rb, 2, (u8)(x >> 2*8));
    ail_ring_write_at(rb, 3, (u8)(x >> 3*8));
    rb->end = (rb->end + 4)%AIL_RING_SIZE;
}

void ail_ring_write8msb(AIL_RingBuffer *rb, u64 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 7*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 6*8));
    ail_ring_write_at(rb, 2, (u8)(x >> 5*8));
    ail_ring_write_at(rb, 3, (u8)(x >> 4*8));
    ail_ring_write_at(rb, 4, (u8)(x >> 3*8));
    ail_ring_write_at(rb, 5, (u8)(x >> 2*8));
    ail_ring_write_at(rb, 6, (u8)(x >> 1*8));
    ail_ring_write_at(rb, 7, (u8)(x >> 0*8));
    rb->end = (rb->end + 8)%AIL_RING_SIZE;
}

void ail_ring_write8lsb(AIL_RingBuffer *rb, u64 x)
{
    ail_ring_write_at(rb, 0, (u8)(x >> 0*8));
    ail_ring_write_at(rb, 1, (u8)(x >> 1*8));
    ail_ring_write_at(rb, 2, (u8)(x >> 2*8));
    ail_ring_write_at(rb, 3, (u8)(x >> 3*8));
    ail_ring_write_at(rb, 4, (u8)(x >> 4*8));
    ail_ring_write_at(rb, 5, (u8)(x >> 5*8));
    ail_ring_write_at(rb, 6, (u8)(x >> 6*8));
    ail_ring_write_at(rb, 7, (u8)(x >> 7*8));
    rb->end = (rb->end + 8)%AIL_RING_SIZE;
}

void ail_ring_writen(AIL_RingBuffer *rb, u8 n, u8 *buf)
{
    for (u8 i = 0; i < n; i++, rb->end = (rb->end+1)%AIL_RING_SIZE) {
        rb->data[rb->end] = buf[i];

        u8 tmp = (rb->end+1)%AIL_RING_SIZE;
        AIL_RING_ASSERT(tmp != rb->start);
    }
}


#endif // _AIL_RING_IMPL_GUARD_
#endif // AIL_RING_IMPL