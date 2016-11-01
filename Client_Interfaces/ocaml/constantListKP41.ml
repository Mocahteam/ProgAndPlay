(*
  List of constants for System units of Kernel Panic 4.1
*)

(*****************
 * Kind of units *
 *****************)
let idAssembler = 2;;
let idBadBlock = 3;;
let idBit = 4;;
let idByte = 7;;
let idKernel = 25;;
let idLogicBomb = 26;;
let idPointer = 39;;
let idSignal = 44;;
let idSocket = 45;;
let idTerminal = 46;;

(***************************************************************************
 * Actions available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER *
 * and SOCKET                                                              *
 ***************************************************************************)
let aStop = 0;;             (* expect 0 parameters *)
let aWait = 5;;             (* expect 0 parameters *)
let aFireState = 45;;       (* expect 1 parameter:
                                  0.0 => Hold fire
                                  1.0 => Return fire
                                  2.0 => Fire at will *)
let aSelfDestruction = 65;; (* expect 0 parameters *)
let aRepeat = 115;;         (* expect 1 parameter:
                                  0.0 => Hold fire
                                  1.0 => Return fire
                                  2.0 => Fire at will *)

(**************************************************************************
 * Actions available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
 **************************************************************************)
let aMove = 10;;       (* expect 1 parameter: a position or a unit *)
let aPatrol = 15;;     (* expect 1 parameter: a position or a unit *)
let aFight = 16;;      (* expect 1 parameter: a position or a unit *)
let aGuard = 25;;      (* expect 1 parameter: a position or a unit *)
let aMoveState = 50;;  (* expect 1 parameter:
                             0.0 => Hold pos
                             1.0 => Maneuver
                             2.0 => Roam *)

(***************************************************************************
 * Actions available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
 ***************************************************************************)
let aAttack = 20;;    (* expect 1 parameter: a position or a unit *)

(***********************************
 * Actions available for ASSEMBLER *
 ***********************************)
let aRepair = 40;;          (* expect 1 parameter: a position or a unit *)
let aReclaim = 90;;         (* expect 1 parameter: a position or a unit *)
let aRestore = 110;;        (* expect 1 parameter: a position or a unit *)
let aBuildBadBlock = -3;;   (* expect 1 parameter: a position or a unit *)
let aBuildLogicBomb = -26;; (* expect 1 parameter: a position or a unit *)
let aBuildSocket = -45;;    (* expect 1 parameter: a position or a unit *)
let aBuildTerminal = -46;;  (* expect 1 parameter: a position or a unit *)
let aDebug = -35;;          (* expect 1 parameter: a position or a unit *)

(*******************************
 * Actions available for KERNEL *
 *******************************)
let aBuildAssembler = -2;;  (* expect 1 parameter: a position or a unit *)
let aBuildByte = -7;;       (* expect 1 parameter: a position or a unit *)
let aBuildPointer = -39;;   (* expect 1 parameter: a position or a unit *)

(******************************************
 * Actions available for KERNEL and SOCKET *
 ******************************************)
let aBuildBit = -4;;        (* expect 1 parameter: a position or a unit *)
let aStopBuilding = -7658;; (* expect 0 parameters *)

(******************************
 * Actions available for BYTE *
 ******************************)
let aLaunchMine = 33395;;  (* expect 0 parameters *)

(*********************************
 * Actions available for POINTER *
 *********************************)
let aNxFlag = 33389;;  (* expect 0 parameters *)

(**********************************
 * Actions available for TERMINAL *
 **********************************)
let aSigterm = 35126;;  (* expect 0 parameters *)

(****************
 * Resources id *
 ****************)
let rMetal = 0;;
let rEnergy = 1;;
