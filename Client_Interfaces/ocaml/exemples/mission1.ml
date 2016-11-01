#load "pp.cma";;
#use "constantListKP41.ml";;

(*
 * MISSION 1
 *)
(* Déplace la première unité à la bonne coordonnée *)
let mission1 = function () -> Pp.actionOnPosition ((List.hd (Pp.getEntities Pp.MyCoalition)), aMove, (1983.0, 1279.0));;
