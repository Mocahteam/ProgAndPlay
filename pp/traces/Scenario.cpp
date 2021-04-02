#include "Scenario.h"
#include "Sequence.h"

// Init static fields
float Scenario::SCORE_TOLERENCE = 0.1;
float Scenario::WEIGHT_ALIGN_RATIO = 0.6;
float Scenario::OPTION_PENALTY = 1.94;
float Scenario::WEIGHT_MAXIMIZE_ALIGN = 0.2;
float Scenario::WEIGHT_MINIMIZE_LENGTH = 0.2;
float Scenario::MIN_WINDOW = 2;

Scenario::Scenario (std::vector<Trace::sp_trace> traces, int upCount, int rootStartingPos, int initAlignCount, int initoptCount, int initScore){
    pattern = traces;
	position = 0;
	alignCount = initAlignCount;
	optCount = initoptCount;
	score = initScore;
    this->upCount = upCount;
	this->rootStartingPos = rootStartingPos;
}

Scenario::sp_scenario Scenario::clone (){
    sp_scenario sp_sc_clone = boost::make_shared<Scenario>(Sequence::cloneLinearSequence(pattern), upCount, rootStartingPos);
	sp_sc_clone->position = position;
	sp_sc_clone->alignCount = alignCount;
	sp_sc_clone->optCount = optCount;
	sp_sc_clone->score = -1;
	return sp_sc_clone;
}
    
float Scenario::simulateScore(int alignIncr, int optIncr, int minLength, int maxAligned){
    int nbAlign = alignCount+alignIncr;
    int nbOpt = optCount+optIncr;
	if (nbAlign + nbOpt == 0) return 0;
    //return ((float)(nbAlign*nbAlign) / (nbAlign+nbOpt));
    //return ((float)(nbAlign) / (rootMainPos+nbAlign+nbOpt+pattern.size()-Sequence::getEndPosOfLinearSequence(pattern, 0)));
    //return ((float)(nbAlign) / (nbAlign+nbOpt));
    //return 0.7*(float)nbAlign / (nbAlign+nbOpt) + 0.3*(float)nbAlign / maxAligned;
    //return 0.6*(float)nbAlign / (nbAlign+nbOpt) + 0.2*(float)nbAlign / maxAligned + 0.2*(float)minLength / (pattern.size());
    //return 0.6*(float)nbAlign / (nbAlign+nbOpt) + 0.2*(float)nbAlign / maxAligned + 0.2*(float)minLength / (upCount+pattern.size());
	//return 0.6*(float)nbAlign / (nbAlign+nbOpt*10000) + 0.2*(float)nbAlign / maxAligned + 0.2*(float)minLength / (upCount+pattern.size());

	return WEIGHT_ALIGN_RATIO*(float)nbAlign / (nbAlign+nbOpt*OPTION_PENALTY) + WEIGHT_MAXIMIZE_ALIGN*(float)nbAlign / maxAligned + WEIGHT_MINIMIZE_LENGTH*(float)minLength / (upCount+pattern.size());
}

void Scenario::updateScore(int minLength, int maxAligned){
    score = simulateScore(0, 0, minLength, maxAligned);
}

std::vector<Scenario::sp_scenario> Scenario::simulateMoveToNextCall(){
	// Note :
	//  - lorsque'on atteint une fin de séquence il faut proposer deux scénarios (=> duplication du pattern en question) à savoir recommencer la séquence ou continuer sur la prochaine trace du pattern
	//  - lorsqu'on atteint un début de séquence optionnelle il faut aussi proposer deux scénarios à savoir accéder à la première trace de la séquence ou sauter la séquence pour se positionner directement sur la prochaine trace suivant la séquence (les cas des Call optionnels sont gérés dans "simulateNewCallIntegration")
	
    std::vector<Scenario::sp_scenario> results;
    // sauvegarde de la position initiale pour pouvoir la repositionner à la fin de la simulation
    int savePos = position;

	// Incrémenter de 1 l'indice de parcours du pattern courrant et sauter les Event
	do{
		position++;
	}while (position < (signed)pattern.size() && pattern[position]->isEvent());

	// Si on a dépassé la fin du pattern courrant
	if ((signed)pattern.size() <= position)
	{
		// rien à faire, on est à la fin du pattern, on reste à la fin du pattern
		// On recalle quand mmême la position à la taille du pattern
		position = pattern.size();
		// Et on enregistre une copie de ce scénario
		results.push_back(clone());
	}
	else
	{
		// Si on est sur un Call, c'est nickel
		if (pattern[position]->isCall()){
			// Rien à faire à part enregistrer une copie de ce scénario
			results.push_back(clone());
		}
		else
		{
			// Si on est sur une fin de séquence il faut dupliquer le pattern pour dans un cas se repositionner au début de la séquence et dans l'autre cas passer à la trace suivante
			if (pattern[position]->isSequence() && pattern[position]->getInfo().compare("End") == 0)
			{
				// Comme ici on est en fin de séquence il faut retrouver la position du début de la séquence
				int startSeq = Sequence::getBeginPosOfLinearSequence(pattern, position);
				if (startSeq > -1)
				{
					// On note qu'on a fait une itération supplémentaire. 
					boost::dynamic_pointer_cast<Sequence>(pattern[startSeq])->addOne();

					// Cas 1 : Cloner le pattern. Comme sur ce clone on est positionné sur la fin de la séquence, il faut le faire avancer jusqu'à arrivé à la fin du scénario ou être tombé sur un Call. D'où l'appel récursif
					std::vector<Scenario::sp_scenario> res = clone()->simulateMoveToNextCall();
					results.insert(results.end(), res.begin(), res.end());

					// Cas 2 : repositionner le pattern courant au début de la séquence (le cas où la première trace est une séquence est géré dans la cas juste après)
					position = startSeq;
					boost::dynamic_pointer_cast<Sequence>(pattern[startSeq])->newIter = true; // noter que cette séquence est en reprise d'itération
				}
			}
			// Si on est sur un début de séquence, il faut gérer le cas des séquences optionnelles
			if (pattern[position]->isSequence() && pattern[position]->getInfo().compare("Begin") == 0)
			{
				// Si on tombe sur une séquence optionnelle il faut dupliquer le pattern pour gérer le cas où on la saute
				if (pattern[position]->isOptional())
				{
					// Récupération de l'indice de fin de la séquence
					int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, position);
					if (endSeqPos > -1)
					{
						// Cloner le pattern
						Scenario::sp_scenario clonedSc = clone();
						// On positionne ce clone sur la fin de la séquence
						clonedSc->position = endSeqPos;
						// Et on fait un appel récursif pour atteindre le prochain Call
						std::vector<Scenario::sp_scenario> res = clonedSc->simulateMoveToNextCall();
						results.insert(results.end(), res.begin(), res.end());
					}
				}
				// Pour le scénario courrant (non clonné), on est toujours sur un début de séquence, il faut donc progresser dans cette séquence jusqu'à tomber sur un Call
				std::vector<Scenario::sp_scenario> res = simulateMoveToNextCall();
				results.insert(results.end(), res.begin(), res.end());
			}
		}
	}
    // rétablissement de la position initiale avant la simulation
    position = savePos;
    return results;
}

std::vector<Scenario::sp_scenario> Scenario::simulateNewCallIntegration(const Trace::sp_trace & rootCall, float maxRatio, int minLength, int * maxAligned, int nbRec, std::vector<Scenario::sp_scenario> * viewed){
	/*std::cout << "Scenario::simulateNewCallIntegration " << position << "/" << pattern.size() << " " << nbRec;
	if (position > 0 && position < (signed)pattern.size())
		pattern[position]->exportAsString(std::cout);
	else
		std::cout << std::endl;
	std::cout << "Root : ";
	rootCall->exportAsString(std::cout); 
	Sequence::exportLinearSequenceAsString(pattern, std::cout);*/

	std::vector<Scenario::sp_scenario> result;

	if (nbRec == 0 && viewed == NULL)
		viewed = new std::vector<Scenario::sp_scenario>();
	
    // Vérifier si on a dépassé la fin du pattern
	if (position >= (signed)pattern.size())
	{
		// Si on a dépassé la fin du pattern courrant deux cas se présentent :
		//  1 - si le Call courrant précède immédiatement une fin de séquence, l'ajouter comme option de cette fin de séquence (remonter tant qu'on a des fins de séquences)
		//  2 - ajouter ce Call à la fin du pattern

		// Cas 1 : si le Call courrant précède immédiatement une fin de séquence et que le score resterait intéressant malgrès l'ajout d'une nouvelle option
		int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, 0);
		if (endSeqPos == (signed)pattern.size()-1 && simulateScore(0, 1, minLength, *maxAligned) > maxRatio - SCORE_TOLERENCE)
		{
			// Ajouter comme option le Call en fin de séquence (on remonte tant qu'on tombe sur une fin de séquence ou sur une trace optionnelle)
            while (endSeqPos > 0 && (pattern[endSeqPos]->getInfo().compare("End") == 0 || pattern[endSeqPos]->isOptional())){
				// On ajoute le Call comme option que si on est sur une fin de séquence
				if (pattern[endSeqPos]->getInfo().compare("End") == 0){
					// on travaille maintenant sur un clone
					Scenario::sp_scenario tmpSc = clone();
					tmpSc->pattern.insert(tmpSc->pattern.begin()+endSeqPos, rootCall->clone());
					if (!tmpSc->pattern[endSeqPos]->isOptional())
					{
						tmpSc->pattern[endSeqPos]->setOptional(true);
						tmpSc->optCount++;
					}
					// Bien se repositionner à la fin du pattern
					tmpSc->position = endSeqPos;
					// On est revenu dans la séquence donc il faut réduire le nombre d'itération de 1 car on va la remettre quand on retombera à nouveau sur la fin de la séquence
					int startSeqPos = Sequence::getBeginPosOfLinearSequence(tmpSc->pattern, endSeqPos);
					while (startSeqPos != -1){
						if (tmpSc->pattern[startSeqPos]->isSequence()){
							boost::dynamic_pointer_cast<Sequence>(tmpSc->pattern[startSeqPos])->removeOne();
						}
						startSeqPos = Sequence::getBeginPosOfLinearSequence(tmpSc->pattern, startSeqPos-1);
					}
					// Insérer le clone dans le vecteur de résultats
					result.push_back(tmpSc);
				}
				// Tenter de remonter à la fin de séquence précédente
				endSeqPos--;
			}
		}

		// Cas 2 : ajouter le Call courant (root) à la fin du pattern
		// Insérer un clone dans le vecteur de résultats
		result.push_back(clone());
		// Ici on stoque la trace à l'extérieur du pattern
		result.back()->pattern.push_back(rootCall->clone());
		result.back()->position++;
	}
	else
	{
		// Ici on est à l'intérieur du pattern, il faut maintenant voir si le Call du pattern est alignable avec celui du root
		// Si le Call courrant du pattern == au Call du root
		if (pattern[position]->operator==(rootCall.get()))
		{
			// on travaille maintenant sur un clone
			Scenario::sp_scenario tmpSc = clone();
			// Cas idéal => augmenter de 1 le nombre d'alignement de ce pattern dans le vecteur de compteur d'alignement
			tmpSc->alignCount++;
			// Si la trace courrante du pattern est une option on augmente quand même le nombre d'option
			if (tmpSc->pattern[tmpSc->position]->isOptional())
				tmpSc->optCount++;
			// Fusionner les deux appels
			dynamic_cast<Call *>(tmpSc->pattern[tmpSc->position].get())->filterCall(dynamic_cast<const Call *>(rootCall.get()));
			// Mise à jour du max
			*maxAligned = (tmpSc->alignCount > *maxAligned) ? tmpSc->alignCount : *maxAligned;
			// Réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle, par sécurité on reset toutes les séquences du pattern
            for (int i = 0 ; i < (signed)tmpSc->pattern.size() ; i++)
                if (tmpSc->pattern[i]->isSequence() && tmpSc->pattern[i]->getInfo().compare("Begin") == 0)
                    boost::dynamic_pointer_cast<Sequence>(tmpSc->pattern[i])->newIter = false;
			// Insérer le clone dans le vecteur de résultats
			result.push_back(tmpSc);
		}
		else
		{
			// Ici les deux call ne sont pas alignable, l'un des deux doit être mis en option

			// Vérifier que le score resterait intéressant malgrès l'ajout d'une nouvelle option
			if (pattern[position]->isOptional() || simulateScore(0, 1, minLength, *maxAligned) > maxRatio - SCORE_TOLERENCE){

				// CAS 1 : Possibilité de mettre le Call du root en option
				
				// On commence par vérifier si la trace précédente est un début de séquence
				if (pattern[position-1]->isSequence() && pattern[position-1]->getInfo().compare("Begin") == 0){
					// Ici on est sur un début de séquence et les deux calls sont différents. On doit donc générer plusieurs scénarios qui dépendent du contexte.
					//  (a) Si on avait atteint une fin de séquence dans le pattern et qu'on est revenu au début de la séquence (cf ETAPE 1 lors de la progression dans chaque pattern). Dans ce cas il faut générer deux scénarios : un pour insérer l'option en début de séquence et un autre pour l'insérer en fin de séquence.
					//  (b) Si on était en train de progresser dans le pattern et qu'on était arrivé sur un début de séquence, alors on a plongé dans cette séquence (et éventuelement ses sous-séquences) jusqu'à tomber sur un Call. Danc ce cas là, il faut générer plusieurs scénarios : un pour insérer l'option en début de séquence avant le Call mais aussi autant que necessaire pour insérer l'option avant chaque début de séquence imbriquées. Attention, l'un de ces scénarios pourrait remonter sur une séquence correspondant au cas (a), dans ce cas, c'est un cas d'arrêt. 

					// => Dans ces deux possibilités un scénario consiste à insérer l'option en début de séquence (juste avant le call du pattern) ce qui correspond aussi au cas où on n'aurait pas été en début de séquence. Ce cas est traité juste à la sortie de ce "if"

					int savePos = position; // on sauvegarde notre position dans le pattern pour pouvoir le rétablir après le traitement de la boucle qui peut faire évoluer la position notemment pour gérer les cas (b)
					bool newLoop = false;
					do{
						// on travaille sur un clone
						Scenario::sp_scenario tmpSc = clone();

						// On teste pour vérifier dans quel contexte on est
						if (boost::dynamic_pointer_cast<Sequence>(tmpSc->pattern[tmpSc->position-1])->newIter){
							// cas (a)
							
							// Se positionner à la fin de la séquence du clone
							tmpSc->position = Sequence::getEndPosOfLinearSequence(tmpSc->pattern, tmpSc->position);
							// Insérer dans le clone un clone du Call du root (donc à la fin de la séquence)
							tmpSc->pattern.insert (tmpSc->pattern.begin()+tmpSc->position, rootCall->clone());
							// Indiquer ce clone du Call du root comme optionnel
							tmpSc->pattern[tmpSc->position]->setOptional(true);
							// Augmenter de 1 le nombre d'option de ce pattern
							tmpSc->optCount++;
							// stopper la boucle
							newLoop = false;
						} else {
							// cas (b)

							// Insérer dans le clone un clone du Call du root AVANT le début de la séquence (donc à l'extérieur de la séquence)
							tmpSc->pattern.insert (tmpSc->pattern.begin()+tmpSc->position-1, rootCall->clone());
							// Indiquer ce clone du Call du root comme optionnel
							tmpSc->pattern[tmpSc->position-1]->setOptional(true);
							// Augmenter de 1 le nombre d'option de ce pattern
							tmpSc->optCount++;

							// Dans le modèle ou souhaite vérifier si la trace en amont est un début de séquence (sauf si la première car on ne souhaite pas insérer de Call en amont du scénario). Si oui, autoriser une nouvelle itération pour insérer le clone du Call du root en amont de ce début de séquence.
							// Donc il faut se positionner sur la trace en amont dans le modèle
							// Rq : on est en train de remonter le pattern et tant qu'on tombe sur des débuts de séquence on refait le processus (cloner et insérer la trace du root en option)
							position--;

							// Vérifer s'il faut refaire un tour de boucle
							newLoop = position-1 > 0 && pattern[position-1]->isSequence() && pattern[position-1]->getInfo().compare("Begin") == 0;
						}

						// Insérer le clone dans le vecteur de résultat
						result.push_back(tmpSc);
					} while (newLoop);
					
					// En sortant de cette boucle cela signifie que la trace actuelle du modèle n'est pas un début de séquence. Il faut donc revenir sur la position courrante
					position = savePos;
				}
				
				// Dans tous les cas on ajoute le call du root en option juste avant le call du pattern
				// on céée un nouveau clone pour pouvoir gérer le Cas 2 (call du pattern en option)
				Scenario::sp_scenario tmpSc = clone();
				// Ajout du call du root en option juste avant le call du pattern
				tmpSc->pattern.insert (tmpSc->pattern.begin()+tmpSc->position, rootCall->clone());
				// Indiquer ce clone du Call du root comme optionnel
				tmpSc->pattern[tmpSc->position]->setOptional(true);
				// Augmenter de 1 le nombre d'option de ce pattern
				tmpSc->optCount++;
				// Insérer le clone dans le vecteur de résultat
				result.push_back(tmpSc);
				
				// CAS 2 : mettre le Call du pattern en option
				// On crée un clone pour travailler
				tmpSc = clone();
				// Si la trace courrante du pattern n'est pas une option
				if (!tmpSc->pattern[tmpSc->position]->isOptional())
				{
					// Indiquer la trace courante du pattern comme optionnel
					tmpSc->pattern[tmpSc->position]->setOptional(true);
					// Augmenter de 1 le nombre d'option de ce pattern
					tmpSc->optCount++;
				}
				// Avancer dans ce pattern pour prendre en compte le fait qu'on saute l'option
				std::vector<Scenario::sp_scenario> movedScenar = tmpSc->simulateMoveToNextCall();
				for (int j = 0 ; j < (signed)movedScenar.size() ; j++){
					// Vérifier si on n'est pas retombé sur un scénario déjà expertisé
					if (!movedScenar[j]->existsIn(*viewed, true) && !movedScenar[j]->existsIn(result, true)){
						// On fait donc un appel récursif pour tenter de positionner la trace courrante
						viewed->push_back(movedScenar[j]);
						std::vector<Scenario::sp_scenario> sim = movedScenar[j]->simulateNewCallIntegration(rootCall, maxRatio, minLength, maxAligned, nbRec+1, viewed);
						// Intégration aux résultats des scénarios simulés
						result.insert(result.end(), sim.begin(), sim.end());
                    }
				}
			}
		}
	}
	if (nbRec == 0)
		delete viewed;
	return result;
}

bool Scenario::existsIn(std::vector<Scenario::sp_scenario> set, bool checkPosition){
	bool found = false;
	std::stringstream thisSS;
	Sequence::exportLinearSequenceAsString(pattern, thisSS, false);
	for (int i = 0 ; i < (signed)set.size() && !found; i++){
		found = this->isEqualWith(set[i], checkPosition);
	}
	return found;
}

bool Scenario::isEqualWith(Scenario::sp_scenario scenario, bool checkPosition){
	if (scenario->pattern.size() == pattern.size()){
		if (checkPosition && scenario->position != position)
			return false;
		std::stringstream setSS;
		Sequence::exportLinearSequenceAsString(scenario->pattern, setSS, false);
		std::stringstream thisSS;
		Sequence::exportLinearSequenceAsString(pattern, thisSS, false);
		return setSS.str().compare(thisSS.str()) == 0;
	}
	return false;
}