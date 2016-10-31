with PP; use PP;
with ConstantListKP41; use ConstantListKP41;

-- deplacer le bit et le byte a leurs coordonnees respectives
procedure mission4 is
	pBits, pBytes : Position; -- positions cibles
	u : Unit; -- unite courante
begin
	Open; -- ouverture du jeu
	-- definition des positions cibles
	pBits.x := 1400.0;
	pBits.y := 1371.0;
	pBytes.x := 479.0;
	pBytes.y := 1825.0;
	-- recuperer la premiere unite
	u := GetUnitAt(MyCoalition, 0);
	if GetType(u) = Bit then
		-- deplacer l'unite courante sur sa position cible
		CarryOutCommand(u, Move, pBits);
		-- recuperer l'unite suivante
		u := GetUnitAt(MyCoalition, 1);
		-- la deplacer sur sa position cible
		CarryOutCommand(u, Move, pBytes);
	else
		-- deplacer l'unite courante sur sa position cible
		CarryOutCommand(u, Move, pBytes);
		-- recuperer l'unite suivante
		u := GetUnitAt(MyCoalition, 1);
		-- la deplacer sur sa position cible
		CarryOutCommand(u, Move, pBits);
	end if;
	Close; -- fermer le jeu
end mission4;