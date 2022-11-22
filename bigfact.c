#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>

#include "term.h"

#define BigIntWordSize 4
typedef uint32_t BigInt_t;
typedef uint64_t BigInt_tmp_t;

typedef struct {
  int n;
  BigInt_t *bits;
} BigNum;

static BigNum *smallNum(BigInt_t x);
static void freeBigNum(BigNum *bx);
static BigNum *bigMul(BigNum *bx, BigNum *by);
static BigNum *seqArrayProd(int a[], int i, int j);

/* Tokens returned by BigInt_cmp() for value comparison */
enum { SMALLER = -1, EQUAL = 0, LARGER = 1 };

/* In/out functions */ 
void BigInt_zero(size_t NumWords, BigInt_t * BigInt);
void BigInt_from_int(size_t NumWords, BigInt_t * BigInt, BigInt_tmp_t Integer);
int  BigInt_to_int(size_t NumWords, BigInt_t * BigInt);
void BigInt_from_string(size_t NumWords, BigInt_t * BigInt, char * Str); /* From decimal string */
void BigInt_from_hex_string(size_t NumWords, BigInt_t * BigInt, char * Str); /* From hex string */
void BigInt_to_hex_string(size_t NumWords, BigInt_t * BigInt, char * Str); /* To hex string */
void BigInt_copy(size_t NumWords, BigInt_t * dst, BigInt_t * src);
void BigInt_copy_dif(size_t DstNumWords, BigInt_t * Dst, size_t SrcNumWords, BigInt_t * Src); /* Copy different sized ones */

/* Basic arithmetic operations: */
void BigInt_add(size_t ANumWords, BigInt_t * A, size_t BNumWords, BigInt_t * B, size_t OutNumWords, BigInt_t * Out); /* Out = A + B */
void BigInt_sub(size_t ANumWords, BigInt_t * A, size_t BNumWords, BigInt_t * B, size_t OutNumWords, BigInt_t * Out); /* Out = A - B */
void BigInt_mul(size_t XWords, BigInt_t * X, size_t YWords, BigInt_t * Y, size_t OutWords, BigInt_t * Out); /* Karatsuba multiplication */
void BigInt_mul_basic(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Out = A * B, old method, faster for small numbers */
void BigInt_div(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Out = A / B */
void BigInt_mod(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Out = A % B */
void BigInt_divmod(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * C, BigInt_t * D); /* C = A/B, D = A%B */

/* Bitwise operations: */
void BigInt_and(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Out = A & B */
void BigInt_or(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out);  /* Out = A | B */
void BigInt_xor(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Out = A ^ B */
void BigInt_lshift(size_t NumWords, BigInt_t * B, int nbits); /* B = A << nbits */
void BigInt_rshift(size_t NumWords, BigInt_t * B, int nbits); /* B = A >> nbits */

/* Special operators and comparison */
int  BigInt_cmp(size_t NumWords, BigInt_t * A, BigInt_t * B); /* Compare: returns LARGER, EQUAL or SMALLER */
int  BigInt_is_zero(size_t NumWords, BigInt_t * BigInt); /* For comparison with zero */
void BigInt_inc(size_t NumWords, BigInt_t * BigInt); /* Increment: add one to BigInt */
void BigInt_dec(size_t NumWords, BigInt_t * BigInt); /* Decrement: subtract one from BigInt */
void BigInt_pow(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out); /* Calculate A^B -- e.g. 2^10 => 1024 */
void BigInt_isqrt(size_t NumWords, BigInt_t * A, BigInt_t * B); /* Integer square root -- e.g. isqrt(5) => 2*/
size_t BigInt_truncate(size_t BigIntWords, BigInt_t * BigInt); /* How many digits are actually needed */

// ----------------------------------------------------
// Funciones para trabajar con big numbers

static BigNum *smallNum(BigInt_t x) {
  BigNum *bx= malloc(sizeof(BigNum));
  bx->n= 1;
  bx->bits= malloc(sizeof(BigInt_t));
  bx->bits[0]= x;
  return bx;
}

static void freeBigNum(BigNum *bx) {
  free(bx->bits);
  free(bx);
}

static BigNum *newBigNum(int n, BigInt_t *bits) {
  BigNum *bx= malloc(sizeof(BigNum));
  int nadj= n;
  while (nadj>0 && bits[nadj-1]==0) {
    nadj--;
  }
  bx->n= nadj;
  if (n==nadj)
    bx->bits= bits;
  else {
    BigInt_t *bitsadj= malloc(nadj*sizeof(BigInt_t));
    for (int i= 0; i<nadj; i++) {
      bitsadj[i]= bits[i];
    }
    bx->bits= bitsadj;
    free(bits);
  }
  return bx;
}

int bigCmp(BigNum *bx, BigNum *by) {
  if (bx->n!=by->n)
    return bx->n - by->n;
  else
    return BigInt_cmp(bx->n, bx->bits, by->bits);
}

#if 1

// This uses the fast Karatsuba multiplication algorithm.  
// Below is my own more intuitive but slow algorithm.
// If you want to test how slow is, change the #if 1 by #if 0
// Compare with time ./test-prod 100000 for both algorithms.
static BigNum *bigMul(BigNum *bx, BigNum *by) {
  int n= bx->n+by->n;
  BigInt_t *bits= malloc(n*sizeof(BigInt_t));
  BigInt_mul(bx->n, bx->bits, by->n, by->bits, n, bits);

  return newBigNum(n, bits);
}

#else

static BigNum *bigMul(BigNum *bx, BigNum *by) {
  int n= bx->n+by->n;
  BigInt_t *bits= malloc(n*sizeof(BigInt_t));
  for (int i= 0; i<n; i++)
    bits[i]= 0;
  BigInt_t carry= 0;
  for (int i= 0; i<by->n; i++) {
    int pos= i;
    for (int j= 0; j<bx->n; j++) {
      BigInt_tmp_t prod= (BigInt_tmp_t)bits[pos] + (BigInt_tmp_t)carry;
      prod += (BigInt_tmp_t)bx->bits[j] * (BigInt_tmp_t)by->bits[i];
      // prod += (BigInt_tmp_t)bx->bits[j] * by->bits[i];
      bits[pos]= prod;
      carry= prod>>(8*sizeof(BigInt_t));
      pos++;
    }
    if (pos<n) {
      bits[pos]= carry;
      carry= 0;
    }
  }
  if (carry!=0) {
    fprintf(stderr, "Asercion fallida: ultimo carry no fue 0\n");
    exit(1);
  }
  
  return newBigNum(n, bits);
}

#endif

// ----------------------------------------------------
// Version secuencial del producto de los elementos de
// un arreglo

static int verbose= 0;

static BigNum *recArrayProd(int a[], int i, int j) {
  if (i>j) {
    fprintf(stderr, "Asercion fallida: i > j\n");
    exit(1);
  }
  if (i==j) {
    return smallNum(a[i]);
  }
  else {
    int h= (i+j)/2;
    BigNum *left= recArrayProd(a, i, h);
    BigNum *right= recArrayProd(a, h+1, j);
    BigNum *prod= bigMul(left, right);
    freeBigNum(left);
    freeBigNum(right);
    return prod;
  }
}

static BigNum *seqArrayProd(int a[], int i, int j) {
  if (verbose) {
    printf("Llamada secuencial con i=%d j=%d\n", i, j);
    fflush(stdout);
  }
  return recArrayProd(a, i, j);
}

#if 0
// ----------------------------------------------------
// Calculo del factorial

// This iterative algorithm to compute factorial is simple and intuitive,
// but it is slow for big numbers
static BigNum *slowBigFact(int n) {
  BigNum *bp= smallNum(1);
  for (int i= 1; i<=n; i++) {
    BigNum *bi= smallNum(i);
    BigNum *newbp= bigMul(bp, bi);
    freeBigNum(bp);
    freeBigNum(bi);
    bp= newbp;
  }
  return bp;
}
#endif

// This is the recursive divide and conquer algorithm.  It happens to
// be way faster than the iterative algorithm.  Why?
static BigNum *fastBigFact(int n) {
  if (n==0)
    return smallNum(0);

  int *a= malloc(n*sizeof(int));

  // Fill a with 1, 2, 3, ..., n
  for (int i= 0; i<n; i++) {
    a[i]= i+1;
  }

#if 1
  // Filling a with 1, 2, 3, ..., n would work but the product of the first
  // half of the array would be much smaller than the second half,
  // making an unbalanced size of big numbers.
  // So the the array is filled with a permutation of 1, 2, 3, ..., n
  // to mix randomly small numbers with big numbers.
  // If you want to experiment without the permutation, change the
  // the #if 1 by #if 0
  // Try with ./test-prod 100000. It does segmentation fault because
  // of stack overflow.

  // Do a random permutation to balance the size of the numbers
  for (int i= 0; i<n; i++) {
    int r= random()%(n-i) + i;
    int tmp= a[i];
    a[i]= a[r];
    a[r]= tmp;
  }
#endif

  // Compute big product of array numbers
  BigNum *bf= seqArrayProd(a, 0, n-1);    // sequential
  free(a);
  return bf;
}

// ----------------------------------------------------
// Conversion de un big number a un string hexadecimal
// (convertir a decimal tomaria demasiado tiempo)

static char *bigNum2HexString(BigNum *bx) {
  char *str= malloc(bx->n*sizeof(BigInt_t)*2+1);
  char *res= str;
  int imax= sizeof(BigInt_t)*2;
  sprintf(str, "%lx", bx->bits[bx->n-1]);
  str += strlen(str);

  for (int k= bx->n-2; k>=0; k--) {
    BigInt_t w= bx->bits[k];
    for (int i= 1; i<=imax; i++) {
      int hex= w & 0x0f;
      str[imax-i]= hex>=10 ? 'a'+hex-10 : '0'+hex;
      w >>= 4;
    }
    str += imax;
  }
  *str= 0;
  return res;
}

int main(int argc, char *argv[]) {
  for (;;) {
    char lin[81];
    showStr("? ");
    readLine(lin, 80);
    int n= atoi(lin);
    showStr("Computing big factorial of ");
    showInt(n);
    showChar('\n');
    BigNum *bf= fastBigFact(n);
    showStr("size in int words= ");
    showInt(bf->n);
    showChar('\n');
    char *bigstr= bigNum2HexString(bf);
    showStr("Factorial in hexadecimal notation is\n");
    showStr(bigstr);
    showChar('\n');
    freeBigNum(bf);
    free(bigstr);
  }
}

// https://github.com/ilia3101/Big-Integer-C

/* Printing format strings */
#ifndef BigIntWordSize
    #error Must define BigIntWordSize to be 1, 2, 4
#elif (BigIntWordSize == 1)
    /* Max value of integer type */
    #define MAX_VAL ((BigInt_tmp_t)0xFF)
#elif (BigIntWordSize == 2)
    #define MAX_VAL ((BigInt_tmp_t)0xFFFF)
#elif (BigIntWordSize == 4)
    #define MAX_VAL ((BigInt_tmp_t)0xFFFFFFFF)
#elif (BigIntWordSize == 8)
    #define MAX_VAL ((BigInt_tmp_t)0xFFFFFFFFFFFFFFFF)
#endif

/* Bad macros */
#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

/* Functions for shifting number in-place. */
static void _lshift_one_bit(size_t NumWords, BigInt_t * A);
static void _rshift_one_bit(size_t NumWords, BigInt_t * A);
static void _lshift_word(size_t NumWords, BigInt_t * A, int nwords);
static void _rshift_word(size_t NumWords, BigInt_t * A, int nwords);

/* Endianness issue if machine is not little-endian? */
#ifdef BigIntWordSize
#if (BigIntWordSize == 1)
#define BigInt_FROM_INT(BigInt, Integer) { \
    ((BigInt_t *)(void *)BigInt)[0] = (((BigInt_tmp_t)Integer) & 0x000000ff); \
    ((BigInt_t *)(void *)BigInt)[1] = (((BigInt_tmp_t)Integer) & 0x0000ff00) >> 8; \
    ((BigInt_t *)(void *)BigInt)[2] = (((BigInt_tmp_t)Integer) & 0x00ff0000) >> 16; \
    ((BigInt_t *)(void *)BigInt)[3] = (((BigInt_tmp_t)Integer) & 0xff000000) >> 24; \
}
#elif (BigIntWordSize == 2)
#define BigInt_FROM_INT(BigInt, Integer) { \
    ((BigInt_t *)(void *)BigInt)[0] = (((BigInt_tmp_t)Integer) & 0x0000ffff); \
    ((BigInt_t *)(void *)BigInt)[1] = (((BigInt_tmp_t)Integer) & 0xffff0000) >> 16; \
}
#elif (BigIntWordSize == 4)
#define BigInt_FROM_INT(BigInt, Integer) { \
    ((BigInt_t *)(void *)BigInt)[0] = ((BigInt_tmp_t)Integer); \
    ((BigInt_t *)(void *)BigInt)[1] = ((BigInt_tmp_t)Integer) >> ((BigInt_tmp_t)32); \
}
#elif (BigIntWordSize == 8)
#define BigInt_FROM_INT(BigInt, Integer) { \
    ((BigInt_t *)(void *)BigInt)[0] = ((BigInt_tmp_t)Integer); \
    ((BigInt_t *)(void *)BigInt)[1] = ((BigInt_tmp_t)Integer) >> ((BigInt_tmp_t)64); \
}
#endif
#endif

/* Public / Exported functions. */
void BigInt_zero(size_t NumWords, BigInt_t * BigInt)
{
    for (size_t i = 0; i < NumWords; ++i) {
        BigInt[i] = 0;
    }
}

void BigInt_from_int(size_t NumWords, BigInt_t * BigInt, BigInt_tmp_t Integer)
{
    BigInt_zero(NumWords, BigInt);
    BigInt_FROM_INT(BigInt, Integer);
}

int BigInt_to_int(size_t NumWords, BigInt_t * BigInt)
{
    int ret = 0;

/* Endianness issue if machine is not little-endian? */
#if (BigIntWordSize == 1)
    ret += BigInt[0];
    ret += BigInt[1] << 8;
    ret += BigInt[2] << 16;
    ret += BigInt[3] << 24;
#elif (BigIntWordSize == 2)
    ret += BigInt[0];
    ret += BigInt[1] << 16;
#elif (BigIntWordSize == 4)
    ret += BigInt[0];
#elif (BigIntWordSize == 8)
    ret += BigInt[0];
#endif

    return ret;
}

size_t BigInt_truncate(size_t NumWords, BigInt_t * BigInt)
{
    if (NumWords==0)
      return 0;
    do {
      --NumWords;
    } while (NumWords>0 && BigInt[NumWords] == 0);
    return ++NumWords;
}

void BigInt_from_string(size_t NumWords, BigInt_t * BigInt, char * str)
{
    BigInt_zero(NumWords, BigInt);

    BigInt_t * temp = alloca(NumWords*BigIntWordSize);
    BigInt_t digit;
    BigInt_t ten = 10;

    while (*str != 0)
    {
        BigInt_mul(NumWords, BigInt, 1, &ten, NumWords, temp);

        digit = (*(str++)-'0');

        if (digit != 0)
            BigInt_add(NumWords, temp, 1, &digit, NumWords, BigInt);
        else
            BigInt_copy(NumWords, BigInt, temp);
    }
}

static BigInt_t hex_to_word(char * Text, int Length)
{
    BigInt_t word = 0;
    for (int i = 0; i < Length; ++i)
    {
        char character = Text[i];
        word <<= 4;
        if (character >= '0' && character <= '9')
            word += character - '0';
        else if (character <= 'F' && character >= 'A')
            word += character - 'A' + 10;
        else if (character <= 'f' && character >= 'a')
            word += character - 'a' + 10;
    }
    return word;
}

void BigInt_from_hex_string(size_t NumWords, BigInt_t * BigInt, char * Str)
{
    BigInt_zero(NumWords, BigInt);
    size_t length = strlen(Str);

    /* whole Words in this string */
    size_t num_words = length / (BigIntWordSize*2);
    if (num_words * (BigIntWordSize*2) < length) ++num_words; /* round up */

    char * string_word = Str + length;

    for (size_t i = 0; i < num_words; ++i)
    {
        /* How many characters should be read from the string */
        size_t hex_length = MIN(BigIntWordSize*2, string_word-Str);
        string_word -= (BigIntWordSize*2);
        BigInt[i] = hex_to_word(string_word, hex_length);
    }
}

void BigInt_to_hex_string(size_t NumWords, BigInt_t * BigInt, char * Str)
{
    NumWords = BigInt_truncate(NumWords, BigInt);

    size_t str_index = 0;

    for (int_fast32_t d = NumWords-1; d >= 0; --d)
    {
        BigInt_t word = BigInt[d];
        for (int BigInt = 0; BigInt < BigIntWordSize*2; ++BigInt) {
            uint8_t nibble = (word >> (BigInt_t)(BigInt*4)) & 0x0F;
            char hexchar = (nibble <= 9) ? '0' + nibble : 'a' + nibble-10;
            Str[str_index+BigIntWordSize*2-1-BigInt] = hexchar;
        }
        str_index += BigIntWordSize*2;
    }

    Str[str_index] = 0;
}

void BigInt_dec(size_t NumWords, BigInt_t * BigInt)
{
    BigInt_t tmp; /* copy of BigInt */
    BigInt_t res;

    for (size_t i = 0; i < NumWords; ++i) {
        tmp = BigInt[i];
        res = tmp - 1;
        BigInt[i] = res;

        if (!(res > tmp)) {
            break;
        }
    }
}

void BigInt_inc(size_t NumWords, BigInt_t * BigInt)
{
    BigInt_t res;
    BigInt_tmp_t tmp; /* copy of BigInt */

    for (size_t i = 0; i < NumWords; ++i) {
        tmp = BigInt[i];
        res = tmp + 1;
        BigInt[i] = res;

        if (res > tmp) {
            break;
        }
    }
}

void BigInt_add(size_t AWords, BigInt_t * A, size_t BWords, BigInt_t * B, size_t Out_NumWords, BigInt_t * Out)
{
    /* Make it so that A will be smaller than B */
    if (AWords > BWords)
    {
        size_t temp1 = BWords;
        BWords = AWords;
        AWords = temp1;
        BigInt_t * temp2 = B;
        B = A;
        A = temp2;
    }

    int loop_to = 0;
    size_t loop1 = 0;
    size_t loop2 = 0;
    size_t loop3 = 0;

    if (Out_NumWords <= AWords) {
        loop_to = 1;
        loop1 = Out_NumWords;
    }
    else if (Out_NumWords <= BWords) {
        loop_to = 2;
        loop1 = AWords;
        loop2 = Out_NumWords;
    }
    else {
        loop_to = 3;
        loop1 = AWords;
        loop2 = BWords;
        loop3 = Out_NumWords;
    }

    int carry = 0;
    BigInt_tmp_t tmp;
    size_t i;

    for (i = 0; i < loop1; ++i)
    {
        tmp = (BigInt_tmp_t)A[i] + B[i] + carry;
        carry = (tmp > MAX_VAL);
        Out[i] = (tmp & MAX_VAL);
    }

    if (loop_to == 1) return;

    for (; i < loop2; ++i)
    {
        tmp = (BigInt_tmp_t)B[i] + 0 + carry;
        carry = (tmp > MAX_VAL);
        Out[i] = (tmp & MAX_VAL);
    }

    if (loop_to == 2) return;

    /* Do the carry, then fill the rest with zeros */
    Out[i++] = carry;
    for (; i < loop3; ++i) Out[i] = 0;
}

void BigInt_sub(size_t AWords, BigInt_t * A, size_t BWords, BigInt_t * B, size_t Out_NumWords, BigInt_t * Out)
{
    int loop_to = 0;
    size_t loop1 = 0;
    size_t loop2 = 0;
    size_t loop3 = 0;

    if (Out_NumWords <= MIN(AWords, BWords))
    {
        loop_to = 1;
        loop1 = MIN(AWords, BWords);
    }
    else if (Out_NumWords <= MAX(AWords, BWords))
    {
        loop_to = 2;
        loop1 = MIN(AWords, BWords);
        loop2 = Out_NumWords;
    }
    else {
        loop_to = 3;
        loop1 = AWords;
        loop2 = BWords;
        loop3 = Out_NumWords;
    }

    BigInt_tmp_t res;
    BigInt_tmp_t tmp1;
    BigInt_tmp_t tmp2;
    int borrow = 0;
    size_t i;

    for (i = 0; i < loop1; ++i) {
        tmp1 = (BigInt_tmp_t)A[i] + (MAX_VAL + 1); /* + number_base */
        tmp2 = (BigInt_tmp_t)B[i] + borrow;
        ;
        res = (tmp1 - tmp2);
        Out[i] = (BigInt_t)(res & MAX_VAL); /* "modulo number_base" == "%
            (number_base - 1)" if number_base is 2^N */
        borrow = (res <= MAX_VAL);
    }

    if (loop_to == 1) return;

    if (AWords > BWords)
    {
        for (; i < loop2; ++i) {
            tmp1 = (BigInt_tmp_t)A[i] + (MAX_VAL + 1);
            tmp2 =  borrow;
            res = (tmp1 - tmp2);
            Out[i] = (BigInt_t)(res & MAX_VAL);
            borrow = (res <= MAX_VAL);
        }
    }
    else
    {
        for (; i < loop2; ++i) {
            tmp1 = (BigInt_tmp_t)MAX_VAL + 1;
            tmp2 = (BigInt_tmp_t)B[i] + borrow;
            res = (tmp1 - tmp2);
            Out[i] = (BigInt_t)(res & MAX_VAL);
            borrow = (res <= MAX_VAL);
        }
    }

    if (loop_to == 2) return;

    for (; i < loop3; ++i) {
        tmp1 = (BigInt_tmp_t)0 + (MAX_VAL + 1);
        tmp2 = (BigInt_tmp_t)0 + borrow;
        res = (tmp1 - tmp2);
        Out[i] = (BigInt_t)(res & MAX_VAL);
        borrow = (res <= MAX_VAL);
    }
}

void BigInt_mul_basic(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    BigInt_t * row = alloca(NumWords*BigIntWordSize);
    BigInt_t * tmp = alloca(NumWords*BigIntWordSize);
    size_t i, j;

    BigInt_zero(NumWords, Out);

    for (i = 0; i < NumWords; ++i) {
        BigInt_zero(NumWords, row);

        for (j = 0; j < NumWords; ++j) {
            if (i + j < NumWords) {
                BigInt_zero(NumWords, tmp);
                BigInt_tmp_t intermediate = ((BigInt_tmp_t)A[i] * (BigInt_tmp_t)B[j]);
                BigInt_from_int(NumWords, tmp, intermediate);
                _lshift_word(NumWords, tmp, i + j);
                BigInt_add(NumWords, tmp, NumWords, row, NumWords, row);
            }
        }
        BigInt_add(NumWords, Out, NumWords, row, NumWords, Out);
    }
}

/* Cool USSR algorithm for fast multiplication (THERE IS NOT A SINGLE 100% CORRECT PSEUDO CODE ONLINE) */
static void BigInt_Karatsuba_internal(size_t num1_NumWords, BigInt_t * num1, size_t num2_NumWords, BigInt_t * num2, size_t Out_NumWords, BigInt_t * Out, int rlevel) /* Out should be XWords + YWords in size to always avoid overflow */
{
    /* Optimise the size, to avoid any waste any resources */
    num1_NumWords = BigInt_truncate(num1_NumWords, num1);
    num2_NumWords = BigInt_truncate(num2_NumWords, num2);

    if (num1_NumWords == 0 || num2_NumWords == 0)
    {
        BigInt_zero(Out_NumWords, Out);
        return;
    }
    if (num1_NumWords == 1 && num2_NumWords == 1)
    {
        BigInt_tmp_t result = ((BigInt_tmp_t)(*num1)) * ((BigInt_tmp_t)(*num2));
        if (Out_NumWords == 2) { BigInt_FROM_INT(Out, result); }
        else BigInt_from_int(Out_NumWords, Out, result);
        return;
    }

    size_t m = MIN(num2_NumWords, num1_NumWords);
    size_t m2 = m / 2;
    /* do A round up, this is what stops infinite recursion when the inputs are size 1 and 2 */
    if ((m % 2) == 1) ++m2;

    /* low 1 */
    size_t low1_NumWords = m2;
    BigInt_t * low1 = num1;
    /* high 1 */
    size_t high1_NumWords = num1_NumWords - m2;
    BigInt_t * high1 = num1 + m2;
    /* low 2 */ 
    size_t low2_NumWords = m2;
    BigInt_t * low2 = num2;
    /* high 2 */
    size_t high2_NumWords = num2_NumWords - m2;
    BigInt_t * high2 = num2 + m2;

    // z0 = karatsuba(low1, low2)
    // z1 = karatsuba((low1 + high1), (low2 + high2))
    // z2 = karatsuba(high1, high2)
    size_t z0_NumWords = low1_NumWords + low2_NumWords;
    BigInt_t * z0 = alloca(z0_NumWords*BigIntWordSize);
    size_t z1_NumWords = (MAX(low1_NumWords, high1_NumWords)+1) + (MAX(low2_NumWords, high2_NumWords)+1);
    BigInt_t * z1 = alloca(z1_NumWords*BigIntWordSize);
    size_t z2_NumWords =  high1_NumWords + high2_NumWords;
    int use_out_as_z2 = (Out_NumWords >= z2_NumWords); /* Sometimes we can use Out to store z2, then we don't have to copy from z2 to out later (2X SPEEDUP!) */
    if (use_out_as_z2) {BigInt_zero(Out_NumWords-(z2_NumWords),Out+z2_NumWords);}/* The remaining part of Out must be ZERO'D */
    BigInt_t * z2 = (use_out_as_z2) ? Out : alloca(z2_NumWords*BigIntWordSize);

    /* Make z0 and z2 */
    BigInt_Karatsuba_internal(low1_NumWords, low1, low2_NumWords, low2, z0_NumWords, z0, rlevel+1);
    BigInt_Karatsuba_internal(high1_NumWords, high1, high2_NumWords, high2, z2_NumWords, z2, rlevel+1);

    /* make z1 */
    {
        size_t low1high1_NumWords = MAX(low1_NumWords, high1_NumWords)+1;
        size_t low2high2_NumWords = MAX(low2_NumWords, high2_NumWords)+1;
        BigInt_t * low1high1 = alloca(low1high1_NumWords*BigIntWordSize);
        BigInt_t * low2high2 = alloca(low2high2_NumWords*BigIntWordSize);
        BigInt_add(low1_NumWords, low1, high1_NumWords, high1, low1high1_NumWords, low1high1);
        BigInt_add(low2_NumWords, low2, high2_NumWords, high2, low2high2_NumWords, low2high2);
        BigInt_Karatsuba_internal(low1high1_NumWords, low1high1, low2high2_NumWords, low2high2, z1_NumWords, z1, rlevel+1);
    }

    // return (z2 * 10 ^ (m2 * 2)) + ((z1 - z2 - z0) * 10 ^ m2) + z0
    BigInt_sub(z1_NumWords, z1, z2_NumWords, z2, z1_NumWords, z1);
    BigInt_sub(z1_NumWords, z1, z0_NumWords, z0, z1_NumWords, z1);
    if (!use_out_as_z2) BigInt_copy_dif(Out_NumWords, Out, z2_NumWords, z2);
    _lshift_word(Out_NumWords, Out, m2);
    BigInt_add(z1_NumWords, z1, Out_NumWords, Out, Out_NumWords, Out);
    _lshift_word(Out_NumWords, Out, m2);
    BigInt_add(Out_NumWords, Out, z0_NumWords, z0, Out_NumWords, Out);
}

void BigInt_mul(size_t ANumWords, BigInt_t * A, size_t BNumWords, BigInt_t * B, size_t OutNumWords, BigInt_t * Out)
{
    BigInt_Karatsuba_internal(ANumWords, A, BNumWords, B, OutNumWords, Out, 0);
}
 
void BigInt_div(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    BigInt_t * current = alloca(NumWords*BigIntWordSize);
    BigInt_t * denom = alloca(NumWords*BigIntWordSize);
    BigInt_t * tmp = alloca(NumWords*BigIntWordSize);

    BigInt_from_int(NumWords, current, 1); // int current = 1;
    BigInt_copy(NumWords, denom, B); // denom = B
    BigInt_copy(NumWords, tmp, A); // tmp   = A

    const BigInt_tmp_t half_max = 1 + (BigInt_tmp_t)(MAX_VAL / 2);
    int overflow = 0;
    while (BigInt_cmp(NumWords, denom, A) != LARGER) // while (denom <= A) {
    {
        if (denom[NumWords - 1] >= half_max) {
            overflow = 1;
            break;
        }
        _lshift_one_bit(NumWords, current); //   current <<= 1;
        _lshift_one_bit(NumWords, denom); //   denom <<= 1;
    }
    if (!overflow) {
        _rshift_one_bit(NumWords, denom); // denom >>= 1;
        _rshift_one_bit(NumWords, current); // current >>= 1;
    }
    BigInt_zero(NumWords, Out); // int answer = 0;

    while (!BigInt_is_zero(NumWords, current)) // while (current != 0)
    {
        if (BigInt_cmp(NumWords, tmp, denom) != SMALLER) //   if (dividend >= denom)
        {
            BigInt_sub(NumWords, tmp, NumWords, denom, NumWords, tmp); //     dividend -= denom;
            BigInt_or(NumWords, Out, current, Out); //     answer |= current;
        }
        _rshift_one_bit(NumWords, current); //   current >>= 1;
        _rshift_one_bit(NumWords, denom); //   denom >>= 1;
    }
}

void BigInt_lshift(size_t NumWords, BigInt_t * B, int nbits)
{
    /* Handle shift in multiples of word-size */
    const int nbits_pr_word = (BigIntWordSize * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0) {
        _lshift_word(NumWords, B, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0) {
        size_t i;
        for (i = (NumWords - 1); i > 0; --i) {
            B[i] = (B[i] << nbits) | (B[i - 1] >> ((8 * BigIntWordSize) - nbits));
        }
        B[i] <<= nbits;
    }
}

void BigInt_rshift(size_t NumWords, BigInt_t * B, int nbits)
{
    /* Handle shift in multiples of word-size */
    const int nbits_pr_word = (BigIntWordSize * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0) {
        _rshift_word(NumWords, B, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0) {
        size_t i;
        for (i = 0; i < (NumWords - 1); ++i) {
            B[i] = (B[i] >> nbits) | (B[i + 1] << ((8 * BigIntWordSize) - nbits));
        }
        B[i] >>= nbits;
    }
}

void BigInt_mod(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    /* Take divmod and throw away div part */
    BigInt_t * tmp = alloca(NumWords*BigIntWordSize);
    BigInt_divmod(NumWords, A, B, tmp, Out);
}

void BigInt_divmod(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * C, BigInt_t * D)
{
    BigInt_t * tmp = alloca(NumWords*BigIntWordSize);

    /* Out = (A / B) */
    BigInt_div(NumWords, A, B, C);

    /* tmp = (Out * B) */
    BigInt_mul(NumWords, C, NumWords, B, NumWords, tmp);

    /* Out = A - tmp */
    BigInt_sub(NumWords, A, NumWords, tmp, NumWords, D);
}

void BigInt_and(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    for (size_t i = 0; i < NumWords; ++i) {
        Out[i] = (A[i] & B[i]);
    }
}

void BigInt_or(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    for (size_t i = 0; i < NumWords; ++i) {
        Out[i] = (A[i] | B[i]);
    }
}

void BigInt_xor(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    for (size_t i = 0; i < NumWords; ++i) {
        Out[i] = (A[i] ^ B[i]);
    }
}

int BigInt_cmp(size_t NumWords, BigInt_t * A, BigInt_t * B)
{
    size_t i = NumWords;
    do {
        i -= 1; /* Decrement first, to start with last array element */
        if (A[i] > B[i]) {
            return LARGER;
        } else if (A[i] < B[i]) {
            return SMALLER;
        }
    } while (i != 0);

    return EQUAL;
}

int BigInt_is_zero(size_t NumWords, BigInt_t * BigInt)
{
    for (size_t i = 0; i < NumWords; ++i) {
        if (BigInt[i]) {
            return 0;
        }
    }

    return 1;
}

void BigInt_pow(size_t NumWords, BigInt_t * A, BigInt_t * B, BigInt_t * Out)
{
    BigInt_zero(NumWords, Out);

    if (BigInt_cmp(NumWords, B, Out) == EQUAL) {
        /* Return 1 when exponent is 0 -- BigInt^0 = 1 */
        BigInt_inc(NumWords, Out);
    } else {
        BigInt_t * bcopy = alloca(NumWords*BigIntWordSize), * tmp = alloca(NumWords*BigIntWordSize);
        BigInt_copy(NumWords, bcopy, B);

        /* Copy A -> tmp */
        BigInt_copy(NumWords, tmp, A);

        BigInt_dec(NumWords, bcopy);

        /* Begin summing products: */
        while (!BigInt_is_zero(NumWords, bcopy)) {
            /* Out = tmp * tmp */
            BigInt_mul(NumWords, tmp, NumWords, A, NumWords, Out);
            /* Decrement B by one */
            BigInt_dec(NumWords, bcopy);

            BigInt_copy(NumWords, tmp, Out);
        }

        /* Out = tmp */
        BigInt_copy(NumWords, Out, tmp);
    }
}

void BigInt_isqrt(size_t NumWords, BigInt_t * A, BigInt_t * B)
{
    BigInt_t * low = alloca(NumWords*BigIntWordSize);
    BigInt_t * high = alloca(NumWords*BigIntWordSize);
    BigInt_t * mid = alloca(NumWords*BigIntWordSize);
    BigInt_t * tmp = alloca(NumWords*BigIntWordSize);

    BigInt_zero(NumWords, low);
    BigInt_copy(NumWords, high, A);
    BigInt_copy(NumWords, mid, high);
    BigInt_rshift(NumWords, mid, 1);
    BigInt_inc(NumWords, mid);

    while (BigInt_cmp(NumWords, high, low) > 0) {
        BigInt_mul(NumWords, mid, NumWords, mid, NumWords, tmp);
        if (BigInt_cmp(NumWords, tmp, A) > 0) {
            BigInt_copy(NumWords, high, mid);
            BigInt_dec(NumWords, high);
        } else {
            BigInt_copy(NumWords, low, mid);
        }
        BigInt_sub(NumWords, high, NumWords, low, NumWords, mid);
        _rshift_one_bit(NumWords, mid);
        BigInt_add(NumWords, low, NumWords, mid, NumWords, mid);
        BigInt_inc(NumWords, mid);
    }
    BigInt_copy(NumWords, B, low);
}

void BigInt_copy(size_t NumWords, BigInt_t * Dst, BigInt_t * Src)
{
    for (size_t i = 0; i < NumWords; ++i) {
        Dst[i] = Src[i];
    }
}

void BigInt_copy_dif(size_t DstNumWords, BigInt_t * Dst, size_t SrcNumWords, BigInt_t * Src)
{
    size_t smallest = (DstNumWords < SrcNumWords) ? DstNumWords : SrcNumWords;
    size_t i;
    for (i = 0; i < smallest; ++i) Dst[i] = Src[i];
    for (; i < DstNumWords; ++i) Dst[i] = 0;
}

/* Private / Static functions. */
static void _rshift_word(size_t NumWords, BigInt_t * A, int nwords)
{
    size_t i;
    if (nwords >= NumWords) {
        for (i = 0; i < NumWords; ++i) {
            A[i] = 0;
        }
        return;
    }

    for (i = 0; i < NumWords - nwords; ++i) {
        A[i] = A[i + nwords];
    }
    for (; i < NumWords; ++i) {
        A[i] = 0;
    }
}

static void _lshift_word(size_t NumWords, BigInt_t * A, int nwords)
{
    int_fast32_t i;
    /* Shift whole words */
    for (i = (NumWords - 1); i >= nwords; --i) {
        A[i] = A[i - nwords];
    }
    /* Zero pad shifted words. */
    for (; i >= 0; --i) {
        A[i] = 0;
    }
}

static void _lshift_one_bit(size_t NumWords, BigInt_t * A)
{
    for (size_t i = (NumWords - 1); i > 0; --i) {
        A[i] = (A[i] << 1) | (A[i - 1] >> ((8 * BigIntWordSize) - 1));
    }
    A[0] <<= 1;
}

static void _rshift_one_bit(size_t NumWords, BigInt_t * A)
{
    for (size_t i = 0; i < (NumWords - 1); ++i) {
        A[i] = (A[i] >> 1) | (A[i + 1] << ((8 * BigIntWordSize) - 1));
    }
    A[NumWords - 1] >>= 1;
}
