#ifndef MD5_H
#define MD5_H
/* GLOBAL.H - RSAREF types and constants
*/
/* PROTOTYPES should be set to one if and only if the compiler supports
function argument prototyping.
The following makes PROTOTYPES default to 0 if it has not already been defined with C compiler flags.
*/


#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif
/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;
/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;
/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;
/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
returns an empty list.
*/
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif



/* MD5.H - header file for MD5C.C
*/
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.
License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.
License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.
RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
*/
namespace aimy {
/* MD5 context. */
//the return buf must be delete by free()
char *Get_MD5_String(const void *buf,int buf_size);
 char* MY_our_MD5Data(void const* data, unsigned dataSize, char* outputDigest);
    // "outputDigest" must be either NULL (in which case this function returns a heap-allocated
    // buffer, which should be later delete[]d by the caller), or else it must point to
    // a (>=)33-byte buffer (which this function will also return).

 unsigned char* MY_our_MD5DataRaw(void const* data, unsigned dataSize,
                     unsigned char* outputDigest);
    // Like "ourMD5Data()", except that it returns the digest in 'raw' binary form, rather than
    // as an ASCII hex string.
    // "outputDigest" must be either NULL (in which case this function returns a heap-allocated
    // buffer, which should be later delete[]d by the caller), or else it must point to
    // a (>=)16-byte buffer (which this function will also return).
}

#endif // MD5_H
