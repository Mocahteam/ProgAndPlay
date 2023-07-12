#include "Scenario.h"
#include "Sequence.h"

// Init static fields
float Scenario::SCORE_TOLERENCE = 0.1;
float Scenario::WEIGHT_ALIGN_RATIO = 0.6;
float Scenario::WEIGHT_MINIMIZE_LENGTH = 0.4;

Scenario::Scenario (std::vector<Trace::sp_trace> traces, int upCount, int rootStartingPos, int initAlignCount, int initoptCount, int initScore){
    pattern = traces;
    pattern_tke = Sequence::cloneLinearSequence(traces);
	position = 0;
	num_descend = 0;
	alignCount = initAlignCount;
	optCount = initoptCount;
	score = initScore;
    this->upCount = upCount;
	this->rootStartingPos = rootStartingPos;
}

Scenario::sp_scenario Scenario::clone (){
    sp_scenario sp_sc_clone = std::make_shared<Scenario>(Sequence::cloneLinearSequence(pattern), upCount, rootStartingPos);
	sp_sc_clone->pattern_tke = Sequence::cloneLinearSequence(pattern_tke);
	sp_sc_clone->position = position;
	sp_sc_clone->alignCount = alignCount;
	sp_sc_clone->optCount = optCount;
	sp_sc_clone->score = score;
	sp_sc_clone->num_descend = num_descend;
	return sp_sc_clone;
}
    
float Scenario::simulateScore(int alignIncr, int optIncr, int minLength){
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
	// float part1 = WEIGHT_ALIGN_RATIO*(float)nbAlign / (nbAlign+nbOpt*OPTION_PENALTY);
	// float part2 = WEIGHT_MAXIMIZE_ALIGN*(float)nbAlign / maxAligned;
	// float part3 = WEIGHT_MINIMIZE_LENGTH*(float)minLength / (upCount+pattern.size());
	// float total = part1 + part2 + part3;
	// return WEIGHT_ALIGN_RATIO*(float)nbAlign / (nbAlign+nbOpt*OPTION_PENALTY) + WEIGHT_MAXIMIZE_ALIGN*(float)nbAlign / maxAligned + WEIGHT_MINIMIZE_LENGTH*(float)minLength / (upCount+pattern.size());
	float part1 = WEIGHT_ALIGN_RATIO * ((float)nbAlign / float(nbAlign + nbOpt));
	float part2 = WEIGHT_MINIMIZE_LENGTH * ((float)minLength / (float)getLength());
	//std::cout<<"part1="<<part1<< " x1=>"<< nbAlign <<" x2=>"<< nbOpt <<" part2="<<part2<<std::endl;
	return (part1+part2);
}

void Scenario::updateScore(int minLength){
	float new_score = simulateScore(0, 0, minLength);
	if(new_score<score){
		num_descend++;
	}
	else{
		num_descend=0;
	}
    score = new_score;
}

std::vector<Scenario::sp_scenario> Scenario::simulateMoveToNextCall(){
	// Note :
	//  - lorsqu'on atteint une fin de séquence il faut proposer deux scénarios (=> duplication du pattern en question) à savoir recommencer la séquence ou continuer sur la prochaine trace du pattern
	//  - lorsqu'on atteint un début de séquence il faut aussi proposer deux scénarios à savoir accéder à la première trace de la séquence ou sauter la séquence pour se positionner directement sur la prochaine trace suivant la séquence (mise de la séquence en optionnelle, les cas des Call optionnels sont gérés dans "simulateNewCallIntegration")
	
    std::vector<Scenario::sp_scenario> results;
    // sauvegarde de la position initiale pour pouvoir la repositionner à la fin de la simulation
    int savePos = position;

	// Incrémenter de 1 l'indice de parcours du pattern courrant
	position++;

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
					std::dynamic_pointer_cast<Sequence>(pattern[startSeq])->addOne();

					// Cas 1 : Cloner le pattern. Sur ce clone on est toujours positionné sur la fin de la séquence, il faut le faire avancer jusqu'à arrivé à la fin du scénario ou être tombé sur un Call. D'où l'appel récursif
					std::vector<Scenario::sp_scenario> res = clone()->simulateMoveToNextCall();
					results.insert(results.end(), res.begin(), res.end());

					// Cas 2 : repositionner le pattern courant au début de la séquence (le cas où la première trace est une séquence est géré dans la cas juste après)
					position = startSeq;
					std::dynamic_pointer_cast<Sequence>(pattern[startSeq])->newIter = true; // noter que cette séquence est en reprise d'itération
				}
			}
			// Si on est sur un début de séquence, il faut générer les cas où on saute la séquence ou on rentre dans la séquence
			if (pattern[position]->isSequence() && pattern[position]->getInfo().compare("Begin") == 0)
			{
				// Récupération de l'indice de fin de la séquence
				int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, position);
				// On génère un scénario où on saute la séquence en la mettant optionnelle si on n'est pas sur une reprise d'itération
				if (endSeqPos > -1 && !std::dynamic_pointer_cast<Sequence>(pattern[position])->newIter)
				{
					// Cloner le pattern
					Scenario::sp_scenario clonedSc = clone();
					// Mettre la séquence en optionnelle
					clonedSc->pattern[position]->setOptional(true);
					// On positionne ce clone sur la fin de la séquence
					clonedSc->position = endSeqPos;
					// Et on fait un appel récursif pour atteindre le prochain Call
					std::vector<Scenario::sp_scenario> res = clonedSc->simulateMoveToNextCall();
					results.insert(results.end(), res.begin(), res.end());
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

std::vector<Scenario::sp_scenario> Scenario::simulateNewCallIntegration(const Call::sp_call & rootCall, float maxRatio, int minLength, int nbRec, std::vector<Scenario::sp_scenario> * viewed){
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
		// Si on a dépassé la fin du pattern courrant trois cas se présentent :
		//  1 - si le Call courrant précède immédiatement une fin de séquence, l'ajouter comme option de cette fin de séquence (remonter tant qu'on a des fins de séquences)
		//  2 - ajouter ce Call à la fin du pattern
		//  3 - ajouter une nouvelle sequence de tke à la fin du pattern si ce Call est identique au premier Call de tke

		// Cas 1 : si le Call courrant précède immédiatement une fin de séquence et que le score resterait intéressant malgrès l'ajout d'une nouvelle option
		int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, 0);
		if (endSeqPos == (signed)pattern.size()-1 && simulateScore(0,1,minLength) > maxRatio - SCORE_TOLERENCE * maxRatio)
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
							std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[startSeqPos])->removeOne();
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
		result.back()->alignCount++;
		result.back()->position++;

		// Cas 3 : ajouter une nouvelle sequence de tke à la fin du pattern si ce Call est identique au premier Call de tke et il ne suis pas une sequence 'End'
		int tkeFirstCallPos = Sequence::getCallPosInLinearSequence(pattern_tke, 1);
		if(tkeFirstCallPos >= 0 && rootCall->operator==(pattern_tke[tkeFirstCallPos].get()) && pattern.back()->getInfo().compare("End") != 0){
			result.push_back(clone());
			std::vector<Trace::sp_trace> tke_copy = Sequence::cloneLinearSequence(pattern_tke);
			result.back()->pattern.insert(result.back()->pattern.end(), tke_copy.begin(), tke_copy.end());
			result.back()->alignCount++;
			result.back()->position += tkeFirstCallPos; // on se repositionne sur le premier call du tke_copy
		}
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

			// Réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle, par sécurité on reset toutes les séquences du pattern
            for (int i = 0 ; i < (signed)tmpSc->pattern.size() ; i++)
                if (tmpSc->pattern[i]->isSequence() && tmpSc->pattern[i]->getInfo().compare("Begin") == 0)
                    std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[i])->newIter = false;
			// Insérer le clone dans le vecteur de résultats
			result.push_back(tmpSc);
		}
		else
		{
			// Ici les deux call ne sont pas alignable, l'un des deux doit être mis en option

			// Vérifier que le score resterait intéressant malgrès l'ajout d'une nouvelle option
			if (pattern[position]->isOptional() || simulateScore(0, 1, minLength) > maxRatio - SCORE_TOLERENCE * maxRatio ){

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
						if (std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[tmpSc->position-1])->newIter){
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
				else{

					// Dans tous les cas où on n'est pas dans la première position de sequence,  on ajoute le call du root en option juste avant le call du pattern (pour éviter d'y mettre une option au premier)
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
							std::vector<Scenario::sp_scenario> sim = movedScenar[j]->simulateNewCallIntegration(rootCall, maxRatio, minLength, nbRec+1, viewed);
							// Intégration aux résultats des scénarios simulés
							result.insert(result.end(), sim.begin(), sim.end());
	                    }
					}
				}
			}
		}
	}
	if (nbRec == 0)
		delete viewed;
	return result;
}

std::vector<Scenario::sp_scenario> Scenario::simulateNewTraceIntegration(const Trace::sp_trace & rootTrace, float maxRatio, int minLength, int nbRec, std::vector<Scenario::sp_scenario> * viewed){




	/*std::cout << "Scenario::simulateNewCallIntegration " << position << "/" << pattern.size() << " " << nbRec;
	if (position > 0 && position < (signed)pattern.size())
		pattern[position]->exportAsString(std::cout);
	else
		std::cout << std::endl;
	std::cout << "Root : ";
	rootCall->exportAsString(std::cout); 
	Sequence::exportLinearSequenceAsString(pattern, std::cout);*/

	std::vector<Scenario::sp_scenario> result;
/*
	if (nbRec == 0 && viewed == NULL)
		viewed = new std::vector<Scenario::sp_scenario>();
	
	std::vector<Trace::sp_trace> linearRoot = rootTrace->clone()->getLinearSequence();
    
	// Vérifier si on a dépassé la fin du pattern
	if (position >= (signed)pattern.size())
	{
		// Si on a dépassé la fin du pattern courrant trois cas se présentent :
		//  1 - si la Trace courrante précède immédiatement une fin de séquence, l'ajouter comme option de cette fin de séquence (remonter tant qu'on a des fins de séquences)
		//  2 - ajouter cette Trace à la fin du pattern
		//  3 - ajouter une nouvelle instance de tke à la fin du pattern si cette Trace est similaire au modèle tke

		// Cas 1 : si la Trace courrante précède immédiatement une fin de séquence et que le score resterait intéressant malgrès l'ajout d'une nouvelle option
		int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, 0);
		if (endSeqPos == (signed)pattern.size()-1 && simulateScore(0,1,minLength) > maxRatio - SCORE_TOLERENCE * maxRatio)
		{
			// Ajouter comme option la Trace en fin de séquence (on remonte tant qu'on tombe sur une fin de séquence ou sur une trace optionnelle)
            while (endSeqPos > 0 && (pattern[endSeqPos]->getInfo().compare("End") == 0 || pattern[endSeqPos]->isOptional())){
				// On ajoute la Trace comme option que si on est sur une fin de séquence
				if (pattern[endSeqPos]->getInfo().compare("End") == 0){
					// on travaille maintenant sur un clone
					Scenario::sp_scenario tmpSc = clone();
					tmpSc->pattern.insert(tmpSc->pattern.begin()+endSeqPos, linearRoot.begin(), linearRoot.end());
					if (!tmpSc->pattern[endSeqPos]->isOptional())
					{
						tmpSc->pattern[endSeqPos]->setOptional(true);
						tmpSc->optCount++;
					}
					// Bien se repositionner à la fin du pattern
					tmpSc->position = endSeqPos+linearRoot.size()-1;
					// On est revenu dans la séquence donc il faut réduire le nombre d'itération de 1 car on va la remettre quand on retombera à nouveau sur la fin de la séquence
					int startSeqPos = Sequence::getBeginPosOfLinearSequence(tmpSc->pattern, endSeqPos);
					while (startSeqPos != -1){
						if (tmpSc->pattern[startSeqPos]->isSequence()){
							std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[startSeqPos])->removeOne();
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

		// Cas 2 : ajouter la Trace courante (root) à la fin du pattern
		// Insérer un clone dans le vecteur de résultats
		result.push_back(clone());
		// Ici on stoque la trace à l'extérieur du pattern
		result.back()->pattern.push_back(rootTrace->clone());
		result.back()->alignCount++;
		result.back()->position += linearRoot.size();

		// Cas 3 : ajouter une nouvelle sequence de tke à la fin du pattern si cette Trace est compatible avec le modèle tke
		// d'abord on s'assure que la trace ne suis pas une sequence 'End' (géré dans le cas 1)
		if (pattern.back()->getInfo().compare("End") != 0){
			bool addNewTkePattern = false;
			int newPos;
			if (rootTrace->isCall())
			{
				// vérifier si le Call est identique au premier Call du pattern tke
				int tkeFirstCallPos = Sequence::getCallPosInLinearSequence(pattern_tke, 1);
				if(tkeFirstCallPos >= 0 && rootTrace->operator==(pattern_tke[tkeFirstCallPos].get())){
					addNewTkePattern = true;
					newPos = tkeFirstCallPos; // on se repositionnera sur le premier call du tke_copy
				}
			}
			else
			{
				// vérifier si la distance entre la séquence du root et le pattern tke est égal à 0 (ce calcul de distance ignore les traces optionnelles si elles ne matchent pas)
				if (Sequence::computeLinearSequenceDistance(linearRoot, pattern_tke) == 0){
					addNewTkePattern = true;
					newPos = -1; // on se repositionnera sur la trace précédente de manière à ce que sur le prochain simulateMoveToNextCall on retombe sur le Seq("Begin") et génèrer les scénarios pour rentrer dans le pattern ou le sauter
				}
			}
			if (addNewTkePattern){
				result.push_back(clone());
				std::vector<Trace::sp_trace> tke_copy = Sequence::cloneLinearSequence(pattern_tke);
				result.back()->pattern.insert(result.back()->pattern.end(), tke_copy.begin(), tke_copy.end());
				result.back()->alignCount++;
				result.back()->position += newPos;
			}
		}
	}
	else
	{
		// Ici on est à l'intérieur du pattern, il faut maintenant voir si la Trace du pattern est alignable avec celle du root
		// Si la Trace courrante du pattern == à la Trace du root
		if (pattern[position]->operator==(rootTrace.get()))
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

			// Réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle, par sécurité on reset toutes les séquences du pattern
            for (int i = 0 ; i < (signed)tmpSc->pattern.size() ; i++)
                if (tmpSc->pattern[i]->isSequence() && tmpSc->pattern[i]->getInfo().compare("Begin") == 0)
                    std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[i])->newIter = false;
			// Insérer le clone dans le vecteur de résultats
			result.push_back(tmpSc);
		}
		else
		{
			// Ici les deux call ne sont pas alignable, l'un des deux doit être mis en option

			// Vérifier que le score resterait intéressant malgrès l'ajout d'une nouvelle option
			if (pattern[position]->isOptional() || simulateScore(0, 1, minLength) > maxRatio - SCORE_TOLERENCE * maxRatio ){

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
						if (std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[tmpSc->position-1])->newIter){
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
				else{

					// Dans tous les cas où on n'est pas dans la première position de sequence,  on ajoute le call du root en option juste avant le call du pattern (pour éviter d'y mettre une option au premier)
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
							std::vector<Scenario::sp_scenario> sim = movedScenar[j]->simulateNewCallIntegration(rootCall, maxRatio, minLength, nbRec+1, viewed);
							// Intégration aux résultats des scénarios simulés
							result.insert(result.end(), sim.begin(), sim.end());
	                    }
					}
				}
			}
		}
	}
	if (nbRec == 0)
		delete viewed;*/
	return result;
}

//std::vector<Scenario::sp_scenario> Scenario::simulateNewSeqIntegration(const Sequence::sp_sequence & rootSeq, float maxRatio, int minLength, int nbRec = 0, std::vector<Scenario::sp_scenario> * viewed = NULL){
	// Ici la position dans this->pattern est positionnée sur un Call. Ce call peut être le successeur d'un autre Call, le premier d'une séquence (prédécesseur "Seq(Begin)"), le premier à la suite d'une séquence (prédécesseur "Seq(End)"). Il faut donc chercher à aligner une séquence récupérée depuis le Root à partir de ce point là. Différents scénario sont possibles :
	//  Cas 1 et 3 (successeur d'un autre Call ou successeur d'un "Seq(End)") - On va intercaller la séquence on la mettant optionnelle juste avant le Call pointé
	//  Cas 2 (premier d'une séquence) - On va chercher à vérifier si on a un match entre les séquences :
	//    1 : calculer l'indice du premier call du Root (Ex: Sb Sb A Se B Se => 2)
	//    2 : remonter dans le pattern d'autant de position (en sautant les options) pour tenter de se repositionner sur un début de séquence et vérifier la compatibilité des deux Séquences.
	//      2.1 : S'il y a un match => on repositionne la pattern sur le Seq(End) de la séquence alignée et on fusionne les deux séquences (gestion des options).
	//      2.2 : S'il n'y a pas de match il faut vérifier si on est en reprise d'itération. Pour déterminer si c'est une reprise d'itération il faut remonter les séquences imbriquées pour vérifier si dans la chaine il n'y en aurait pas une tagué en reprise d'itération.
	//        2.2.1 : S'il y a reprise d'itération, insérer la séquence optionnelle en début et en fin de séquence en reprise d'itération.
	//        2.2.2 : S'il n'y a pas de reprise d'itération, mettre la séquende du root en option OU mettre celle du pattern en option et tenter d'aligner avec la trace suivante.
	
/*	std::vector<Scenario::sp_scenario> result;

	if (nbRec == 0 && viewed == NULL)
		viewed = new std::vector<Scenario::sp_scenario>();
	
    // Vérifier si on a dépassé la fin du pattern
	if (position >= (signed)pattern.size())
	{
		// Si on a dépassé la fin du pattern courrant trois cas se présentent :
		//  1 - si la Séquence courrante précède immédiatement une fin de séquence, l'ajouter comme option de cette fin de séquence (remonter tant qu'on a des fins de séquences)
		//  2 - ajouter cette Séquence à la fin du pattern
		//  3 - ajouter une nouvelle sequence de tke à la fin du pattern si cette séquence est fusionable au modèle tke

		// Cas 1 : si la Séquence courrante précède immédiatement une fin de séquence et que le score resterait intéressant malgrès l'ajout d'une nouvelle option
		int endSeqPos = Sequence::getEndPosOfLinearSequence(pattern, 0);
		if (endSeqPos == (signed)pattern.size()-1 && simulateScore(0,1,minLength) > maxRatio - SCORE_TOLERENCE * maxRatio)
		{
			// Ajouter comme option la Séquence en fin de séquence (on remonte tant qu'on tombe sur une fin de séquence ou sur une trace optionnelle)
            while (endSeqPos > 0 && (pattern[endSeqPos]->getInfo().compare("End") == 0 || pattern[endSeqPos]->isOptional())){
				// On ajoute la Séquence comme option que si on est sur une fin de séquence
				if (pattern[endSeqPos]->getInfo().compare("End") == 0){
					// on travaille maintenant sur un clone
					Scenario::sp_scenario tmpSc = clone();
					std::vector<Trace::sp_trace> linearRoot = Sequence::cloneLinearSequence(rootSeq->getLinearSequence());
					tmpSc->pattern.insert(tmpSc->pattern.begin()+endSeqPos, linearRoot.begin(), linearRoot.end());
					if (!tmpSc->pattern[endSeqPos]->isOptional())
					{
						tmpSc->pattern[endSeqPos]->setOptional(true);
						tmpSc->optCount++;
					}
					// Bien se repositionner à la fin du pattern
					tmpSc->position = Sequence::getEndPosOfLinearSequence(tmpSc->pattern, 0);;
					// On est revenu dans la séquence donc il faut réduire le nombre d'itération de 1 car on va la remettre quand on retombera à nouveau sur la fin de la séquence
					int startSeqPos = Sequence::getBeginPosOfLinearSequence(tmpSc->pattern, tmpSc->position);
					while (startSeqPos != -1){
						if (tmpSc->pattern[startSeqPos]->isSequence()){
							std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[startSeqPos])->removeOne();
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

		// Cas 2 : ajouter la Sequence courante (root) à la fin du pattern
		// Insérer un clone dans le vecteur de résultats
		result.push_back(clone());
		// Ici on stoque la trace à l'extérieur du pattern
		result.back()->pattern.push_back(rootSeq->clone());
		result.back()->alignCount++;
		result.back()->position++;

		// Cas 3 : ajouter une nouvelle sequence de tke à la fin du pattern si ce Call est identique au premier Call de tke et il ne suis pas une sequence 'End'
		int tkeFirstCallPos = Sequence::getCallPosInLinearSequence(pattern_tke, 1);
		if(tkeFirstCallPos >= 0 && rootCall->operator==(pattern_tke[tkeFirstCallPos].get()) && pattern.back()->getInfo().compare("End") != 0){
			result.push_back(clone());
			std::vector<Trace::sp_trace> tke_copy = Sequence::cloneLinearSequence(pattern_tke);
			result.back()->pattern.insert(result.back()->pattern.end(), tke_copy.begin(), tke_copy.end());
			result.back()->alignCount++;
			result.back()->position += tkeFirstCallPos; // on se repositionne sur le premier call du tke_copy
		}
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

			// Réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle, par sécurité on reset toutes les séquences du pattern
            for (int i = 0 ; i < (signed)tmpSc->pattern.size() ; i++)
                if (tmpSc->pattern[i]->isSequence() && tmpSc->pattern[i]->getInfo().compare("Begin") == 0)
                    std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[i])->newIter = false;
			// Insérer le clone dans le vecteur de résultats
			result.push_back(tmpSc);
		}
		else
		{
			// Ici les deux call ne sont pas alignable, l'un des deux doit être mis en option

			// Vérifier que le score resterait intéressant malgrès l'ajout d'une nouvelle option
			if (pattern[position]->isOptional() || simulateScore(0, 1, minLength) > maxRatio - SCORE_TOLERENCE * maxRatio ){

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
						if (std::dynamic_pointer_cast<Sequence>(tmpSc->pattern[tmpSc->position-1])->newIter){
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
				else{

					// Dans tous les cas où on n'est pas dans la première position de sequence,  on ajoute le call du root en option juste avant le call du pattern (pour éviter d'y mettre une option au premier)
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
							std::vector<Scenario::sp_scenario> sim = movedScenar[j]->simulateNewCallIntegration(rootCall, maxRatio, minLength, nbRec+1, viewed);
							// Intégration aux résultats des scénarios simulés
							result.insert(result.end(), sim.begin(), sim.end());
	                    }
					}
				}
			}
		}
	}
	if (nbRec == 0)
		delete viewed;
	return result;
}*/

bool Scenario::existsIn(std::vector<Scenario::sp_scenario> set, bool checkPosition){
	bool found = false;
	for (int i = 0 ; i < (signed)set.size() && !found; i++){
		found = this->isEqualWith(set[i], checkPosition);
	}
	return found;
}

bool Scenario::isEqualWith(Scenario::sp_scenario scenario, bool checkPosition){
	std::vector<Trace::sp_trace> sc_pattern = scenario->pattern;
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

bool Scenario::isIncludedIn(Scenario::sp_scenario scenario){
	std::vector<Trace::sp_trace> sc_pattern = scenario->pattern;
	std::stringstream setSS;
	Sequence::exportLinearSequenceAsString(scenario->pattern, setSS, false);
	std::stringstream thisSS;
	Sequence::exportLinearSequenceAsString(pattern, thisSS, false);
	return setSS.str().find(thisSS.str()) != std::string::npos;
}

void Scenario::insertTraces(int prev_post, std::vector<Trace::sp_trace> input_traces){
	if(prev_post == 0){
		std::vector<Trace::sp_trace> newPattern = input_traces;
		newPattern.insert(newPattern.end(), pattern.begin(), pattern.end());
		pattern = newPattern;
	}
	else{
		pattern.insert(pattern.end(), input_traces.begin(), input_traces.end());
	}
}

int Scenario::getLength(){
	int ret = 0;
	for(unsigned int i=0; i<pattern.size(); ++i){
		if (pattern[i]->isSequence() && pattern[i]->getInfo().compare("End") == 0)
			continue;
		//else if (pattern[i]->isSequence() && pattern[i]->getInfo().compare("Begin") == 0)
		//	continue;
		ret++;
	}
	return ret;
}

int Scenario::getNumEnd(int pos){
	int ret=0;
	for(unsigned int i=pos+1; i<pattern.size(); ++i){
		if (pattern[i]->isSequence() && pattern[i]->getInfo().compare("End") == 0)
			ret++;
	}
	return ret;
}

int Scenario::checkEnd(){
	int p = position;
	while(p<(int)pattern.size()-1 && ((pattern[p+1]->isSequence() && pattern[p+1]->getInfo().compare("End") == 0) || (pattern[p+1]->isOptional()))){
		if(pattern[p+1]->isOptional()){
			p++;
		}
		else{
			int posStart = Sequence::getBeginPosOfLinearSequence(pattern, p);
			if(posStart!=-1){
				std::dynamic_pointer_cast<Sequence>(pattern[posStart])->addOne();
				p++;
			}
		}
	}
	if(p==(int)pattern.size()-1){
		return 1;
	}
	return 0;
}
