#load "pp.cma";;
#use "constantListKP47.ml";;

(*
 * MISSION 1
 *)
(* Déplace la première unité à la bonne coordonnée *)
let mission1 = function () -> Pp.openConnexion() & Pp.actionOnPosition ((List.hd (Pp.getEntities Pp.MyCoalition)), aMove, (1983.0, 1279.0), false) & Pp.closeConnexion();;
