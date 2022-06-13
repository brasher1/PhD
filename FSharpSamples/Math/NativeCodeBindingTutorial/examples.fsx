(* (c) Microsoft Corporation 2005-2006.  *)

#light
#nowarn "0049";; // turn off warnings about using upper case identifiers for variables (e.g. matrices)

#load "library.fs";;

open Microsoft.FSharp.Math.Bindings.LAPACK
open Microsoft.FSharp.Math.Notation
open Microsoft.FSharp.Math

// Ensure you have LAPACK.dll and BLAS.dll in the sample directory or elsewhere
// on your path. Here we set the current directory so we can find any local copies
// of LAPACK.dll and BLAS.dll.
System.Environment.CurrentDirectory <- __SOURCE_DIRECTORY__ 

// Here are some simple matrix values.
let onesM = matrix [ [ 1.0; 1.0];  
                      [1.0; 1.0] ]
let onesA = [| 1.0; 1.0 |]
let onesV = vector [ 1.0; 1.0]
let onesv = matrix [ [ 1.0] ; 
                     [ 1.0] ]
let twosM = matrix [ [ 2.0; 2.0]; 
                     [ 2.0; 2.0] ]
let twosV = vector [ 2.0; 2.0 ]
let twosv = matrix [ [ 2.0] ; 
                     [ 2.0] ]
let iM = matrix [ [ 1.0; 0.0]; 
                  [ 0.0; 1.0] ]

let miM = matrix [ [ -1.0; 0.0]; 
                   [  0.0; -1.0] ]

matrix [ [ 0.0; 0.0]; 
         [ 0.0; 0.0] ]

Matrix.identity 2

Matrix.identity 10

let A = 1

let show x = printf "%s\n" (any_to_string x)
printf " ------------------------\n" 

let J  = matrix [ [ 2.0; 3.0]; 
                  [ 4.0; 5.0] ]
let J2 = matrix [ [ 2.0; 3.0;4.0]; 
                  [ 2.0; 3.0;5.0]; 
                  [ 4.0; 5.0;6.0] ]

let ( $ ) (M : matrix) v = ImmutableMatrixRoutines.solve M v
let ( *. ) A B = ImmutableMatrixRoutines.mulMM A B
let ( *%. ) A v = ImmutableMatrixRoutines.mulMV A v

let Random = Matrix.create 500 500 1.0 |> Matrix.randomize
let RandomV = Matrix.create 500 1 1.0 |> Matrix.to_vector

let time s f =
  let sw = new System.Diagnostics.Stopwatch() 
  sw.Start();
  let res = f()
  printf "%A, time: %d\n" res sw.ElapsedMilliseconds;
  res
  
  
time "500x500 Matrix-MAtrix Multiplication using F# built-in: Random * Random" (fun () -> Random * Random )
time "500x500 Matrix-Matrix Multiplication using LAPACK.dll: Random *. Random" (fun () -> Random *. Random )
time "500x500 Matrix-Vector Multiplication using LAPACK.dll:  Random $ Random" (fun () -> Random $ RandomV )
time "Checking results correlate, err = " (fun () -> Vector.sum (Random * (Random $ RandomV) - RandomV))
time "Solving system of 500 equations in 500 variables, check with *%, err = " (fun () -> Vector.sum (Random *%. (Random $ RandomV) - RandomV))

time "computeEigenValues Random" (fun () -> ImmutableMatrixRoutines.computeEigenValues Random)

