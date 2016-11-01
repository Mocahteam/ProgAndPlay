-- pp.spec
-- Interface avec les méthodes natives de la bibliothèque de
-- communication avec le jeu
--

---------------------------------------
-- Opération de gestion de Prog&Play --
---------------------------------------

-- Initialise la PP
-- Retourne 0 si l'ouverture c'est bien passée
-- Retourne -1 en cas d'erreur
fonction open retourne <Entier> ;

-- Ferme Prog&Play et libère les ressources allouées
-- Elle doit toujours être appelée avant la fin du programme
-- Cette fonction doit être la dernière des fonctions du PP à être appelée
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
fonction close retourne <Entier> ;

--------------------------------
-- Opération générales du jeu --
--------------------------------

-- Indique si le jeu est terminé
-- retourne une valeur > 0 si le jeu est terminé
-- retourne 0 si le jeu n'est pas terminé
-- retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction isGameOver retourne <Entier> ;

-- Fournit la taille de la carte sous forme de deux coordonnées x (abcisse) et
-- y (ordonnée) en cas de succès
-- en cas d'erreur, x = y = -1
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
procédure getMapSize (sortie x <Réel>, sortie y <Réel>) ;

-- Fournit sous forme de deux coordonnées x (abcisse) et y (ordonnée) la
-- position de départ du joueur courant en cas de succès
-- en cas d'erreur, x = y = -1
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
procédure getStartPosition (sortie x <Réel>, sortie y <Réel>) ;

-- Fournit le niveau courant d'une certaine ressource
-- Retourne le niveau courant de la ressource "id"
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction getResource (entrée id <Entier>) retourne <Entier> ;

----------------------------------------
-- Opérations sur les zones spéciales --
----------------------------------------

-- Retourne le nombre de zones spéciales du jeu en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction getNumSpecialAreas retourne <Entier> ;


-- Fournit la position sous forme de coordonnées x (abcisse) et y (ordonnée) de
-- la zone spéciale z en cas de succès
-- en cas d'erreur, x = y = -1
-- nécessite z doit être compris dans l'intervalle [0..n[ où n est le nombre de
-- zones spéciales
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
procédure getSpecialAreaPosition
    (entrée z <Entier>, sortie x <Réel>, sortie y <Réel>) ;

-----------------------------------
-- Opérations sur les coalitions --
-----------------------------------

-- Retourne le nombre d'unités visibles de la coalition c en cas de succès
-- Retourne -1 en cas de problème
-- nécessite c est une coalition valide : MOI, ENNEMI ou ALLIE
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction getNumUnits (entrée c <Entier>) retourne <Entier> ;

-- Retourne le numéro de la ième unité visible de la coalition c en cas de
-- succès
-- Retourne -1 en cas d'erreur
-- nécessite i compris dans l'intervalle [0..n[ où n est le nombre d'unité de
-- la coalition c
-- nécessite c est une coalition valide : MOI, ENNEMI ou ALLIE
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction getUnitAt (entrée c <Entier>, entrée i <Entier>) retourne <Entier> ;

-------------------------------
-- Opérations sur les unités --
-------------------------------

-- Retourne la coalition de l'unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetCoalition (entrée u <Entier>) retourne <Entier> ;

-- Retourne le type d'une unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetType (entrée u <Entier>) retourne <Entier> ;

-- Retourne la position de l'unité u sous forme de coordonnées x (abscisse) et
-- y (ordonnée) en cas de succès
-- en cas d'erreur, x = y = -1
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
procédure unitGetPosition (entrée u <Entier>,
                           sortie x <Réel>, sortie y <Réel>);

-- Retourne le niveau de santé de l'unité u en cas de succès
-- Retourne -1.0 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetHealth (entrée u <Entier>) retourne <Réel> ;


-- Retourne le niveau de santé maximum que l'unité u peut atteindre en cas de
-- succès
-- Retourne -1.0 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetMaxHealth (entrée u <Entier>) retourne <Réel> ;

-- Retourne le numéro du groupe dont fait partie l'unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- Retourne -2 si l'unité n'est affectée à aucun groupe
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetGroup (entrée u <Entier>) retourne <Entier> ;

-- Donne l'ordre à l'unité u de s'affecter au groupe g
-- Si g = -1 alors l'unité est retirée de son groupe
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'echec
-- nécessite u soit dirigée par le joueur
-- nécessite g >= -1 
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitSetGroup (entrée u <Entier>, entrée g <Entier>)
retourne <Entier> ;

-- Donne l'ordre à l'unité u de se retirer de son groupe courant
-- nécessite u soit dirigée par le joueur
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitRemoveFromGroup (entrée u <Entier>) retourne <Entier> ;

-- Donne l'ordre à l'unité u de réaliser l'action action sur l'unité cible cible
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite action est une action basique {DEPLACER, PATROUILLER, COMBATTRE,
-- ATTAQUER, GARDER, REPARER, RECLAMER} ou une action spéciale {NXFALG,
-- SIGTERM} ou une action de construction {-BADBLOCK, -LOGICBOMB, -DEBUG,
-- -KERNEL, -SOCKET, -TERMINAL, -ASSEMBLER, -BIT, -BYTE, -POINTER}
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitActionOnUnit
    (entrée u <Entier>, entrée action <Entier>, entrée cible <Entier>)
retourne <Entier> ;

-- Donne l'ordre à l'unité u de réaliser l'action action à la position (x, y)
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite action est une action basique {DEPLACER, PATROUILLER, COMBATTRE,
-- ATTAQUER, GARDER, RESTORER} ou une action spéciale {NXFALG, SIGTERM} ou une
-- action de construction {-BADBLOCK, -LOGICBOMB, -DEBUG, -KERNEL, -SOCKET,
-- -TERMINAL, -ASSEMBLER, -BIT, -BYTE, -POINTER}
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitActionOnPosition
    (entrée u <Entier>, entrée action <Entier>, entrée x <Réel>,
    entrée y <Réel>)
retourne <Entier> ;

-- Donne l'ordre à l'unité u de réaliser l'action action avec le paramètre param
-- Retourne 0 en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite action est une action basique {WAIT, FIRESTATE, SELFDESTRUCTION,
-- REPEAT, MOVESTATE} ou une action spéciale {LAUNCHMINES}
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitUntargetedAction
    (entrée u <Entier>, entrée action <Entier>, entrée param <Réel>)
retourne <Entier> ;

------------------------------------------------------------------------------
-- Opération d'accès aux commandes en attente                               --
-- Attention : ces opérations doivent impérativement être encadrées par les --
-- opérations de gestion de section critique                                --
------------------------------------------------------------------------------

-- Retourne le nombre de commande en attente pour l'unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitGetNumPdgCmds (entrée u <Entier>) retourne <Entier> ;

-- Retourne le code de la ième commande en attente pour l'unité u en cas de
-- succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite i doit être compris dans l'intervalle [0..n[ où n est le nombre de
-- commande en attente pour l'unité u
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitPdgCmdGetCode (entrée u <Entier>, entrée i <Entier>)
retourne <Entier> ;

-- Retourne le nombre de paramètres associés à la ième commande en attente pour
-- l'unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite i doit être compris dans l'intervalle [0..n[ où n est le nombre de
-- commande en attente pour l'unité u
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitPdgCmdGetNumParam (entrée u <Entier>, entrée i <Entier>)
retourne <Entier> ;

-- Retourne le paramètre numéro idParam de la commande en attente numéro idCmd
-- de l'unité u en cas de succès
-- Retourne -1 en cas d'erreur
-- nécessite u soit dirigée par le joueur
-- nécessite idCmd doit être compris dans l'intervalle [0..n[ où n est le
-- nombre de commandes en attente pour l'unité u
-- nécessite idParam doit être compris dans l'intervalle [0..m[ où m est le
-- nombre de paramètre de la commande en attente numéro idCmd pour l'unité u
-- nécessite la PP initialisée
-- nécessite le jeu rafraîchit
fonction unitPdgCmdGetParam (entrée u <Entier>, entrée idCmd <Entier>,
                             entrée idParam <Entier>) retourne <Réel> ;

-----------------------------------------------
-- Opérations de gestion de section critique --
-----------------------------------------------

-- Cette procédure doit être appelée avant l'entrée d'une section critique
-- Ne pas oublier d'appeler "exitCriticalSection" en sortant de la section critique
procédure enterCriticalSection;

-- Cette procédure doit être appelée à la fin de la section critique
procédure exitCriticalSection;

--------------------------------------
-- Opération de gestion des erreurs --
--------------------------------------

-- Retourne la dernière erreur sous la forme d'une chaîne
fonction getError retourne <Chaîne>;

-- Supprime toutes les informations liées à la dernière erreur survenue
procédure clearError;

-------------------------------
-- Opérations additionnelles --
-------------------------------

-- Retourne un nombre aléatoire compris entre 0.0 et limite
fonction random (entrée limite <Réel>) retourne <Réel> ;

-- Effectue une pause de t milisecondes
procédure delay (entrée t <Entier>) ;

-- convertit un Réel en Entier
fonction réelVersEntier (entrée r <Réel>) retourne <Entier> ;
