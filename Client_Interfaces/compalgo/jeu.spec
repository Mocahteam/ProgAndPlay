-- jeu.spec
-- bibliothèque d'interaction avec le jeu de Compalgo
--

--------------------------
-- Définition des types --
--------------------------

-- une position représente un point précis sur la carte
type Position : enregistrement
    x <Réel>,  -- abcisse
    y <Réel> ; -- ordonnée

-- un tableau de paramètres
constante NBPARAMETRESMAX <Entier> = 3;
type TabParamètres : tableau [1 à NBPARAMETRESMAX] de <Réel>;

-- représentation d'une commande
type Commande : enregistrement
    code <Entier>, -- code de la commande
    nbParamètres <Entier>, -- nombre de paramètres associés à cette commande
    paramètres <TabParamètres> ; -- liste des paramètres associés à cette
                                 -- commande

-- un tableau de commande
constante NBCOMMANDEMAX <Entier> = 10;
type TabCommandes : tableau [1 à NBCOMMANDEMAX] de <Commande>;

-- représentation d'une unité
type Unité : enregistrement
    id <Entier>,         -- identifiant unique de l'unité
    coalition <Entier>,  -- coalition d'appartenance de l'unité : MOI, ENNEMI
                         -- ou ALLIE
    sorte <Entier>,      -- type de l'unité : BADBLOCK, LOGICBOMB, DEBUG,
                         -- KERNEL, SOCKET, TERMINAL, ASSEMBLER, BIT, BYTE,
                         -- POINTER
    position <Position>, -- position de l'unité sur la carte
    santé <Réel>,        -- niveau de santé de l'unité
    santéMax <Réel>,     -- santé maximum que peut atteindre l'unité
    groupe <Entier>,     -- groupe d'affectaction de l'unité
                         -- si groupe < 0 alors l'unité n'est affectée à aucun
                         -- groupe
    nbCommandeATraiter <Entier>,         -- nombre de commande en attente
    commandeATraiter <TabCommandes> ;    -- liste de commande en attente
                            

----------------------------------
-- Opérations de gestion du jeu --
----------------------------------

-- Ouvre le jeu
-- Cette procédure doit être appelée avant toute autre fonction du jeu sous
-- peine de résultats incohérents
procédure ouvrirJeu ;

-- Ferme le jeu
procédure fermerJeu ;

--------------------------------
-- Opérations générales du jeu --
--------------------------------

-- Indique si le jeu est terminé
fonction jeuEstTerminé retourne <Booléen> ;

-- Fournit la dimention de la carte de jeu sous la forme d'une Position (coin
-- inférieure droit)
fonction dimensionsCarte retourne <Position> ;

-- Fournit la position de départ de la partie
fonction positionDeDépart retourne <Position> ;

-- Fournit le niveau courant de la ressource id
fonction resourcesDisponibles (entrée id <Entier>) retourne <Entier> ;

----------------------------------------
-- Opérations sur les zones spéciales --
----------------------------------------

-- Retourne le nombre de zones spéciales
fonction nombreZonesSpéciales retourne <Entier> ;

-- Fournit les positions de la zone spéciale numéro zs
-- nécessite zs compris dans l'intervalle [1..n] 
-- où n est le nombre de zone constructible
fonction positionZoneSpéciale (entrée zs <Entier>) retourne <Position> ;

-----------------------------------
-- Opérations sur les coalitions --
-----------------------------------

-- Retourne le nombre d'unités de la coalition c
-- Seules les unités visibles par le joueur sont comptabilisées
-- nécessite c appartient à l'ensemble {MOI, ALLIE, ENNEMI)
fonction nombreUnités (entrée c <Entier>) retourne <Entier> ;

-- Retourne la ième unité visible de la coalition c
-- nécessite c appartient à l'ensemble {MOI, ALLIE, ENNEMI)
-- nécessite i compris dans l'intervalle [1..n] 
-- où n est le nombre d'unité de la coalition c
fonction ièmeUnité (entrée c <Entier>, entrée i <Entier>) retourne <Unité> ;

-------------------------------
-- Opérations sur les unités --
-------------------------------

-- Retourne VRAI si l'unité est visible par le joueur
fonction estVisible (entrée u <Unité>) retourne <Booléen> ;

-- Retourne VRAI si l'unité u est synchrone avec le dernier rafraîchissement du
-- jeu
-- nécessite u visible
fonction estSynchronisée (entrée u <Unité>) retourne <Booléen> ;

-- Synchronise l'unité u avec le dernier rafraîchissement du jeu
-- nécessite u visible
procédure synchroniserUnité (màj u <Unité>) ;

-- Ajoute l'unité u au groupe g
-- nécessite u visible
-- nécessite u dirigée par le joueur (u.coalition = MOI)
-- nécessite groupe >= 0
procédure affecterGroupe (entrée u <Unité>, entrée g <Entier>) ;

-- Retire l'unité u de son groupe d'appartenance
-- nécessite u visible
-- nécessite u dirigée par le joueur  (u.coalition = MOI)
procédure retirerGroupe (entrée u <Unité>) ;

-- L'unité u réalise l'action action sur l'unité cible cible
-- nécessite u unité visible
-- nécessite u dirigée par le joueur (u.coalition = MOI)
-- nécessite cible unité visible
-- nécessite action est une action basique {DEPLACER, PATROUILLER, COMBATTRE,
-- ATTAQUER, GARDER, REPARER, RECLAMER} ou une action spéciale {NXFALG,
-- SIGTERM} ou une action de construction {BUILDBADBLOCK, BUILDLOGICBOMB,
-- BUILDSOCKET, BUILDTERMINAL, DEBUG, BUILDASSEMBLER, BUILDBYTE, BUILDPOINTER,
-- BUILDBIT}
procédure réaliserActionSurUnité (entrée u <Unité>, entrée action <Entier>,
    entrée cible <Unité>) ;

-- L'unité u réalise l'action action sur la position pos
-- nécessite u unité visible
-- nécessite u dirigée par le joueur (u.coalition = MOI)
-- nécessite action est une action basique {DEPLACER, PATROUILLER, COMBATTRE,
-- ATTAQUER, GARDER, RESTORER} ou une action spéciale {NXFALG, SIGTERM} ou une
-- action de construction {BUILDBADBLOCK, BUILDLOGICBOMB, BUILDSOCKET,
-- BUILDTERMINAL, DEBUG, BUILDASSEMBLER, BUILDBYTE, BUILDPOINTER, BUILDBIT}
procédure réaliserActionSurPosition (entrée u <Unité>, entrée action <Entier>,
    entrée pos <Position>) ;

-- L'unité u réalise l'action non ciblé action avec comme paramètre param
-- nécessite u unité visible
-- nécessite u dirigée par le joueur (u.coalition = MOI)
-- nécessite action est une action basique {WAIT, FIRESTATE, SELFDESTRUCTION,
-- REPEAT, MOVESTATE} ou une action spéciale {LAUNCHMINES, STOPBUILDING}
procédure réaliserActionNonCiblée (entrée u <Unité>, entrée action <Entier>,
    entrée param <Réel>) ;

-------------------------------
-- Opérations additionnelles --
-------------------------------

-- Retourne un nombre aléatoire compris entre 0.0 et limite
fonction hasard (entrée limite <Réel>) retourne <Réel> ;

-- Effectue une pause de t milisecondes
procédure délai (entrée t <Entier>) ;

