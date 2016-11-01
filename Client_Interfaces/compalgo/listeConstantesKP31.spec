--
-- listeConstanteKP31.spec
-- liste des constantes valides pour intéragir avec les unités System du mod
-- Kernel_Panic 3.1 compatible avec le moteur Spring
--

----------------
-- Coalitions --
----------------
constante MOI <Entier> = 0 ;      -- code de la coalition du joueur
constante ALLIE <Entier> = 1 ;    -- code de la coalition des alliés du joueur
constante ENNEMI <Entier> = 2 ;   -- code de la coalition des ennemis du joueur

----------------------------
-- Identifiant des unités --
----------------------------
constante ASSEMBLER <Entier> = 2;
constante BADBLOCK <Entier> = 3;
constante BIT <Entier> = 4;
constante BYTE <Entier> = 7;
constante KERNEL <Entier> = 24;
constante LOGICBOMB <Entier> = 25;
constante POINTER <Entier> = 30;
constante SIGNAL <Entier> = 33;
constante SOCKET <Entier> = 34;
constante TERMINAL <Entier> = 35;

-------------------------------------------------------------------------------
-- Ordres valables pour ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER et --
-------------------------------------------------------------------------------
constante STOP <Entier> = 0;
constante WAIT <Entier> = 5;
constante FIRESTATE <Entier> = 45;
constante SELFDESTRUCTION <Entier> = 65;
constante REPEAT <Entier> = 115;
---------------------------------------------------------------------------
-- Ordres valables pour ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET --
---------------------------------------------------------------------------
constante MOVE <Entier> = 10;
constante PATROL <Entier> = 15;
constante FIGHT <Entier> = 16;
constante GUARD <Entier> = 25;
constante MOVESTATE <Entier> = 50;
----------------------------------------------------------------------------
-- Ordres valables pour BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET --
----------------------------------------------------------------------------
constante ATTACK <Entier> = 20;
------------------------------------
-- Ordres valables pour ASSEMBLER --
------------------------------------
constante REPAIR <Entier> = 40;
constante RECLAIM <Entier> = 90;
constante RESTORE <Entier> = 110;
-------------------------------
-- Ordres valables pour BYTE --
-------------------------------
constante LAUNCHMINE <Entier> = 33395;
----------------------------------
-- Ordres valables pour POINTER --
----------------------------------
constante NXFLAG <Entier> = 33389;
-----------------------------------
-- Ordres valables pour TERMINAL --
-----------------------------------
constante SIGTERM <Entier> = 35126;
