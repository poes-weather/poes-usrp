//       1         2         3         4         5         6         7         8
//345678901234567890123456789012345678901234567890123456789012345678901234567890
//******************************************************************************
// File: Reed Solonom.cpp
//
// Description:
//     This is the class implementation of the Reed Solomon Class. 
//
// Version |    Date    | Author          | Notes
// --------|------------|-----------------|-------------------------------------
//   1.0   | 04/01/2002 | BAZ             | Program Created
//         |            |                 |
//******************************************************************************

#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "ReedSolomon.h"

#if 1
#  ifndef MAX
#     define MAX(A, B) ( A > B ? A:B )
#  endif
#else
using namespace std;

// define MAX template
template <typename T1> T1 MAX(T1 A, T1 B) {
    return (A<B ? B : A);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Initialize constants
////////////////////////////////////////////////////////////////////////////////
/* Reed Solomon Codes define the symbol ordering in the opposite direction  
   that the data symbols are received.  
  
   R254 == rs_data[0]  
   R253 == rs_data[1]  
   ...  
   R1   == rs_data[253]  
   R0   == rs_data[254]  
  
   For CCSDS, there are 223 symbols (including virtual fill) of rs data from   
   which 32 parity symbols are generated. This gives a total code word length   
   of 255 bytes.    
                                   8  
  Galois Field Operations over GF(2 ), ground field of GF(2):  
    Addition:  
        c = a XOR b  

    Subtraction:  
        c = a - b, w/o borrow, i.e. XOR also therefore  
        c = a XOR b = a - b  

    Multiplication:  
                                       8  
        To multiply two element of GF(2 )  
        take their logarithm to base alpha,  
        Add the logs as integers modulo 255,  
        and take the antilog of the answer.  
  
        c = a * b ====>  
  
        c = antilog((log(a) + log(b)) mod 255)   

    Division:  
        c = a / b  
        let d = inverse(b)  
        c = a * d (see Multiplication)  

    Inverse:  
        b = alpha**x  
        b_inverse = alpha**(number_of_symbols - x) = alpha**(255 - x)  
*/
unsigned char CReedSolomon::GF256[]={ // normal
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x87,0x89,0x95,0xAD,0xDD,0x3D,0x7A,0xF4,
    0x6F,0xDE,0x3B,0x76,0xEC,0x5F,0xBE,0xFB,0x71,0xE2,0x43,0x86,0x8B,0x91,0xA5,0xCD,
    0x1D,0x3A,0x74,0xE8,0x57,0xAE,0xDB,0x31,0x62,0xC4,0x0F,0x1E,0x3C,0x78,0xF0,0x67,
    0xCE,0x1B,0x36,0x6C,0xD8,0x37,0x6E,0xDC,0x3F,0x7E,0xFC,0x7F,0xFE,0x7B,0xF6,0x6B,
    0xD6,0x2B,0x56,0xAC,0xDF,0x39,0x72,0xE4,0x4F,0x9E,0xBB,0xF1,0x65,0xCA,0x13,0x26,
    0x4C,0x98,0xB7,0xE9,0x55,0xAA,0xD3,0x21,0x42,0x84,0x8F,0x99,0xB5,0xED,0x5D,0xBA,
    0xF3,0x61,0xC2,0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x07,0x0E,0x1C,0x38,0x70,0xE0,
    0x47,0x8E,0x9B,0xB1,0xE5,0x4D,0x9A,0xB3,0xE1,0x45,0x8A,0x93,0xA1,0xC5,0x0D,0x1A,
    0x34,0x68,0xD0,0x27,0x4E,0x9C,0xBF,0xF9,0x75,0xEA,0x53,0xA6,0xCB,0x11,0x22,0x44,
    0x88,0x97,0xA9,0xD5,0x2D,0x5A,0xB4,0xEF,0x59,0xB2,0xE3,0x41,0x82,0x83,0x81,0x85,
    0x8D,0x9D,0xBD,0xFD,0x7D,0xFA,0x73,0xE6,0x4B,0x96,0xAB,0xD1,0x25,0x4A,0x94,0xAF,
    0xD9,0x35,0x6A,0xD4,0x2F,0x5E,0xBC,0xFF,0x79,0xF2,0x63,0xC6,0x0B,0x16,0x2C,0x58,
    0xB0,0xE7,0x49,0x92,0xA3,0xC1,0x05,0x0A,0x14,0x28,0x50,0xA0,0xC7,0x09,0x12,0x24,
    0x48,0x90,0xA7,0xC9,0x15,0x2A,0x54,0xA8,0xD7,0x29,0x52,0xA4,0xCF,0x19,0x32,0x64,
    0xC8,0x17,0x2E,0x5C,0xB8,0xF7,0x69,0xD2,0x23,0x46,0x8C,0x9F,0xB9,0xF5,0x6D,0xDA,
    0x33,0x66,0xCC,0x1F,0x3E,0x7C,0xF8,0x77,0xEE,0x5B,0xB6,0xEB,0x51,0xA2,0xC3};
unsigned char CReedSolomon::GF256d[]={ // dual symbol
    0x7B,0xAF,0x99,0xFA,0x86,0xEC,0xEF,0x8D,0xC0,0x0C,0xE9,0x79,0xFC,0x72,0xD0,0x91,
    0xB4,0x28,0x44,0xB3,0xED,0xDE,0x2B,0x26,0xFE,0x21,0x3B,0xBB,0xA3,0x70,0x83,0x7A,
    0x9E,0x3F,0x1C,0x74,0x24,0xAD,0xCA,0x11,0xAC,0xFB,0xB7,0x4A,0x09,0x7F,0x08,0x4E,
    0xAE,0xA8,0x5C,0x60,0x1E,0x27,0xCF,0x87,0xDD,0x49,0x6B,0x32,0xC4,0xAB,0x3E,0x2D,
    0xD2,0xC2,0x5F,0x02,0x53,0xEB,0x2A,0x17,0x58,0xC7,0xC9,0x73,0xE1,0x37,0x52,0xDA,
    0x8C,0xF1,0xAA,0x0F,0x8B,0x34,0x30,0x97,0x40,0x14,0x3A,0x8A,0x05,0x96,0x71,0xB2,
    0xDC,0x78,0xCD,0xD4,0x36,0x63,0x7C,0x6A,0x03,0x62,0x4D,0xCC,0xE5,0x90,0x85,0x8E,
    0xA2,0x41,0x25,0x9C,0x6C,0xF7,0x5E,0x33,0xF5,0x0D,0xD8,0xDF,0x1A,0x80,0x18,0xD3,
    0xF3,0xF9,0xE4,0xA1,0x23,0x68,0x50,0x89,0x67,0xDB,0xBD,0x57,0x4C,0xFD,0x43,0x76,
    0x77,0x46,0xE0,0x06,0xF4,0x3C,0x7E,0x39,0xE8,0x48,0x5A,0x94,0x22,0x59,0xF6,0x6F,
    0x95,0x13,0xFF,0x10,0x9D,0x5D,0x51,0xB8,0xC1,0x3D,0x4F,0x9F,0x0E,0xBA,0x92,0xD6,
    0x65,0x88,0x56,0x7D,0x5B,0xA5,0x84,0xBF,0x04,0xA7,0xD7,0x54,0x2E,0xB0,0x8F,0x93,
    0xE7,0xC3,0x6E,0xA4,0xB5,0x19,0xE2,0x55,0x1F,0x16,0x69,0x61,0x2F,0x81,0x29,0x75,
    0x15,0x0B,0x2C,0xE3,0x64,0xB9,0xF0,0x9B,0xA9,0x6D,0xC6,0xF8,0xD5,0x07,0xC5,0x9A,
    0x98,0xCB,0x20,0x0A,0x1D,0x45,0x82,0x4B,0x38,0xD9,0xEE,0xBC,0x66,0xEA,0x1B,0xB1,
    0xBE,0x35,0x01,0x31,0xA6,0xE6,0xF2,0xC8,0x42,0x47,0xD1,0xA0,0x12,0xCE,0xB6};

////////////////////////////////////////////////////////////////////////////////
// Define constructor functions
////////////////////////////////////////////////////////////////////////////////
// Definition of RSD parameters and how to set them to decode various
///Reed-Solomon Codes 
//
// Generator Polynomial:
//
//        2t-1           i+Mo                 POA
// G(X) = PROD (X - gamma   ),   gamma = alpha
//        i=0
//
// Parameters necessary to set up the program:
// m, t, Mo, POA, vf, I, MODE
//
// Equations to calculate parameters:
//
// m = number of bits per symbol
// n = 2**m - 1 = code word size
// k = (n - 2t) - vf = data size
// d = 2t = number of check symbols
//         (note that d often == 2t+1 = minimum distance)
// t = number of correctable errors
// n = modulus
// vf = number of symbols of virtual fill
// I = depth of interleaving
// frame_size = FrameSyncLength + (((n - 2t) - vf) * I) + (2t * I) = length of frame incl fs
// Mo = see generator polynomial
// POA = see generator polynomial
// MODE 
//    0 = Use conventional tables
//    1 = Use dual tables
CReedSolomon::CReedSolomon(int BitsPerSymbol, int CorrectableErrors, int mo, int poa, int VirtualFill, 
                           int Interleave, int FrameSyncLength, int mode)
{	
    rsp.m = BitsPerSymbol; /* bits per RS symbol */  
    /* 2**m - 1, number of symbols in field, rsp.n */  
    rsp.n = (int)pow((double)2, (double)BitsPerSymbol) - 1;  
    rsp.t = CorrectableErrors; /* number of correctable errors, rsp.t */  
    rsp.d = 2 * CorrectableErrors; /* 2 * t, rsp.d */  
    rsp.k = (rsp.n - 2 * CorrectableErrors) - VirtualFill; /* (n - 2t) - vf, rsp.k */  
    rsp.Mo = mo; /* see generator polynomial */  
    rsp.poa = poa; /* see generator polynomial */  
    rsp.vf = VirtualFill; /* number of bytes of virtual fill */  
    rsp.I = Interleave; /* the depth of interleaving */  
    rsp.mode = mode; /* specifies which table to use, dual or conventional */  
    rsp.modulus = rsp.n; /* n */  
    rsp.fsLength = FrameSyncLength;  
    /*  fsl + (((n - 2t) - vf) * I) + (2t * I), rsp.frameLength */  
    rsp.frameLength = FrameSyncLength + (((rsp.n - 2*CorrectableErrors) - VirtualFill) * Interleave) + 
                      (2*CorrectableErrors * Interleave);  
    rsp.synLoopMax = (rsp.frameLength - FrameSyncLength);  
  
    rsp.numErrorsPerFrame = 0;  
    rsp.uncorErrsPerFrame = 0;  
  
    rsp.numErrorsPerInterleave = new unsigned long[Interleave];

    rsp.s = new unsigned char[rsp.d];
    memset((char*)rsp.s,0,(rsp.d) * sizeof(unsigned char));

    rsp.sigma = new unsigned char[rsp.d];
    memset((char*)rsp.sigma,0,(rsp.d) * sizeof(unsigned char));

    rsp.errorMagnitudes = new unsigned char[rsp.t];
    memset((char*)rsp.errorMagnitudes,0,(rsp.t) * sizeof(unsigned char));

    rsp.errorLocations = new unsigned char[rsp.t];
    memset((char*)rsp.errorLocations,0,(rsp.t) * sizeof(unsigned char));
   
    /* local to CalcELPCoef() */  
    rsp.D = new unsigned char[rsp.d];
    memset((char*)rsp.D,0,(rsp.d) * sizeof(unsigned char));
   
    rsp.tmpSigma = new unsigned char[rsp.d];
    memset((char*)rsp.tmpSigma,0,(rsp.d) * sizeof(unsigned char));
   
    /* local to CalcErrorMagnitudes() */  
    rsp.Z = new unsigned char[rsp.t + 1];
    memset((char*)rsp.Z,0,(rsp.t + 1) * sizeof(unsigned char));

    /* determine max number of repititions for the antilog able */
    unsigned long m1, m2, maxAntilogReptitions;
    m1 = (rsp.n + 1) + (rsp.poa * (rsp.Mo + rsp.n));
    m2 = (rsp.n + 1) + (rsp.poa * rsp.n * (2 * rsp.t));
    maxAntilogReptitions = MAX(m1, m2);
    maxAntilogReptitions = (maxAntilogReptitions / (unsigned long) rsp.n) + 1;

    /*     allocate memory to hold Galois Field tables, the base table
        becomes the antilog table, and is repeated XXX number of
        times to eliminate the use of the mod function, since this is
        a time consuming operation, the antilog table will always
        be of length = 2^m */
    rsp.log_ptr = new unsigned char[rsp.n + 1]; 
    memset((char*)rsp.log_ptr,0,(rsp.n + 1) * sizeof(unsigned char));
    rsp.antilog_ptr = new unsigned char[maxAntilogReptitions * rsp.n];
    memset((char*)rsp.antilog_ptr,0,(maxAntilogReptitions * rsp.n) * sizeof(unsigned char));

    /* load table */
    if (mode==1 && rsp.m==8) memcpy((unsigned char*)rsp.antilog_ptr, GF256d, rsp.n);
    if (mode==0 && rsp.m==8) memcpy((unsigned char*)rsp.antilog_ptr, GF256, rsp.n);

	int i, j;
    /* repeat GF antilog table maxAntilogReptitions times */
    for(i=0, j=rsp.n;i<(int)(maxAntilogReptitions - 1);i++, j+=rsp.n)
        memcpy(&rsp.antilog_ptr[j], rsp.antilog_ptr, rsp.n);
   
    /* load GF log table */
    rsp.log_ptr[0] = 0;
	 for(i=0;i<rsp.n;i++)
		  rsp.log_ptr[rsp.antilog_ptr[i]] = (unsigned char) i;
}

// destructor
CReedSolomon::~CReedSolomon()
{
    delete [] rsp.numErrorsPerInterleave;
    delete [] rsp.s;
    delete [] rsp.sigma;
    delete [] rsp.errorMagnitudes;
    delete [] rsp.errorLocations; 
    delete [] rsp.D;
    delete [] rsp.tmpSigma;
    delete [] rsp.Z;
    delete [] rsp.log_ptr; 
    delete [] rsp.antilog_ptr;
}	

////////////////////////////////////////////////////////////////////////////////
// define overloaded operator functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Define member access functions
////////////////////////////////////////////////////////////////////////////////
unsigned long CReedSolomon::CorrectableErrorsInFrame() const
{
    return rsp.numErrorsPerFrame;
}

unsigned long CReedSolomon::UncorrectableErrorsInFrame() const
{
    return rsp.uncorErrsPerFrame;
}

// this returns the number of errors per interleave where you must pass an array
// of length Interleave to receive the results
void CReedSolomon::CorrectableErrorsPerInterleave(
                        unsigned long *ErrorsPerInterleave) const
{
    for(int i=0; i<rsp.I; ++i) 
        ErrorsPerInterleave[i]=rsp.numErrorsPerInterleave[i];
}

////////////////////////////////////////////////////////////////////////////////
// Define member operations functions
////////////////////////////////////////////////////////////////////////////////
// decode the data frame
// This function passes each interleaved code block to the  core decoder.  It
// also maintains statistics.
// returns true if no errors, false if correctable or uncorrectable errors
bool CReedSolomon::Decode(unsigned char* rsDataFrame)  
{  
    register unsigned char *startOfData;  
    int numErrors;  
  
    /* the start of data is rsp.fsLength bytes from the beginning of the frame  
       because the frame sync pattern takes up the first rsp.fsLength bytes */  
    startOfData = rsDataFrame + rsp.fsLength;  

    /*     decode and correct each code word, the value of i specifies
        which level in the interleaved code block to decode */  
    rsp.uncorErrsPerFrame = 0;  
    rsp.numErrorsPerFrame = 0;  
    for(int i=0;i<rsp.I;i++)  
    {  
		numErrors = RSDecode(startOfData, i);
        if(numErrors > 0)  
        {  
            rsp.numErrorsPerFrame += numErrors;  
            rsp.numErrorsPerInterleave[i]=numErrors;
        }  
        else if(numErrors < 0)  
        {  
            rsp.uncorErrsPerFrame++;  
            rsp.numErrorsPerInterleave[i]=0;//numErrors;
        }  
    }  

    if (rsp.numErrorsPerFrame>0 || rsp.uncorErrsPerFrame>0) {
		return false; }
    else return true;
} 

////////////////////////////////////////////////////////////////////////////////
// Define helper functions
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************* 
*   Argument list:
*   Name          Definition                 Use   Purpose
*   ------------  -----------                ----  --------------------------
*    rs_data      register unsigned char*    I/O   Pointer to start of
*                                                  synchronized RS code block.
*    intLev       int                        I     Specifies which level of
*                                                  interleaving we are to
*                                                  decode.
*
*  Return Value:  Number of Errors Detected or the uncorrectable  
*                 flag if the code word is uncorrectable.  
*  
*    Notes:
*        1) Calculate the syndrome vector from the received RS codeword.
*        2) Calculate the coefficients of the error locator polynomial.
*        3) Calculate the roots of the error locator polynomial.
*        4) Calculate the error magnitudes.
*        5) Correct the symbols in error with the previously calculated
*           information.
*                                                                     n 
*        Polynomial coefficients are stored P[0] = a*X ... P[n] = z*X
*  
*******************************************************************************/  
int CReedSolomon::RSDecode(unsigned char *rs_data, int intLev)
{  
    int numErrors=0, degreeOfSigma;  

    /* clear polynomial coefficient vectors, (this is necessary) */  
    memset((char *) rsp.s, 0x00, rsp.d);  
    memset((char *) rsp.sigma, 0x00, rsp.d);  
    memset((char *) rsp.errorLocations, 0x00, rsp.t);  
    memset((char *) rsp.errorMagnitudes, 0x00, rsp.t);  
  
    /* Step (1) Calculate the syndrome */  
    /* if not equal to zero then there are errors, if equal to zero  
       then there are no errors, so just return */  
    if(RSDCalcSyndrome(rs_data, intLev) != 0)  
    {  
        /* Step (2) Calculation of the Error Locator Polynomial */  
        if((degreeOfSigma = RSDCalcELPCoef()) == UNCORRECTABLE_FLAG)  
            return(UNCORRECTABLE_FLAG * 2);  
  
        /* Step (3) Calculation of the roots of the error locator   
            polynomial, which yields the locations of the errors */  
        if((numErrors = RSDCalcErrorLocations(degreeOfSigma))== UNCORRECTABLE_FLAG)  
            return(UNCORRECTABLE_FLAG * 3);  
  
        /* step (4) */  
        if(RSDCalcErrorMagnitudes(numErrors) == UNCORRECTABLE_FLAG)  
            return(UNCORRECTABLE_FLAG * 4);  
  
        /* step (5) */  
        RSDCorrectSymbols(rs_data, intLev, numErrors);  
    }  

    return(numErrors);  
}

/********************************************************  
*   Argument list:
*   Name          Definition                 Use   Purpose
*   ------------  -----------                ----  --------------------------
*    r              register unsigned char* I/O    The received data vector.
*    intLev          int                      I        Interleave level to decode.
*  
*   Returns: The sum of all syndrome components.  
*            If it is equal to 0, then no errors have occured.  
*
*  This function implements the FFT-like syndrome calculation.
*  
*  Reed Solomon Codes define the symbol ordering in the opposite direction
*  that the data symbols are received.
*
*  R_n-1 = r[0]
*  R_n-2 = r[1]
*  ...
*  R1   = r[n-2]
*  R0   = r[n-1]
*
*  For CCSDS:
*  R254 = r[0]
*  R253 = r[1]
*  ...
*  R1   = r[253]
*  R0   = r[254]
*
*    Recursive syndrome formula.
*
*                           (Mo + i)                  (Mo + i)  
*   Si = ((...(R[n-1] * gamma        + R[n-2]) * gamma          +  
*                    (Mo + i)  
*        ...) * gamma         + R[0])  
*  
*                      POA  
*   Where gamma = alpha    , therefore powerOfAlpha = (POA * (Mo + i))  
*   and  
*   0 <= i <= (d - 2) = (2t - 1)  
*    and
*    n = 2**m - 1, m = number of bits per symbol
*  
*********************************************************/  
int CReedSolomon::RSDCalcSyndrome(register unsigned char *r, int intLev)  
{  
	int i, j, checkSum=0, interleave;
	int poa, Mo, checkSize, loopMax, term;
    unsigned char *sp, *alogp, *logp;  
  
    /* put parameters into registers */  
    poa = rsp.poa;  
    Mo = rsp.Mo;  
    checkSize = rsp.d;  
    loopMax = rsp.synLoopMax;  
    interleave = rsp.I;  
    sp = rsp.s;  
    logp = rsp.log_ptr;  
    alogp = rsp.antilog_ptr;  
  
    for(i=0;i<checkSize;i++)  
    {  
        term = poa * (Mo + i);  
  
        for(j=intLev;j<loopMax;j+=interleave)  
        {  
            if(sp[i] != 0)  
                sp[i] = alogp[logp[sp[i]] + term] ^ r[j];  
            else  
                sp[i] = r[j];  
        }  
        checkSum += sp[i];  
	 }

    return(checkSum);  
}  

/********************************************************  
*   Returns: The degree of the error location polynomial.  
*  
*   This is the Berlekamp Algorithm.  This algorithm always  
*   finds the shortest feedback shift register that generates  
*   the first 2t (32 for CCSDS) terms of S(z), the syndrome.  
*  
*   This algorithm finds the solution of the key equation:  
*               [sigma(z)*syndrome(z)] = 0  
*   The is syndrome is given, so this algorithm finds sigma(z)  
*   of minimum degree.  This is the same problem as finding  
*   the minimum-length shift register which generates the first  
*   2t terms of syndrome(z), which will predict s[1] from s[0]  
*   and so on.  Basically,  
*   For each new syndrome symbol, the algorithm tests to see  
*   whether the current connection polynomial will predict this  
*   symbol correctly.  If so, the connection polynomial is left  
*   unchanged and the correction term is modified by multiplying  
*   by z.  If the current connection polynomial fails, then it is   
*   modified by adding the correction term.  One then checks to  
*   determine if the new register length has increased.  If it has  
*   not, then the correction polynomial that is currently being  
*   maintained is at least as good a choice as any other.  If the  
*   length of the new register increases, then the previous connection  
*   polynomial becomes the better choice since the oldest symbol  
*   associated with this register must have a higher index than the  
*   oldest symbol associated with the current correction term.  
*  
*   sigma: the connection polynomial(error locator poly)  
*   s: the syndrome vector  
*   D: the correction (term) polynomial   
*   d: the discrepancy  
*   L: the length of the associated feedback shift register  
*      (also the degree of the error locator polynomial)
*   k: the location of the oldest symbol in the feedback  
*      shift register where the register fails  
*  
*********************************************************/  
int CReedSolomon::RSDCalcELPCoef()  
{  
    unsigned char d;  
    int i, L, tmpL, n, k, powerOfAlpha;  
    int checkSize;  
    unsigned char *sp, *Dp, *tmpSigmap, *sigmap, *alogp, *logp;  
   
    /* put parameters into registers */  
    checkSize = rsp.d;  
    sp = rsp.s;  
    Dp = rsp.D;  
    tmpSigmap = rsp.tmpSigma;  
    sigmap = rsp.sigma;  
    logp = rsp.log_ptr;  
    alogp = rsp.antilog_ptr;  
  
    /* parameter initialization */  
    memset((char *) Dp, 0x00, checkSize);  
    memset((char *) tmpSigmap, 0x00, checkSize);  
    L = tmpL = 0;  
    k = -1;  
    sigmap[0] = alogp[0]; /* sigma(z) = 1 == alpha**0 */  
    Dp[1] = alogp[0]; /* D(z) = z */  
  
    /* for each syndrome symbol... */  
    for(n=0;n<checkSize;n++)  
    {  
        /* calculate the discrepancy using:  
           d = SUM(i=0, L, sigma[i] * syndrome[n-i]) */  
        d = 0;  
        for(i=0;i<=L;i++)  
        {  
            if(sigmap[i] != 0 && sp[n-i] != 0)  
                d = d ^ alogp[(logp[sigmap[i]] +  
                    logp[sp[n-i]])];  
        }  
  
        /* if there is a discrepancy... */  
        if(d != 0)  
        {  
            /* modify sigma(z) by subtracting the correction term  
               (d * D(z)) <sigma[i] - (d * D[i])>*/  
            for(i=0;i<checkSize;i++)  
            {  
                if(Dp[i] != 0)  
                    tmpSigmap[i] = sigmap[i] ^   
                        alogp[(logp[d] + logp[Dp[i]])];  
                else  
                    tmpSigmap[i] = sigmap[i];  
            }  
  
            /* if new register length has increased... */  
            if(L < (n - k))   
            {  
                tmpL = n - k;  
                k = n - L;  
  
                /* then the previous connection polynomial becomes a  
                   better choice */  
                /* D[i] = sigma[i] / d; */  
                powerOfAlpha = rsp.n - (int)logp[d];  
                for(i=0;i<checkSize;i++)  
                {  
                    if(powerOfAlpha != 0 && sigmap[i] != 0)  
                        Dp[i] = alogp[((int)logp[sigmap[i]]  
                                + powerOfAlpha)];  
                    else  
                        Dp[i] = 0;  
                }  
  
                L = tmpL;  
            }  
  
            /* now update sigma with new sigma */  
            memcpy(sigmap, tmpSigmap, checkSize);
        }  
  
        /* D(z) = z * D(z), this is accomplished by shifting the terms  
           of D(z) to the left, i.e. a logical left shift is effectively  
           a multiply by z */  
        for(i=(checkSize - 1);i>=1;i--)  
            Dp[i] = Dp[i-1];  
        Dp[0] = 0x00;  
    }  
  
    /* return the degree of the error locator polynomial, sigma(x) */
    return(L);  
}  

/********************************************************  
*   Argument list:
*   Name          Definition   Use   Purpose
*   ------------  -----------  ----  --------------------------
*    degreeOfSigma int            I     Provides a means of detecting
*                                     uncorrectable code words in this
*                                     stage.
*  
*   Returns: The number of errors detected.
*             OR
*             If number of errors detected is greater than the
*             degreeOfSigma, then the uncorrectable flag is returned,
*             since this is an uncorrectable code word.
*  
*   Calculate the roots of the error location   
*   polynomial by using the Chien search.  
*   Solve:  
*                            i                 poa  
*   sigma(x): where x = gamma and gamma = alpha    , and  
*   vf_end <= i <= (n - 1), n = 2**m - 1, m = number of bits per symbol,
*   and vf_end = the first location in the received vector following the
*   virtual fill symbols.  I start the Chien Search at vf_end since
*   there is no point trying to find errors in the virtual fill
*   symbols where errors will never exist.
*                                                      i
*    An error location is found for each value of gamma  that
*    makes sigma evaluate to 0.
*  
*********************************************************/  
int CReedSolomon::RSDCalcErrorLocations(int degreeOfSigma)  
{  
    int numErrors=0;  
    unsigned char sum;  
    int j, i, powerOfAlpha;  
    int codeWrdLength, poa, t, vf;  
    unsigned char *sigmap, *alogp, *logp, *errorLocationsp;  
   
    /* put parameters into registers */  
    codeWrdLength = rsp.n;  
    poa = rsp.poa;  
    t = rsp.t;  
    vf = rsp.vf;  
    errorLocationsp = rsp.errorLocations;  
    sigmap = rsp.sigma;  
    logp = rsp.log_ptr;  
    alogp = rsp.antilog_ptr;  
  
    /* check for valid degree of sigma, I don't think that I   
       need this check, because I don't think that this condition  
       will ever exist, so I will comment it out for now. If  
       any strang problems arise(seg fault), uncomment it */  
    /*  
    if(degreeOfSigma >= checkSize)  
        return(UNCORRECTABLE_FLAG);  
    */  
  
    /* Solve:  
       sigma(x) = sum = sigma[0]*X**0 + sigma[1]*X**1 +  
       sigma[2]*X**2 + ... + sigma[i]*X**i   
       by substituting 1, gamma, gamma**2,..., gamma**n-1 into  
       sigma(x), so that  
       sigma(x) = sum = sigma[0] + sigma[1]*gamma**j +  
       sigma[2]*gamma**2j + ... + sigma[i]*gamma**ij  */  
    /* ONLY look for errors in the data symbols,   
       not the virtual fill symbols */  
    for(j=vf;j<=codeWrdLength;j++)  
    {  
        sum = sigmap[0];  
        for(i=1;i<=degreeOfSigma;i++)  
        {  
            /* NOTE: powerOfAlpha = log(alpha**powerOfAlpha) */  
            powerOfAlpha = i * (j * poa);  
  
            if(powerOfAlpha !=0 && sigmap[i] != 0)  
                sum = sum ^ alogp[(logp[sigmap[i]] + powerOfAlpha)];  
        }  
  
        /* if sum equals 0, then alpha**rsp.n-j is an error location  
           number(alpha**j is a root of sigma(x), alpha**rsp.n == 1,   
           alpha**-j == alpha**n-j, therefore alpha**n-j is the reciprocal   
           of the root alpha**j, and the actual error location) */
        if(sum == 0)  
        {  
            /* if the number of errors equals the max number of errors  
               then max number of errors + 1 have just been detected  
               so return the uncorrectable flag. */  
            if(numErrors == t)  
                return(UNCORRECTABLE_FLAG);  
            else  
            {  
                /* n-j is the actual error location for R0 to RN */  
                /* NOTE: that the actual error location within the  
                   data buffer is codeWrdLength - (codeWrdLength - j) - 1 */  
                errorLocationsp[numErrors] = codeWrdLength - j;  
                numErrors++;  
            }  
        }  
    }  
  
    /* the degree of the error location polynomial and the number  
       of errors MUST match, otherwise this is an uncorrectable  
       code word */  
    if(numErrors != degreeOfSigma)  
        return(UNCORRECTABLE_FLAG);  
  
    return(numErrors);  
}  

/********************************************************  
*   Argument list:
*   Name          Definition   Use   Purpose
*   ------------  -----------  ----  --------------------------
*    numErrors      int            I     number of errors detected
*  
*   Returns: 0 if ok
*            else the uncorrectable flag is returned if the code
*            word is found to be uncorrectable.  
*
*   This function uses the Forney Algorithm to calculate the error
*   magnitudes.
*  
*   This function first calculates Z(X){Z(X) is the error magnitude
*   polynomial}:  
*                                                                 2
*   Z(X) = 1 + (s[1]+sigma[1])*X + (s[2]+s[1]*sigma[1]+sigma[2])*X + ...  
*                                                                  v
*   + (s[v] + sigma[1]*s[v-1] + ... + sigma[v-1]*s[1] + sigma[v])*X  
*   Where v = number of errors detected  
*  
*           n-l  
*   If gamma    is an error-location number, then the error value  
*   at location n-l is given by:  
*  
*                -(1-Mo)l         l  
*           gamma        * Z(gamma )     
*   e    = ------------------------  
*    n-l         l               l  
*           gamma  * sigma'(gamma )  
*  
*                      POA  
*   Where gamma = alpha    and  
*   sigma'(X) is the derivative of sigma(X) and is given by:  
*  
*                                    2                     2i  
*   sigma'(X) = sigma[1] + sigma[3]*X + ... + sigma[2i+1]*X  + ...  
*   0 <= i <= floor((t-1)/2)   
*  
*********************************************************/  
int CReedSolomon::RSDCalcErrorMagnitudes(int numErrors)  
{  
    double dtmp;  
    int i, j, powerOfAlpha, location, elpMax;  
    unsigned char emag, elp;  
    int codeWrdLength, poa;  
    int modulus;  
    unsigned char *sp, *sigmap, *Zp, *alogp, *logp;  
   
    /* put parameters into registers */  
    codeWrdLength = rsp.n;  
    modulus = rsp.modulus;
    poa = rsp.poa;  
    sp = rsp.s;  
    sigmap = rsp.sigma;  
    Zp = rsp.Z;  
    logp = rsp.log_ptr;  
    alogp = rsp.antilog_ptr;  
  
    /* this has to be done this way !!! */  
    dtmp = floor((double)((double)rsp.t - 1.0)/2.0);  
    elpMax = (int) dtmp;  
  
    memset((char *) Zp, 0x00, rsp.t+1);  
  
    /* calculate the coeffieicnts of Z(X) */  
    Zp[0] = alogp[0]; /* initialize to 1 == alpha**0 */  
    for(i=1;i<=numErrors;i++)  
    {  
        Zp[i] = sp[i-1] ^ sigmap[i];  
        for(j=1;j<i;j++)  
        {  
            if(sigmap[j] != 0 && sp[(i-j)-1] != 0)  
                Zp[i] = Zp[i] ^ alogp[(logp[sigmap[j]]  
                        + logp[sp[(i-j)-1]])];  
        }  
    }  
  
    /* for each error, calculate the error magnitude */  
    for(i=0;i<numErrors;i++)  
    {  
        location = codeWrdLength - rsp.errorLocations[i];  
  
        /* plug alpha**location into Z(X) */  
        emag = Zp[0];  
        for(j=1;j<=numErrors;j++)  
        {  
            /* calculate power of alpha for this error */  
            powerOfAlpha = (poa * location) * j;  
   
            if(powerOfAlpha != 0 && Zp[j] != 0)  
                emag = emag ^ alogp[(logp[Zp[j]] + powerOfAlpha)];  
        }  
  
        /* plug alpha**location into sigma'(X) */  
        /* sigma'(X) = sigma[1] + sigma[3]*X**2 + ...   
           + sigma[2j+1]*X**2j +...*/  
        elp = sigmap[1];  
        for(j=1;j<=elpMax;j++)  
        {  
            /* calculate power of alpha for this error */  
            powerOfAlpha = (poa * location) * (2 * j);  
  
            if(powerOfAlpha != 0 && sigmap[2*j+1] != 0)  
                elp = elp ^ alogp[(logp[sigmap[2*j+1]] + powerOfAlpha)];  
        }  
  
        /* adjust emag for symmetric code!!! */  
        emag = alogp[(int)((poa * (rsp.Mo - 1) * location) +
                        logp[emag]) % modulus];

        /* adjust elp for symmetric code!!! */  
        elp = alogp[((poa * location) + logp[elp])];  
  
        if(elp == 0)  
            return(UNCORRECTABLE_FLAG);  
   
        /* error_magnitude[i] = emag / elp  = emag * inverse(elp) */  
        /* take inverse of the error location polynomial prime */  
        powerOfAlpha = (codeWrdLength - (int)logp[elp]);  
  
        if(emag != 0 && powerOfAlpha != 0)  
            rsp.errorMagnitudes[i] = alogp[((int)logp[emag] + powerOfAlpha)];   
        else  
            return(UNCORRECTABLE_FLAG);  
    }  
  
    return(0);  
}  
  
/********************************************************  
*   Argument list:
*   Name          Definition              Use   Purpose
*   ------------  -----------             ----  --------------------------
*   r             register unsigned char* I/O   The received data vector.
*   intLev        int                     I     Interleave level to decode.
*   numErrors     int                     I     Number of correctable errors.
*  
*   Returns: Nothing  
*  
*   This function xor's the error magnitudes with the   
*   appropriate symbols in error, thus correcting all  
*   symbols in error.  
*  
*   actual error location in code block buffer =   
*            (rsp.modulus - errorLocations[i]) - 1  
*   This is due to R[254]=rs_data[0], etc and the -1 is because  
*   arrays go from 0..n-1, not 1..n  
*  
*   Note the adjustment needed to handle the new deinterleaving  
*   method.  Also, note that no adjustments are needed to handle  
*   virtual fill, since it is not possible to have errors in  
*   the virtual fill.  
*  
*********************************************************/  
void CReedSolomon::RSDCorrectSymbols(unsigned char *r, int intLev,int numErrors)  
{  
    int i, actuaLocation;  
    int modulus;  
    unsigned char *elp, *emagp;  
  
    /* put parameters into registers */  
    modulus = rsp.modulus;  
    elp = rsp.errorLocations;  
    emagp = rsp.errorMagnitudes;  
  
    for(i=0;i<numErrors;i++)  
    {  
        /* actual location within code block */  
        actuaLocation = ((modulus - elp[i]) - 1) - rsp.vf;
  
        /* actual location in buffer with interleaving adjustments */  
        actuaLocation = (actuaLocation * rsp.I) + intLev;

        /* correct error */  
        r[actuaLocation] = r[actuaLocation] ^ emagp[i];  
    }  
}  

//******************************************************************************
//345678901234567890123456789012345678901234567890123456789012345678901234567890
// 
