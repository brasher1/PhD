(* (c) Microsoft Corporation 2005-2008.  *)

#light

namespace Microsoft.FSharp.Math.Bindings.LAPACK

    open Microsoft.FSharp.Math

    module MutableMatrixRoutines =
      /// C <- A * B
      val mulMM : A: matrix -> B: matrix -> C: matrix -> unit
      /// vRes <- A * v
      val mulMV : A: matrix -> v: vector -> vRes: vector -> unit
      /// v <- A \ v
      val solve : A: matrix -> v: vector -> pivots: int [] -> unit

    module ImmutableMatrixRoutines =
      /// Compute A * B
      val mulMM : A: matrix -> B: matrix -> matrix
      /// Compute A * v
      val mulMV : A: matrix -> v: vector -> vector
      /// Compute A \ v
      val solve : A: matrix -> v: vector -> vector
      /// Compute the eigenvalues of the matrix A
      val computeEigenValues : A : matrix -> Vector<complex>
      /// Compute the eigenvalues of the matrix A
      val computeEigenValuesAndVectors : A : matrix -> Vector<complex> * matrix

    module MutableArrayRoutines =

      val mulMM : double [,] -> double [,] -> double [,] -> unit
      val mulMV : double [,] -> double [] -> double [] -> unit
      val solve : double [,] -> double [] -> pivots: int [] -> unit
    
    module ImmutableArrayRoutines =
    
      val mulMM : double [,] -> double [,] -> double [,]
      val mulMV : double [,] -> double [] -> double []
      val solve : double [,] -> double [] -> double []
