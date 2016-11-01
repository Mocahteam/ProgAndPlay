from constantList_KP4_1 import *
from pp import *

# Ouverture de la connexion avec le jeu
PP_Open()
# Définition de la position cible
p = PP_Pos (1983.0, 1279.0)
# Récupération de la première unité
u = PP_GetUnitAt(MY_COALITION, 0)
# Ordonner à mon unité courante de se déplacer à la position définie
PP_Unit_ActionOnPosition(u, MOVE, p)
# Fermer la connexion avec le jeu
PP_Close()
