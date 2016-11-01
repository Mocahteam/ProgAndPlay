(*
  List of constants for System units of Kernel Panic 3.1
*)

(*****************
 * Kind of units *
 *****************)
let idAssembler = 2;;
let idBadBlock = 3;;
let idBit = 4;;
let idByte = 7;;
let idKernel = 24;;
let idLogicBomb = 25;;
let idPointer = 37;;
let idSignal = 40;;
let idSocket = 41;;
let idTerminal = 42;;

(***************************************************************************
 * Actions available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER *
 * and SOCKET                                                              *
 ***************************************************************************)
let aStop = 0;;
let aWait = 5;;
let aFireState = 45;;
let aSelfDestruction = 65;;
let aRepeat = 115;;
(**************************************************************************
 * Actions available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
 **************************************************************************)
let aMove = 10;;
let aPatrol = 15;;
let aFight = 16;;
let aGuard = 25;;
let aMoveState = 50;;
(***************************************************************************
 * Actions available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
 ***************************************************************************)
let aAttack = 20;;
(***********************************
 * Actions available for ASSEMBLER *
 ***********************************)
let aRepair = 40;;
let aReclaim = 90;;
let aRestore = 110;;
(******************************
 * Actions available for BYTE *
 ******************************)
let aLaunchMine = 33395;;
(*********************************
 * Actions available for POINTER *
 *********************************)
let aNxFlag = 33389;;
(**********************************
 * Actions available for TERMINAL *
 **********************************)
let aSigterm = 35126;;

(****************
 * Resources id *
 ****************)
let rMetal = 0;;
let rEnergy = 1;;
