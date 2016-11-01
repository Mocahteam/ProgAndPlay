--
-- listeConstantesKP41.spec
-- liste des constantes valides pour intéragir avec les unités System du mod
-- Kernel_Panic 4.1 compatible avec le moteur Spring
--

----------------
-- Coalitions --
----------------
constante MOI <Entier> = 0;      -- code de la coalition du joueur
constante ALLIE <Entier> = 1;    -- code de la coalition des alliés du joueur
constante ENNEMI <Entier> = 2;   -- code de la coalition des ennemis du joueur

----------------------------
-- Identifiant des unités --
----------------------------
constante ASSEMBLER <Entier> = 2;
constante BADBLOCK <Entier> = 3;
constante BIT <Entier> = 4;
constante BYTE <Entier> = 7;
constante KERNEL <Entier> = 25;
constante LOGICBOMB <Entier> = 26;
constante POINTER <Entier> = 39;
constante SIGNAL <Entier> = 44;
constante SOCKET <Entier> = 45;
constante TERMINAL <Entier> = 46;

-------------------------------------------------------------------------------
-- Ordres valables pour ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER et --
-- SOCKET                                                                    --
-------------------------------------------------------------------------------
constante STOP <Entier> = 0;              -- attend 0 paramètre
constante WAIT <Entier> = 5;              -- attend 0 paramètre
constante FIRESTATE <Entier> = 45;        -- attend 1 paramètre :
                                          --    0.0 => Hold fire
                                          --    1.0 => Return fire
                                          --    2.0 => Fire at will
constante SELFDESTRUCTION <Entier> = 65;  -- attend 0 paramètre
constante REPEAT <Entier> = 115;          -- attend 1 paramètre :
                                          --    0.0 => Repeat off
                                          --    1.0 => Repeat on
---------------------------------------------------------------------------
-- Ordres valables pour ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET --
---------------------------------------------------------------------------
constante MOVE <Entier> = 10;       -- attend 1 paramètre : une position ou
                                    -- une unité
constante PATROL <Entier> = 15;     -- attend 1 paramètre : une position ou
                                    -- une unité
constante FIGHT <Entier> = 16;      -- attend 1 paramètre : une position ou
                                    -- une unité
constante GUARD <Entier> = 25;      -- attend 1 paramètre : une position ou
                                    -- une unité
constante MOVESTATE <Entier> = 50;  -- attend 1 paramètre :
                                    --    0.0 => Hold pos
                                    --    1.0 => Maneuver
                                    --    2.0 => Roam
----------------------------------------------------------------------------
-- Ordres valables pour BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET --
----------------------------------------------------------------------------
constante ATTACK <Entier> = 20;   -- attend 1 paramètre : une position ou une
                                  -- unité
------------------------------------
-- Ordres valables pour ASSEMBLER --
------------------------------------
constante REPAIR <Entier> = 40;          -- attend 1 paramètre : une position
                                         -- ou une unité
constante RECLAIM <Entier> = 90;         -- attend 1 paramètre : une position
                                         -- ou une unité
constante RESTORE <Entier> = 110;        -- attend 1 paramètre : une position
                                         -- ou une unité
constante BUILDBADBLOCK <Entier> = -3;   -- attend 1 paramètre : une position
                                         -- ou une unité
constante BUILDLOGICBOMB <Entier> = -26; -- attend 1 paramètre : une position
                                         -- ou une unité
constante BUILDSOCKET <Entier> = -45;    -- attend 1 paramètre : une position
                                         -- ou une unité
constante BUILDTERMINAL <Entier> = -46;  -- attend 1 paramètre : une position
                                         -- ou une unité
constante DEBUG <Entier> = -35;          -- attend 1 paramètre : une position
                                         -- ou une unité
---------------------------------
-- Ordres valables pour KERNEL --
---------------------------------
constante BUILDASSEMBLER <Entier> = -2; -- attend 1 paramètre : une position
                                        -- ou une unité
constante BUILDBYTE <Entier> = -7;      -- attend 1 paramètre : une position
                                        -- ou une unité
constante BUILDPOINTER <Entier> = -39;  -- attend 1 paramètre : une position
                                        -- ou une unité
-------------------------------------------
-- Ordres valables pour KERNEL et SOCKET --
-------------------------------------------
constante BUILDBIT <Entier> = -4;        -- attend 1 paramètre : une position
                                         -- ou une unité
constante STOPBUILDING <Entier> = -7658; -- attend 0 paramètre
-------------------------------
-- Ordres valables pour BYTE --
-------------------------------
constante LAUNCHMINE <Entier> = 33395; -- attend 0 paramètre
----------------------------------
-- Ordres valables pour POINTER --
----------------------------------
constante NXFLAG <Entier> = 33389; -- attend 1 paramètre : une position ou une
                                   -- unité
-----------------------------------
-- Ordres valables pour TERMINAL --
-----------------------------------
constante SIGTERM <Entier> = 35126; -- attend 1 paramètre : une position ou
                                    -- une unité

--------------------------------
-- Identifiant des ressources --
--------------------------------
constante METAL <Entier> = 0;
constante ENERGY <Entier> = 1;
