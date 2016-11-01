-- entréeSortie.spec
-- Interface avec les méthodes natives de la bibliothèque standard de Compalgo v2.
--
-- Ajoute la lecture et l'écriture de données à la bibliothèque standard de Compalgo.
--

-- écrit un booléen sur la sortie standard.
procédure écrire( entrée e <Booléen> );
-- écrit un chaîne sur la sortie standard.
procédure écrire( entrée e <Chaîne> );
-- écrit un entier sur la sortie standard.
procédure écrire( entrée e <Entier> );
-- écrit un réel sur la sortie standard.
procédure écrire( entrée e <Réel> );
-- écrit un caractère sur la sortie standard.
procédure écrire( entrée e <Caractère> );

-- retourne à la ligne.
procédure écrirenl;
-- écrit un booléen sur la sortie standard et retourne à la ligne.
procédure écrirenl( entrée e <Booléen> );
-- écrit une chaîne sur la sortie standard et retourne à la ligne.
procédure écrirenl( entrée e <Chaîne> );
-- écrit un entier sur la sortie standard et retourne à la ligne.
procédure écrirenl( entrée e <Entier> );
-- écrit un réel sur la sortie standard et retourne à la ligne.
procédure écrirenl( entrée e <Réel> );
-- écrit un caractère sur la sortie standard et retourne à la ligne.
procédure écrirenl( entrée e <Caractère> );

-- lit un booléen depuis l'entrée standard.
procédure lire( sortie s <Booléen> );
-- lit un entier depuis l'entrée standard.
procédure lire( sortie s <Entier> );
-- lit un réel depuis l'entrée standard.
procédure lire( sortie s <Réel> );
-- lit une chaîne depuis l'entrée standard.
procédure lire( sortie s <Chaîne> );
-- lit un caractère depuis l'entrée standard.
procédure lire( sortie s <Caractère> );


-- Effecue une pause dans la console. Attend un retour chariot pour continuer.
procédure pause;
