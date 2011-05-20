//       1         2         3         4         5         6         7         8
//345678901234567890123456789012345678901234567890123456789012345678901234567890
//******************************************************************************
// File: Reed Solomon.hpp
//
//    This is the header file for the Reed Solomon Base Class.
//
// Version |    Date    | Author          | Notes
// --------|------------|-----------------|-------------------------------------
//   1.0   | 04/01/2002 | BAZ             | Program Created
//         |            |                 |
//******************************************************************************

// only include header file once
#ifndef REED_SOLOMON_HEADER
#define REED_SOLOMON_HEADER

class CReedSolomon
{
// define constants
    enum { UNCORRECTABLE_FLAG=-1};

// define the member functions
public:
	// define constructors
	CReedSolomon(int BitsPerSymbol, int CorrectableErrors, int mo, int poa,
                 int VirtualFill, int Interleave, int FrameSyncLength, int mode);
            // Generator Polynomial:
            //
            //        2t-1           i+Mo                 POA
            // G(X) = PROD (X - gamma   ),   gamma = alpha
            //        i=0
            // frame_size = 4 + (((n - 2t) - vf) * I) + (2t * I) = length of frame incl fs
            // MODE 
            //    0 = Use conventional tables
            //    1 = Use dual tables

	// define destructor
	~CReedSolomon(); 

private:
    // prevent copying
    CReedSolomon(const CReedSolomon&);
    CReedSolomon& operator=(const CReedSolomon&);

// define overloaded operators
public:

// define member access functions
public:
    unsigned long CorrectableErrorsInFrame() const;
    void CorrectableErrorsPerInterleave(unsigned long *ErrorsPerInterleave) const;
        // this returns the number of errors per interleave where you must pass an array
        // of length Interleave to receive the results
    unsigned long UncorrectableErrorsInFrame() const;

// define member operation functions
public:
    bool Decode(unsigned char* rsDataFrame);
            // decode the data frame. returns true if no errors,
            // false if correctable or uncorrectable errors

// define helper functions
private:
    int RSDecode(unsigned char *rs_data, int intLev);
            // decode a specific interleave
    int RSDCalcSyndrome(register unsigned char *r, int intLev); 
            // This function implements the FFT-like syndrome calculation.
    int RSDCalcELPCoef();
            // The degree of the error location polynomial.
    int RSDCalcErrorLocations(int degreeOfSigma);
            // Calculate the roots of the error location polynomial by
            // using the Chien search.  
    int RSDCalcErrorMagnitudes(int numErrors);  
            // This function uses the Forney Algorithm to calculate the error
            // magnitudes.
    void RSDCorrectSymbols(unsigned char *r, int intLev,int numErrors);
            // This function xor's the error magnitudes with the appropriate
            // symbols in error, thus correcting all symbols in error.  

// define member variables
private:
    typedef struct rsd_params
    {
        int m; /* bits per RS symbol */
        int n; /* 2**m - 1, number of symbols in field */
        int t; /* number of correctable errors */
        int d; /* 2 * t */
        int k; /* (n - 2t) - vf */
        int Mo; /* see generator polynomial */
        int poa; /* see generator polynomial */
        int vf; /* number of bytes of virtual fill */
        int I; /* the depth of interleaving */
        int mode; /* specifies which table to use, dual or conventional */
        int modulus; /* n */
        int frameLength; /*  fsLength + (((n - 2t) - vf) * I) + (2t * I) */
        int fsLength; /* frame sync pattern length in bytes */

        unsigned long numErrorsPerFrame;
        unsigned long *numErrorsPerInterleave; /* the number of correctable errors in each interleave */
        unsigned long uncorErrsPerFrame;

        unsigned char *log_ptr; /* pointer Galois log table */
        unsigned char *antilog_ptr; /* pointer Galois antilog table */

        unsigned char *s; /* The syndrome polynomial */
        unsigned char *sigma; /* The error locator polynomial */
        unsigned char *errorMagnitudes; /* stores an error magnitudes */
        unsigned char *errorLocations; /* stores the error locations */

        /* local to CalcSyndrome() */
        int synLoopMax; /* used so that the same values does not need to
                            calculated everytime the function is called */

        /* local to CalcELPCoef() */
        unsigned char *D;
        unsigned char *tmpSigma;

        /* local to CalcErrorMagnitudes() */
        unsigned char *Z; /* error evaluator poly(A.K.A. Magnitude_Polynomial) */
    } RSD_PARAMS;
    RSD_PARAMS rsp; // the rees solomon parameters

    static unsigned char GF256[255], GF256d[255]; // the golais feild tables for n=8
};

#endif

//******************************************************************************
//345678901234567890123456789012345678901234567890123456789012345678901234567890
//       1         2         3         4         5         6         7         8
