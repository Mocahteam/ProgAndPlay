with PP; use PP;
with ConstantListKP41; use ConstantListKP41;

-- deplecer toutes les unites a une coordonnee precise
procedure mission5 is
	p : Position; -- position a atteindre
	u : Unit; -- unite courante
	cpt : Integer; -- compteur de boucle
begin
	Open; -- ouverture du jeu
	p.x := 256.0; -- definition de la coordonnee x a atteindre
	p.y := 1024.0; -- definition de la coordonnee y a atteindre
	-- parcours de toutes les unites
	cpt := 0;
	while cpt < GetNumUnits(MyCoalition) loop
		-- recuperer l'unite courante
		u := GetUnitAt(MyCoalition, cpt);
		-- donner l'ordre de deplacement
		CarryOutCommand(u, Move, p);
		cpt := cpt+1;
	end loop;
	Close; -- fermer le jeu
end mission5;