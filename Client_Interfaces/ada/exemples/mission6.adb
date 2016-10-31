with PP; use PP;
with ConstantListKP41; use ConstantListKP41;

-- deplecer l'assembleur et uniquement lui a une coordonnee precise
procedure mission6 is
	p : Position; -- position a atteindre
	u : Unit; -- unite courante
	cpt : Integer; -- compteur de boucle
begin
	Open; -- ouverture du jeu
	p.x := 256.0; -- definition de la coordonnee x a atteindre
	p.y := 811.0; -- definition de la coordonnee y a atteindre
	-- parcours de toutes les unites
	cpt := 0;
	while cpt < GetNumUnits(MyCoalition) loop
		-- recuperer l'unite courante
		u := GetUnitAt(MyCoalition, cpt);
		-- donner l'ordre de deplacement si c'est un assembleur
		if GetType(u) = Assembler then
			CarryOutCommand(u, Move, p);
		end if;
		cpt := cpt+1;
	end loop;
	Close; -- fermer le jeu
end mission6;