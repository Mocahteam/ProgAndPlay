with PP; use PP;
with ConstantListKP41; use ConstantListKP41;

-- deplacer une unite a une position precise
procedure mission2 is
	posArrivee, posDepart : Position; -- position a atteindre
	u : Unit; -- unite a deplacer
begin
	Open; -- ouverture du jeu
	-- recuperer l'unite
	u := GetUnitAt(MyCoalition, 0);
	-- récupération des coordonnées de l'unité
	posDepart := GetPosition(u);
	-- calcul de la coordonnée d'arrivée
	posArrivee.x := posDepart.x - 927.0;
	posArrivee.y := posDepart.y + 513.0;
	-- deplacer l'unite sur sa position
	CarryOutCommand(u, Move, posArrivee);
	Close; -- fermer le jeu
end mission2;