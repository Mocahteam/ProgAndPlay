#include "PP_Client.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Le passage de structure de Ada à C se fait par référence, pour cette raison.
 * Pour éviter les cas d'erreur le type position et passé sous la forme de 2
 * float et est recounstruite pour l'appel final à Prog&Play */
void Ada_ActionOnPosition (PP_Unit unit, int action, float x, float y){
	PP_Pos p;
	p.x = x;
	p.y = y;
	PP_Unit_ActionOnPosition (unit, action, p);
}

#ifdef __cplusplus
}
#endif
