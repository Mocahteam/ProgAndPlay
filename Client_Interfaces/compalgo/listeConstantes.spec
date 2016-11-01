--
-- listeConstante.spec
-- liste des constantes valides pour intéragir avec le mod Kernel_Panic 2.2
-- Div0 compatible avec le moteur Spring
--

----------------
-- Coalitions --
----------------
constante MOI <Entier> = 0 ;      -- code de la coalition du joueur
constante ALLIE <Entier> = 1 ;    -- code de la coalition des alliés du joueur
constante ENNEMI <Entier> = 2 ;   -- code de la coalition des ennemis du joueur

---------------------
-- Ordres basiques --
---------------------
constante DEPLACER <Entier> = 10 ;       -- code de l'action déplacement
constante PATROUILLER <Entier> = 15 ;    -- code de l'action patrouille
constante COMBATTRE <Entier> = 16 ;      -- code de l'action combat
constante ATTAQUER <Entier> = 20 ;       -- code de l'action attaque
constante GARDER <Entier> = 25 ;         -- code de l'action garde
constante REPARER <Entier> = 40 ;        -- code de l'action réparation
constante RECLAMER <Entier> = 90 ;       -- code de l'action réclamation
constante RESTAURER <Entier> = 110 ;      -- code de l'action restauration

---------------------------------------
-- Identifiants des types des unités --
---------------------------------------
-- Commun à tout les camps
constante BADBLOCK <Entier> = 2 ;         -- code de l'unité BADBLOCK
constante LOGICBOMB <Entier> = 18 ;       -- code de l'unité LOGICBOMB
constante DEBUG <Entier> = 19 ;           -- code de l'unité DEBUG
-- Pour le camp System
constante KERNEL <Entier> = 17 ;          -- code de l'unité KERNEL
constante SOCKET <Entier> = 23 ;          -- code de l'unité SOCKET
constante TERMINAL <Entier> = 24 ;        -- code de l'unité TERMINAL
constante ASSEMBLER <Entier> = 1 ;        -- code de l'unité ASSEMBLER
constante BIT <Entier> = 3 ;              -- code de l'unité BIT
constante BYTE <Entier> = 6 ;             -- code de l'unité BYTE
constante POINTER <Entier> = 21 ;         -- code de l'unité POINTER
-- Pour le camp Hacker
constante HOLE <Entier> = 15 ;            -- code de l'unité HOLE
constante WINDOW <Entier> = 28 ;          -- code de l'unité WINDOW
constante OBELISK <Entier> = 20 ;         -- code de l'unité OBELISK
constante TROJAN <Entier> = 25 ;          -- code de l'unité TROJAN
constante VIRUS <Entier> = 27 ;           -- code de l'unité VIRUS
constante BUG <Entier> = 4 ;              -- code de l'unité BUG
constante EXPLOIT <Entier> = 12 ;         -- code de l'unité EXPLOIT
constante WORM <Entier> = 30 ;            -- code de l'unité WORM
constante DENIALOFSSERVICE <Entier> = 8 ; -- code de l'unité DENIALOFSERVICE


---------------------
-- Armes spéciales --
---------------------
constante NXFALG <Entier> = 33389 ;        -- code de l'arme spéciale du POINTER
constante LAUNCHMINES <Entier> = 33395 ;   -- code de l'arme spéciale du BYTE
constante SIGTERM <Entier> = 35126 ;       -- code de l'arme du TERMINAL
constante DEPLOY <Entier> = 33390 ;        -- code de l'action spéciale du BUG
constante BOMBARD <Entier> = 33394 ;       -- code de l'arme spéciale du BUG
constante UNDEPLOY <Entier> = 33391 ;      -- code de l'action spéciale de
                                           -- l'EXPLOIT
constante AUTOHOLD <Entier> = 32103 ;      -- code de l'arme de l'OBELISK
