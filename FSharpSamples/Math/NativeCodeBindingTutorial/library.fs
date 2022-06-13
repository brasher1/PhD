//----------------------------------------------------------------------------
// (c) Microsoft Corporation 2005-2008.  
//
// Tutorial Part 1. Understanding interfacing between F# and C/Fortran code
// Tutorial Part 2. Binding to LAPACK.
// Tutorial Part 3. Managing Fortran v. C Matrix Layout
// Tutorial Part 4. Pinning F# Matrix and Vector objects
// Tutorial Part 5. Building some high-level mutating routines 
// Tutorial Part 6. Building some easy-to-use high-level applicative routines 
// Tutorial Part 7. Reusing the primitive bindings on other similarly shaped data structures

#light


namespace Microsoft.FSharp.Math.Bindings.LAPACK

#nowarn "0044";;  // suppress warnings about the use of native pointer features to interoperate with native code
#nowarn "0051";;  // suppress warnings about the use of byref features to interoperate with native code
#nowarn "0049";;  // turn off warnings about using upper case identifiers for variables (e.g. matrices)

//----------------------------------------------------------------------------
/// Tutorial Part 1. Understanding interfacing between F# and C/Fortran code
///
///  The basic technique to interface to a native C or Fortran routine is as follows:
///
///      1. Write your DllImport stub.  This usually involves writing or copying a
///         specification of the C prototype for the native function and marking it 'extern'.
///         See examples below.
///
///           - The C syntax can be used for the signature, including for types.
///                   'int *' ==   'int nativeptr' 
///                   'int []' ==  'int []' 
///                   'int &' ==   'int byref' 
///                   'void *' ==  'nativeint' 
///
///           - Full tutorials can be found in material on C# programming 
///
///           - If the function takes a callback then you may have to specify a delegate
///             type that gives the signature of the callback.
///
///      2. Write a simple F# function that abstracts some of the complexity of your 
///         DllImport stubs.  For example:
///
///            - You will need to make locally mutable versions of some arguments that
///              are passed by pointer.  (F# doesn't let you take the address of a non-mutable
///              value).
///
///            - Pairs of arguments e.g. "int n, double *a" are often passed to specify
///              the size of an array and a pointer to the first element of the array. These
///              can be abstracted to a single argument of type
///              Microsoft.FSharp.NativeInterop.NativeArray. 
///
///            - If passing 2-dimensional arrays then you can similarly abstract the arguments to
///              be values of type Microsoft.FSharp.NativeInterop.NativeArray2.   This is
///              a synonym for the type Microsoft.FSharp.NativeInterop.CMatrix.
///              If the array is in column-major (Fortran) format then it is very clarifying 
///              to write your routine so it uses Microsoft.FSharp.NativeInterop.FortranMatrix instead.
///
///
///      3. Values such as Microsoft.FSharp.NativeInterop.NativeArray are simple wrappers around a C
///         pointer.  You don't want anyone using these directly.  This means you will need to 
///         write an F# function that accepts proper F# data structures (e.g. int[], int[,], matrix
///         vector etc.) and pins these data structures. Microsoft.FSharp.NativeInterop namespace has
///         many helpful functions to let you pin objects and take address of important elements.
///         You can also convert, e.g.
///                
///
///      4. Write higher level routines using these primitives. If the C routines mutate data then
///         write wrappers that copy the data defensively where necessary.  
///
///
/// There are many advanced topics related to working with C and LAPACK that are not covered here.
/// For example, native math libraries like to have workspaces, and much more efficient routines
/// can be programmed if you are willing to pre-calculate the required workspace size and pass the
/// allocated 



//----------------------------------------------------------------------------
/// Tutorial Part 2. Binding to LAPACK.
///
/// There are several dimensions of complexity here.  LAPACK routines are named in the form XYYZZZ where
///     X is the data type
///     YY indicates the kind of the matrix, or the kind of the most significant matrix
///     ZZZ indicates the computation performed
///
///   S: single/float32
///   D: double/float/float64
///   C: complex stored in 8 bytes, stored flat
///   Z: complex stored in 16 bytes (double complex)
///
/// In this sample we only bind to a handful of sample D routines.
///
/// A list of matrix types can be found at http://www.netlib.org/lapack/lug/node24.html
/// A list of operation names can be found at 
///     Linear Algebra: http://www.netlib.org/lapack/lug/node38.html 
///     Factorizations, Linear Least Square, 
///     Eigenpoblems, Singular Value Decompositions: http://www.netlib.org/lapack/lug/node37.html#seccomp

open System.Runtime.InteropServices
open System.Diagnostics
open Microsoft.FSharp.NativeInterop
open Microsoft.FSharp.Math
open System

module PrimitiveBindings = 

    /// LAPACK/BLAS primitive matrix/matrix multiply routine
    [<DllImport(@"blas.dll",EntryPoint="dgemm_")>]
    extern void DoubleMatrixMultiply_(char* transa, char* transb, int* m, int* n, int *k,
                                      double* alpha, double* A, int* lda,double* B, int* ldb,
                                      double* beta,
                                      double* C, int* ldc);

    ///  C := alpha*op( A )*op( B ) + beta*C
    let DoubleMatrixMultiply trans alpha (A: FortranMatrix<double>) (B: FortranMatrix<double>) beta (C: FortranMatrix<double>) = 
        // Mutable is needed because F# only lets you take pointers to mutable values
        let mutable trans = trans  // nb. unchanged on exit 
        let mutable beta = beta 
        let mutable alpha = alpha 
        let mutable m = A.NumCols 
        let mutable n = B.NumRows 
        let mutable k = A.NumRows 
        let mutable lda = A.NumCols 
        // Call the BLAS/LAPACK routine
        DoubleMatrixMultiply_(&&trans, &&trans, &&m, &&n, &&k, &&alpha, A.Ptr, &&lda, B.Ptr, &&k, &&beta, C.Ptr, &&m)

    /// LAPACK/BLAS primitive matrix/vector multiply routine
    [<DllImport(@"blas.dll",EntryPoint="dgemv_")>]
    extern void DoubleMatrixVectorMultiply_(char* trans, int* m, int* n,
                                            double* alpha, double* A, int* lda,
                                            double* x, int* incx, double* beta,
                                            double* y, int* incy);

    let DoubleMatrixVectorMultiply trans alpha (A: FortranMatrix<double>) (B: NativeArray<double>) beta (C: NativeArray<double>) = 
        let mutable trans = trans
        let mutable beta = beta 
        let mutable alpha = alpha 
        let mutable m = A.NumCols 
        let mutable n = A.NumRows 
        let mutable i_one = 1
        // Call the BLAS/LAPACK routine
        DoubleMatrixVectorMultiply_(&&trans, &&m, &&n, &&alpha, A.Ptr, &&m, B.Ptr, &&i_one, &&beta, C.Ptr, &&i_one)


    [<DllImport(@"lapack.dll", EntryPoint="dgetrf_")>]
    extern void DoublePLUDecomposition_(int *m, int *n, double *a, int* lda, int *ipiv, int *info);

    let DoublePLUDecomposition (A : FortranMatrix<double>) (ipiv : NativeArray<int>) = 
        let mutable m = A.NumCols
        let mutable n = A.NumRows
        let mutable info = 0
        let mutable lda = A.NumCols
        DoublePLUDecomposition_(&&m, &&n, A.Ptr, &&lda, ipiv.Ptr, &&info);
        match info with 
        | -1 -> invalid_arg "m"
        | -2 -> invalid_arg "n"
        | -3 -> invalid_arg "A"
        | -4 -> invalid_arg "lda"
        | -5 -> invalid_arg "ipiv"
        | -6 -> invalid_arg "info"
        | 0 -> ()
        | n -> invalid_arg (sprintf "singular: U(%d,%d) is zero" n n)


    [<DllImport(@"lapack.dll", EntryPoint="dgetrs_")>]
    extern void DoubleSolveAfterPLUDecomposition_(char *trans, int *n, int *nrhs, double *a, int *lda, int *ipiv, double*b, int * ldb, int*info)

    let DoubleSolveAfterPLUDecomposition trans (A : FortranMatrix<double>) (B : FortranMatrix<double>) (ipiv : NativeArray<int>) = 
        let mutable trans = trans
        let mutable n = A.NumRows
        let mutable nrhs = B.NumCols
        let mutable lda = n
        let mutable ldb = n
        let mutable info = 0
        DoubleSolveAfterPLUDecomposition_(&&trans, &&n, &&nrhs, A.Ptr, &&lda, ipiv.Ptr, B.Ptr, &&ldb, &&info);
        match info with 
        | -1 -> invalid_arg "trans"
        | -2 -> invalid_arg "n"
        | -3 -> invalid_arg "nrhs"
        | -4 -> invalid_arg "A"
        | -5 -> invalid_arg "lda"
        | -6 -> invalid_arg "ipiv"
        | -7 -> invalid_arg "B"
        | -8 -> invalid_arg "ldb"
        | -9 -> invalid_arg "info"
        | _ -> ()

    [<DllImport(@"lapack.dll", EntryPoint="dgeev_")>]
    extern void DoubleComputeEigenValuesAndVectors_(char *jobvl, char *jobvr, int *n, double *a, int *lda, double *wr, double *wi, double *vl, 
                                                    int *ldvl, double *vr, int *ldvr, double*work, int *lwork, int*info);

    let DoubleComputeEigenValuesAndVectors jobvl jobvr (A : FortranMatrix<double>) (WR : NativeArray<double>) (WI : NativeArray<double>) (VL : FortranMatrix<double>) (VR : FortranMatrix<double>) (workspace : NativeArray<double>) = 
        let mutable jobvl = jobvl
        let mutable jobvr = jobvr
        let mutable lda = A.NumCols
        let mutable n = A.NumRows
        let mutable n = A.NumRows
        let mutable ldvl = VL.NumCols
        let mutable ldvr = VR.NumCols
        let mutable lwork = workspace.Length
        let mutable info = 0
        DoubleComputeEigenValuesAndVectors_(&&jobvl, &&jobvr, &&n, A.Ptr, &&lda, WR.Ptr, WI.Ptr, VL.Ptr, &&ldvl, VR.Ptr, &&ldvr,workspace.Ptr, &&lwork, &&info);
        match info with 
        | -1 -> invalid_arg "jobvl"
        | -2 -> invalid_arg "jobvr"
        | -3 -> invalid_arg "n"
        | -4 -> invalid_arg "A"
        | -5 -> invalid_arg "lda"
        | -6 -> invalid_arg "wr"
        | -7 -> invalid_arg "wi"
        | -8 -> invalid_arg "vl"
        | -9 -> invalid_arg "ldvl"
        | -10 -> invalid_arg "vr"
        | -11 -> invalid_arg "ldvr"
        | -12 -> invalid_arg "work"
        | -13 -> invalid_arg "lwork"
        | -14 -> invalid_arg "info"
        | _ -> ()

    let DoubleComputeEigenValuesAndVectorsWorkspaceSize jobvl jobvr (A : FortranMatrix<double>)  = 
        let mutable jobvl = jobvl
        let mutable jobvr = jobvr
        let mutable lda = A.NumCols
        let mutable n = A.NumRows
        let mutable ldvl = n
        let mutable ldvr = n
        let mutable lwork = -1
        let mutable workspaceSize = 0
        let mutable info = 0
        printf "DoubleComputeEigenValuesAndVectorsWorkspaceSize\n" ;
        DoubleComputeEigenValuesAndVectors_(&&jobvl, &&jobvr, &&n, A.Ptr, &&lda, A.Ptr, A.Ptr, A.Ptr, &&ldvl, A.Ptr, &&ldvr,NativePtr.of_nativeint (NativePtr.to_nativeint (&&workspaceSize)), &&lwork, &&info);
        printf "workspaceSize = %d\n" workspaceSize;
        match info with 
        | -1 -> invalid_arg "jobvl"
        | -2 -> invalid_arg "jobvr"
        | -3 -> invalid_arg "n"
        | -4 -> invalid_arg "A"
        | -5 -> invalid_arg "lda"
        | -6 -> invalid_arg "wr"
        | -7 -> invalid_arg "wi"
        | -8 -> invalid_arg "vl"
        | -9 -> invalid_arg "ldvl"
        | -10 -> invalid_arg "vr"
        | -11 -> invalid_arg "ldvr"
        | -12 -> invalid_arg "work"
        | -13 -> invalid_arg "lwork"
        | -14 -> invalid_arg "info"
        | _ -> workspaceSize


//----------------------------------------------------------------------------
/// Tutorial Part 3. LAPACK accepts Fortran matrices, though often permits flags 
/// to view the input matrices in a transposed way.  This is a pain. Here we
/// use some implicit transpose trickery and transpose settings to give 
/// a view of these operations oeprating over CMatrix values.  Note that no actual
/// copying of matrix data occurs.

module PrimitiveCMatrixBindings = 

     /// Here we builda  version that operates on row-major data
    let DoubleMatrixMultiply alpha (A: CMatrix<double>) (B: CMatrix<double>) beta (C: CMatrix<double>) = 
        Debug.Assert(A.NumCols = B.NumRows);
        // Lapack is column-major, so we give it the implicitly transposed matrices and reverse their order:
        // C <- A*B   ~~> C' <- (B'*A')
        PrimitiveBindings.DoubleMatrixMultiply 'n' alpha B.NativeTranspose A.NativeTranspose beta C.NativeTranspose

    let DoubleMatrixVectorMultiply alpha (A: CMatrix<double>) (B: NativeArray<double>) beta (C: NativeArray<double>) = 
        Debug.Assert(A.NumCols = B.Length);
        // Lapack is column-major, so we tell it that A is transposed. The 't' and the A.NativeTranspose effectively cancel.
        // C <- A*B   ~~> C <- (A''*B)
        PrimitiveBindings.DoubleMatrixVectorMultiply 't' alpha A.NativeTranspose B beta C

    let DoubleSolveAfterPLUDecomposition (A : CMatrix<double>) (B : FortranMatrix<double>) (ipiv : NativeArray<int>) = 
        Debug.Assert(A.NumRows = A.NumCols);
        Debug.Assert(A.NumCols = B.NumRows);
        Debug.Assert(ipiv.Length = A.NumRows);
        // Lapack is column-major, so we solve A' X = B.  
        PrimitiveBindings.DoubleSolveAfterPLUDecomposition 'T' A.NativeTranspose B ipiv
        
    let DoubleComputeEigenValues (A: CMatrix<double>) (WR: NativeArray<double>) (WI: NativeArray<double>) (workspace: NativeArray<double>) = 
        Debug.Assert(A.NumCols = A.NumRows);
        Debug.Assert(A.NumCols = WR.Length);
        Debug.Assert(A.NumCols = WI.Length);
        Debug.Assert(workspace.Length >= max 1 (3*A.NumCols));
        let dummy = A.NativeTranspose in 
        // Lapack is column-major, but the eigen values of the transpose are the same as the eigen values of A
        // C <- A*B   ~~> C <- (A''*B)
        PrimitiveBindings.DoubleComputeEigenValuesAndVectors 'v' 'v' A.NativeTranspose WR WI dummy dummy workspace
      
    let DoubleComputeEigenValuesAndVectors (A: CMatrix<double>) (WR: NativeArray<double>) (WI: NativeArray<double>) (VR: FortranMatrix<double>) (workspace: NativeArray<double>) = 
        Debug.Assert(A.NumCols = A.NumRows);
        Debug.Assert(A.NumCols = WR.Length);
        Debug.Assert(A.NumCols = WI.Length);
        Debug.Assert(workspace.Length >= max 1 (3*A.NumCols));
        let dummy = A.NativeTranspose in 
        // Lapack is column-major, but the eigen values of the transpose are the same as the eigen values of A
        // C <- A*B   ~~> C <- (A''*B)
        PrimitiveBindings.DoubleComputeEigenValuesAndVectors 'n' 'v' A.NativeTranspose WR WI dummy VR workspace
      
    let DoubleComputeEigenValuesWorkspace (A: CMatrix<double>) = 
        Debug.Assert(A.NumCols = A.NumRows);
        let dummy = A.NativeTranspose in 
        // Lapack is column-major, but the eigen values of the transpose are the same as the eigen values of A
        // C <- A*B   ~~> C <- (A''*B)
        PrimitiveBindings.DoubleComputeEigenValuesAndVectorsWorkspaceSize 'n' 'n' A.NativeTranspose 
        

//----------------------------------------------------------------------------
/// Tutorial Part 4. To pass F# data structures to C and Fortran you need to
/// pin the underlying array objects.  This can be done entirely in F# code.
///

module NativeUtilities = 
    let nativeArray_as_CMatrix_colvec (arr: 'a NativeArray) =
       new CMatrix<_>(arr.Ptr,arr.Length,1)
       
    let nativeArray_as_FortranMatrix_colvec (arr: 'a NativeArray) =
       new FortranMatrix<_>(arr.Ptr,arr.Length,1)
       
    (* Functions to pin and free arrays *)
    let pinM m = PinnedArray2.of_matrix(m)
    let pinV v = PinnedArray.of_vector(v)
    let pinA arr = PinnedArray.of_array(arr)
    let pinMV m1 v2 = pinM m1,pinV v2
    let pinVV v1 v2 = pinV v1,pinV v2
    let pinAA v1 v2 = pinA v1,pinA v2
    let pinMVV m1 v2 m3 = pinM m1,pinV v2,pinV m3
    let pinMM m1 m2  = pinM m1,pinM m2
    let pinMMM m1 m2 m3 = pinM m1,pinM m2,pinM m3
    let freeM (pA: 'a PinnedArray2) = pA.Free()
    let freeV (pA: 'a PinnedArray) = pA.Free()
    let freeA (pA: 'a PinnedArray) = pA.Free()
    let freeMV ((pA: 'a PinnedArray2),(pB : 'a PinnedArray)) = pA.Free(); pB.Free()
    let freeVV ((pA: 'a PinnedArray),(pB : 'a PinnedArray)) = pA.Free(); pB.Free()
    let freeAA ((pA: 'a PinnedArray),(pB : 'a PinnedArray)) = pA.Free(); pB.Free()
    let freeMM ((pA: 'a PinnedArray2),(pB: 'a PinnedArray2)) = pA.Free();pB.Free()
    let freeMMM ((pA: 'a PinnedArray2),(pB: 'a PinnedArray2),(pC: 'a PinnedArray2)) = pA.Free();pB.Free();pC.Free()
    let freeMVV ((pA: 'a PinnedArray2),(pB: 'a PinnedArray),(pC: 'a PinnedArray)) = pA.Free();pB.Free();pC.Free()



//----------------------------------------------------------------------------
/// Tutorial Part 5. Higher level in-place bindings that operate mutatively over the F#
/// Matrix type by first pinning the data structures.  Be careful with these operations,
/// as they will write some results into your input matrices.
///

module MutableMatrixRoutines = 

    open NativeUtilities

    /// C <- A * B
    let mulMM (A:matrix) (B:matrix) (C:matrix) = 
        let (pA,pB,pC) as ptrs = pinMMM A B C
        try PrimitiveCMatrixBindings.DoubleMatrixMultiply 1.0 pA.NativeArray pB.NativeArray 0.0 pC.NativeArray
        finally
            freeMMM ptrs

    /// C <- A * V
    let mulMV (A:matrix) (B:vector) (C:vector) = 
        let pA,pB,pC as pin = pinMVV A B C
        try PrimitiveCMatrixBindings.DoubleMatrixVectorMultiply 1.0 pA.NativeArray pB.NativeArray 0.0 pC.NativeArray
        finally
            freeMVV pin

    /// B <- A \ B
    let solve (A : matrix) (B: vector) (ipiv: int[]) = 
        let pA = pinM A
        let pB = pinV B
        let pPivots = pinA ipiv
        try 
          PrimitiveBindings.DoublePLUDecomposition pA.NativeArray.NativeTranspose pPivots.NativeArray;
          PrimitiveCMatrixBindings.DoubleSolveAfterPLUDecomposition pA.NativeArray (nativeArray_as_FortranMatrix_colvec pB.NativeArray) pPivots.NativeArray
        finally
            freeM pA; freeV pB; freeA pPivots

    let computeEigenValues (A : matrix) (WR: double[]) (WI:double[]) (workspace:double[])  = 
        let pA = pinM A
        let pWR,pWI as pWRI = pinAA WR WI
        let pWorkspace = pinA workspace
        try 
          PrimitiveCMatrixBindings.DoubleComputeEigenValues pA.NativeArray pWR.NativeArray pWI.NativeArray pWorkspace.NativeArray 
        finally
            freeM pA; freeVV pWRI; freeA pWorkspace
            
    let computeEigenValuesAndVectors (A : matrix) (WR: double[]) (WI:double[]) (VR: matrix) (workspace:double[])  = 
        let pA,pVR as pMM = pinMM A VR
        let pWR,pWI as pWRI = pinAA WR WI
        let pWorkspace = pinA workspace
        try 
          PrimitiveCMatrixBindings.DoubleComputeEigenValuesAndVectors pA.NativeArray pWR.NativeArray pWI.NativeArray pVR.NativeArray.NativeTranspose pWorkspace.NativeArray 
        finally
            freeMM pMM; freeVV pWRI; freeA pWorkspace
            
//----------------------------------------------------------------------------
// Tutorial Part 6. Higher level bindings that defensively copy their input
//

module ImmutableMatrixRoutines = 

    open NativeUtilities

    /// Compute A * B
    let mulMM (A:matrix) (B:matrix) = 
        let C = Matrix.zero A.NumRows B.NumCols
        // C <- A * B
        MutableMatrixRoutines.mulMM A B C;
        C

    /// Compute A * v
    let mulMV (A:matrix) (v:vector) = 
        let C = Vector.zero A.NumRows
        // C <- A * v
        MutableMatrixRoutines.mulMV A v C;
        C

    /// Compute A \ v
    let solve (A : matrix) (v: vector) = 
        Debug.Assert(A.NumRows = A.NumCols);
        let A = Matrix.copy A // workspace (yuck) 
        let vX = Vector.copy v 
        let ipiv = Array.zero_create A.NumCols 
        // vX <- A \ v
        MutableMatrixRoutines.solve A vX ipiv;
        vX

    let computeEigenValues (A : matrix)  = 
        let At = A.Transpose
        let n = At.NumRows
        let WR = Array.zero_create n
        let WI = Array.zero_create n
        let workspace = Array.zero_create (5 * n (* computeEigenValuesWorkspace At *) ) 
        MutableMatrixRoutines.computeEigenValues At WR WI workspace;
        let W = Vector.Generic.init n (fun i -> Complex.mkPolar (WR.[i],WI.[i])) 
        W
             
    let computeEigenValuesAndVectors (A : matrix)  = 
        let At = A.Transpose
        let n = At.NumRows
        let WR = Array.zero_create n
        let WI = Array.zero_create n
        let VR = Matrix.zero n n
        let workspace = Array.zero_create (5 * n (* computeEigenValuesAndVectorsWorkspace A *) ) 
        MutableMatrixRoutines.computeEigenValuesAndVectors At WR WI VR workspace;
        let W = Vector.Generic.init n (fun i -> Complex.mkPolar (WR.[i],WI.[i])) 
        W,VR.Transpose

//----------------------------------------------------------------------------
// Tutorial Part 7. Reusing primitive bindings on other similarly shapped data structures
//
// Here we show that the same NativeArray/CMatrix/FortranMatrix primitive bindings can be used
// with other data structures where the underlying bits are ultimately stored as shape double[] and double[,],
// e.g. they can be used directly on arrays, or even on memory allocated in C.

module MutableArrayRoutines = 
    open NativeUtilities
    let pinA2 arr2 = PinnedArray2.of_array2(arr2)
    let pinA2AA m1 v2 m3 = pinA2 m1,pinA v2,pinA m3
    let freeA2A2A2 ((pA: 'a PinnedArray2),(pB: 'a PinnedArray2),(pC: 'a PinnedArray2)) = pA.Free();pB.Free();pC.Free()
    let freeA2AA ((pA: 'a PinnedArray2),(pB: 'a PinnedArray),(pC: 'a PinnedArray)) = pA.Free();pB.Free();pC.Free()
    let pinA2A2A2 m1 m2 m3 = pinA2 m1,pinA2 m2,pinA2 m3
    let freeA2 (pA: 'a PinnedArray2) = pA.Free()

    let mulMM (A: double[,]) (B: double[,]) (C: double[,]) = 
        let (pA,pB,pC) as ptrs = pinA2A2A2 A B C
        try PrimitiveCMatrixBindings.DoubleMatrixMultiply 1.0 pA.NativeArray pB.NativeArray 0.0 pC.NativeArray
        finally
            freeMMM ptrs

    let mulMV (A: double[,]) (B: double[]) (C: double[]) = 
        let pA,pB,pC as pin = pinA2AA A B C
        try PrimitiveCMatrixBindings.DoubleMatrixVectorMultiply 1.0 pA.NativeArray pB.NativeArray 0.0 pC.NativeArray
        finally
            freeA2AA pin
    
    let solve (A : double[,]) (B: double[]) (ipiv: int[]) = 
        let pA = pinA2 A
        let pB = pinA B
        let pPivots = pinA ipiv
        try 
          PrimitiveBindings.DoublePLUDecomposition pA.NativeArray.NativeTranspose pPivots.NativeArray;
          PrimitiveCMatrixBindings.DoubleSolveAfterPLUDecomposition pA.NativeArray (nativeArray_as_FortranMatrix_colvec pB.NativeArray) pPivots.NativeArray
        finally
            freeA2 pA; freeA pB; freeA pPivots
    
module ImmutableArrayRoutines = 
    let mulMM (A:double[,]) (B:double[,]) = 
        let C = Array2.zero_create (Array2.length1 A) (Array2.length2 B)
        MutableArrayRoutines.mulMM A B C;
        C

    let mulMV (A:double[,]) (B:double[]) = 
        let C = Array.zero_create (Array2.length1 A)
        MutableArrayRoutines.mulMV A B C;
        C
    let solve (A : double[,]) (B: double[]) = 
        Debug.Assert(Array2.length1 A = Array2.length2 A);
        let A = Array2.init (Array2.length1 A) (Array2.length2 A) (fun i j ->  A.[i,j]) 
        let BX = Array.copy B 
        let ipiv = Array.zero_create (Array2.length1 A)
        MutableArrayRoutines.solve A BX ipiv;
        BX



